#include <Arduino.h>
#include <Wire.h>

#include <config.hpp>
#include <vfo.hpp>
#include <logger.hpp>

#include <si5351.h>

#define TRX_PTT 0x80
#define TRX_M16 0x4
#define TRX_DISABLE_AGC 0x2
#define TRX_ENA_TUNE_OSC 0x1



class Ext_Si5351 : public Si5351
{
  public:
    Ext_Si5351(uint8_t i2c_addr = SI5351_BUS_BASE_ADDR) : Si5351 {i2c_addr}
    {
    }
    
    /*
    * Set frequency in Hz
    */
  
    uint8_t set_freq_hz(uint32_t freq_hz, enum si5351_clock output)
    {
      uint64_t freq = 100ULL * freq_hz;
      return set_freq(freq, output);
    }
};


typedef struct band_info {
    BANDS band;
    uint32_t lower_edge;
    uint32_t upper_edge;
    bool usb_def;
} band_info;

const si5351_clock clock_outputs[3] = { SI5351_CLK0, SI5351_CLK1, SI5351_CLK2 };
const si5351_drive drive_strengths[4] = { SI5351_DRIVE_2MA, SI5351_DRIVE_4MA, SI5351_DRIVE_6MA, SI5351_DRIVE_8MA };

band_info band_table[8];

Ext_Si5351 si5351;
PCA9554 lpf(LPF_I2C_ADDR);
PCA9554 bpf(BPF_I2C_ADDR);
PCA9554 trx(TRX_I2C_ADDR);
BANDSEL band_select;


//
// Update clock generator channels
//
// Private
//


void VFO::update_clock_gen()
{
    // Calculate the injection frequencies from the vfo frequency
  
    uint32_t first_lo_freq;
    uint32_t second_lo_freq;
    
    //
    // Calculate the injection frequencies
    //

    high_injection_freq = vfo_freq + bfo_carrier_freq;
    if(vfo_freq > bfo_carrier_freq){
        low_injection_freq = vfo_freq - bfo_carrier_freq;
    }
    else{
        low_injection_freq = bfo_carrier_freq - vfo_freq;
    }

    /*
    Serial1.printf("IS TXing         : %u\r\n", is_txing);
    Serial1.printf("VFO Freq         : %lu\r\n", vfo_freq);
    Serial1.printf("High Injection   : %lu\r\n", high_injection_freq);
    Serial1.printf("Low Injection    : %lu\r\n", low_injection_freq);
    */
    

    if(is_txing){
        // First LO gets the carrier frequency
        first_lo_freq = bfo_carrier_freq;
        second_lo_freq = (is_usb)? high_injection_freq : low_injection_freq;

    } else { // RX
        // Second LO gets the carrier frequency.
        second_lo_freq = bfo_carrier_freq;
        first_lo_freq = (is_usb)? high_injection_freq : low_injection_freq;
    }
    //
    // Set the LO frequencies
    //
    si5351.set_freq_hz(first_lo_freq, clock_outputs[FIRST_LO_ID]);
    si5351.set_freq_hz(second_lo_freq, clock_outputs[SECOND_LO_ID]);
}

//
// Get current vfo frequency
//

uint32_t VFO::get_freq()
{
    return vfo_freq;
}

//
// Set ptt state
//

void VFO::ptt_set(uint8_t mode)
{
    static uint16_t ptt_count;

    if(0 == mode){
        is_txing = false;
        last_ptt_mode = mode;
        trx_save &= ~(TRX_ENA_TUNE_OSC | TRX_PTT);
    }
    else if (1 == mode){
        last_ptt_mode = mode;
        is_txing = true;
        trx_save &= ~( TRX_ENA_TUNE_OSC);
        trx_save |= (TRX_PTT);
    }
    else{
        last_ptt_mode = mode;
        is_txing = true;
        trx_save |=  (TRX_ENA_TUNE_OSC | TRX_PTT);
    }
    update_clock_gen();
    trx.write(trx_save);
    // Breakpoint
    ptt_count++;
}

//
// Get ptt state
//
uint8_t VFO::ptt_get()
{
    return last_ptt_mode;
}

//
// Set sideband
//
void VFO::mode_set(uint8_t mode)
{
    if(is_txing)
        return;

    is_usb = (bool) mode;
    update_clock_gen();
}

//
// Get sideband
//

uint8_t VFO::mode_get()
{
    return (uint8_t) is_usb;

}


//
// Set VFO frequency
//

bool VFO::set_freq (uint32_t freq)
{
    int i;
    // Do not allow frequency to be adjusted when transmitting

    if(is_txing)
        return false;

    // Validate VFO frequency

    for(i = 0; i < 8; i++) {
        if(freq < band_table[i].upper_edge){
            if(freq > band_table[i].lower_edge){
                break;
            }
        }
    }
    if (i >= 8)
        return false;

    vfo_freq = freq;

    // Set the band filters if a new band
    if(last_band != band_table[i].band){
        band_select.set(band_table[i].band);
        last_band = band_table[i].band;
    }

    
    update_clock_gen();

    return true;
}


//
// Initialize VFO
//

bool VFO::begin(uint32_t init_freq) 
{
    digitalWrite(PIN_STM32_LED,1); // LED off

    uint8_t i;


    // SI5351 library calls Wire.begin, so it has to be the first thing initialized

    if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, CLK_SOURCE_CAL_VALUE)){
        logger.error(ERR_NO_CLK_GEN);
        return false;
    }

    // Initialize the TRX motherboard defaults

    if(!trx.present()){
        logger.error(ERR_NO_TRX);
        return false;
    }
    else{
        trx_save = 0;
        trx.write(trx_save);
        trx.set_gpio_config(0x00); 
    }


    for(i = 0; i < 3; i++) {
        // Default freq 10 MHz
        si5351.set_freq_hz(10000000, clock_outputs[i]);
        // 8mA Drive strength
        si5351.drive_strength(clock_outputs[i], drive_strengths[3]);
        // Note: setting up the previous commands turns the outputs on for some reason. Output disable must be the last thing we do.
        // All outputs off
        si5351.output_enable(clock_outputs[i], 0);
    }

  

    //
    // Generate band table from the configuration information
    //

    #ifdef BAND_FILTER_1
        band_table[0].band = BAND_FILTER_1;
        band_table[0].lower_edge = BAND_EDGE_LOW_1;
        band_table[0].upper_edge = BAND_EDGE_HIGH_1;
        band_table[0].usb_def = BAND_DEF_USB_1;
    #endif

    #ifdef BAND_FILTER_2
        band_table[1].band = BAND_FILTER_2;
        band_table[1].lower_edge = BAND_EDGE_LOW_2;
        band_table[1].upper_edge = BAND_EDGE_HIGH_2;
        band_table[1].usb_def = BAND_DEF_USB_2;
    #endif

    #ifdef BAND_FILTER_3
        band_table[2].band = BAND_FILTER_3;
        band_table[2].lower_edge = BAND_EDGE_LOW_3;
        band_table[2].upper_edge = BAND_EDGE_HIGH_3;
        band_table[2].usb_def = BAND_DEF_USB_3;
    #endif

    #ifdef BAND_FILTER_4
        band_table[3].band = BAND_FILTER_4;
        band_table[3].lower_edge = BAND_EDGE_LOW_4;
        band_table[3].upper_edge = BAND_EDGE_HIGH_4;
        band_table[3].usb_def = BAND_DEF_USB_4;
    #endif

    #ifdef BAND_FILTER_5
        band_table[4].band = BAND_FILTER_5;
        band_table[4].lower_edge = BAND_EDGE_LOW_5;
        band_table[4].upper_edge = BAND_EDGE_HIGH_5;
        band_table[4].usb_def = BAND_DEF_USB_5;
    #endif

    #ifdef BAND_FILTER_6
        band_table[5].band = BAND_FILTER_6;
        band_table[5].lower_edge = BAND_EDGE_LOW_6;
        band_table[5].upper_edge = BAND_EDGE_HIGH_6;
        band_table[5].usb_def = BAND_DEF_USB_6;
    #endif

    #ifdef BAND_FILTER_7
        band_table[6].band = BAND_FILTER_7;
        band_table[6].lower_edge = BAND_EDGE_LOW_7;
        band_table[6].upper_edge = BAND_EDGE_HIGH_7;
        band_table[6].usb_def = BAND_DEF_USB_7;
    #endif

    #ifdef BAND_FILTER_7
        band_table[7].band = BAND_FILTER_8;
        band_table[7].lower_edge = BAND_EDGE_LOW_8;
        band_table[7].upper_edge = BAND_EDGE_HIGH_8;
        band_table[7].usb_def = BAND_DEF_USB_8;
    #endif

    // Initialize the band select 
    band_select.begin(&bpf, &lpf);

    bfo_carrier_freq = CARRIER_OSC_FREQ;
    
    is_txing = false;
    set_freq(init_freq);
    si5351.output_enable(clock_outputs[FIRST_LO_ID], 1);
    si5351.output_enable(clock_outputs[SECOND_LO_ID], 1);


    digitalWrite(PIN_STM32_LED,0); // LED on

    return true;

} 
