#include <Arduino.h>
#include <Wire.h>

#include <config.hpp>
#include <event.hpp>
#include <bandsel.hpp>
#include <vfo.hpp>
#include <24cw640.hpp>
#include <mcp4725.hpp>

#include <si5351.h>

#define TRX_PTT 0x80
#define TRX_SPKR_MUTE 0x08 
#define TRX_DISABLE_AGC 0x2
#define TRX_ENA_TUNE_OSC 0x1


// TRX EEPROM initialization values for IF information

static const trx_eeprom_master_info trx_eeprom_master_init = {
        RECNAME_MASTER,
        SYSTEM_NAME      
};

static const trx_eeprom_if_info trx_eeprom_if_init = {
    RECNAME_TRXIF, // Record name
    SECOND_IF_CARRIER, // Second IF carrier
    SECOND_IF_BW6DB, // Second IF bandwidth
    FIRST_IF_BW6DB, // First IF 6dB bandwidth
    FIRST_IF_FCENTER, // First IF center frequency
    FIRST_TO_SECOND_IF_DELTA //  First IF to second IF delta
};

// TRX EEPROM initialization values for TX fain

static const trx_eeprom_txgain_info trx_eeprom_txgain_init = {
    RECNAME_TXGAIN, // Record name
    { // Gain constants
        TRX_TXGAIN_160M,
        TRX_TXGAIN_80M,
        TRX_TXGAIN_40M,
        TRX_TXGAIN_20M,
        TRX_TXGAIN_17M,
        TRX_TXGAIN_15M,
        TRX_TXGAIN_12M,
        TRX_TXGAIN_10M
    }

};



//
// Extend Etherkit Si5351 class to include setting frequency in Hz.
//

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
uint8_t page_buffer[EEPROM_24CW640_PAGE_SIZE];
band_info band_table[8];
Ext_Si5351 si5351;
PCA9554 lpf(LPF_I2C_ADDR);
PCA9554 bpf(BPF_I2C_ADDR);
PCA9554 trx(TRX_I2C_ADDR);
BANDSEL band_select;
EEPROM_24CW640 trx_eeprom(TRX_EEPROM_I2C_ADDR);
DAC_MCP4725 trx_dac(TRX_DAC_I2C_ADDR);



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
    uint32_t third_lo_freq;
    
    //
    // Calculate the injection frequencies
    //

    high_injection_freq = vfo_freq + trx_if_info.first_if_fcenter;
    low_injection_freq = trx_if_info.first_if_fcenter-vfo_freq;

    if(is_txing){
        // First LO gets the carrier frequency
        first_lo_freq = trx_if_info.second_if_carrier;
        second_lo_freq = trx_if_info.first_to_second_if_delta; // One inversion here

        third_lo_freq = (is_usb)? high_injection_freq : low_injection_freq; // DEBUG

    } else { // RX
        // Second LO gets the carrier frequency.
        first_lo_freq = trx_if_info.first_to_second_if_delta; // One inversion here
        second_lo_freq = trx_if_info.second_if_carrier; 
        third_lo_freq = (is_usb)? high_injection_freq : low_injection_freq; // DEBUG
    }
    //
    // Set the LO frequencies
    //
    si5351.set_freq_hz(first_lo_freq, clock_outputs[FIRST_LO_ID]);
    si5351.set_freq_hz(second_lo_freq, clock_outputs[SECOND_LO_ID]);
    si5351.set_freq_hz(third_lo_freq, clock_outputs[THIRD_LO_ID]);
}


//
// Update the display showing the TX state
//

void VFO::update_display_tx(uint8_t mode)
{
    event_data ed;
    ed.u8_val = mode;
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_TRX_MODE, ed);

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

    if(RADIO_RX == mode){
        is_txing = false;
        last_ptt_mode = mode;
        trx_save &= ~(TRX_ENA_TUNE_OSC | TRX_PTT | TRX_SPKR_MUTE);
    }
    else if (RADIO_TX == mode){
        last_ptt_mode = mode;
        is_txing = true;
        trx_save &= ~(TRX_ENA_TUNE_OSC);
        trx_save |= (TRX_PTT | TRX_SPKR_MUTE);
    }
    else{
        last_ptt_mode = mode;
        is_txing = true;
        trx_save |=  (TRX_ENA_TUNE_OSC | TRX_PTT | TRX_SPKR_MUTE);
    }
    update_clock_gen();
    trx.write(trx_save);

    // Fire event to update PTT mode
    update_display_tx(mode);
    

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
void VFO::sideband_set(uint8_t mode)
{
    if(is_txing)
        return;

    if(MODE_DEFAULT == mode)
        mode = band_table[band_index].usb_def; // Sideband default for band

    is_usb = (bool) mode;
    update_clock_gen();

    // Fire display event to update mode
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_SET_SIDEBAND, mode);
}

//
// Enable or disable the AGC
//

void VFO::agc_set(uint8_t state)
{
    if(state)
        trx_save &=  ~TRX_DISABLE_AGC;
    else
        trx_save |= TRX_DISABLE_AGC;
    trx.write(trx_save);
    // Fire display event to update mode
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_SET_AGC, state);
}

//
// Set tx gain for calibration purposes
//

void VFO::set_tx_gain(uint16_t gain)
{

    trx_dac.write_fast(gain);
    trx_gain_info.tx_gain_values[band_index];
}

//
// Store current TX gain for a given band
//

void VFO::store_tx_gain()
{
    return;
}


//
// Get mode
//

uint8_t VFO::mode_get()
{
    return (uint8_t) is_usb;

}

//
// Get tuning knob increment
//

uint32_t VFO::incr_get()
{
    return tuning_knob_increment;
}



//
// Set VFO frequency
//

bool VFO::set_freq (uint32_t freq)
{
    int i;
    // Do not allow frequency to be adjusted when transmitting
    // Except when we are in test mode

    if(is_txing && !test_mode)
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
    band_index = i;

    // Set correct TX gain for selected band
    trx_dac.write_fast(trx_gain_info.tx_gain_values[i]);

    update_clock_gen();

    // Fire display event to update frequency
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_SET_FREQ, freq);

    return true;
}

//
// VFO event subscriber
//

void VFO::subscriber(event_data ed, uint32_t event_subtype )
{
    switch(event_subtype){
        case EV_SUBTYPE_TUNE_CW:
            set_freq(get_freq() + tuning_knob_increment);
            break;
        case EV_SUBTYPE_TUNE_CCW:
            set_freq(get_freq() - tuning_knob_increment);
            break;
        case EV_SUBTYPE_SET_FREQ:
            set_freq(ed.u32_val);
            sideband_set(MODE_DEFAULT); // Set default for band
            break;
        case EV_SUBTYPE_SET_AGC:
            agc_set(ed.u8_val);
            break;
        case EV_SUBTYPE_SET_TXGAIN:
            set_tx_gain(ed.u16_val);
            break;
        case EV_SUBTYPE_STORE_TXGAIN:
            store_tx_gain();
            break;
        case EV_SUBTYPE_SET_SIDEBAND:
            sideband_set(ed.u8_val);
            break;
        case EV_SUBTYPE_PTT_PRESSED:
            ptt_set(RADIO_TX);
            break;
        case EV_SUBTYPE_PTT_RELEASED:
            ptt_set(RADIO_RX);
            break;
        case EV_SUBTYPE_TUNE_PRESSED:
            ptt_set(RADIO_TUNE);
            break;
        case EV_SUBTYPE_TUNE_RELEASED:
            ptt_set(RADIO_RX);
            break;
        case EV_SUBTYPE_SET_INCR:
            tuning_knob_increment = ed.u32_val;
            break;
        case EV_SUBTYPE_TICK_MS:
            break;

        case EV_SUBTYPE_TICK_HUNDRED_MS:
            break;

        default:
            break;

    }
}


//
// Initialize VFO
//

bool VFO::begin(uint32_t init_freq) 
{
    digitalWrite(PIN_STM32_LED,1); // LED off

    uint8_t i;
    bool eeprom_invalid = false;

    // SI5351 library calls Wire.begin, so it has to be the first thing initialized


    if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, CLK_SOURCE_CAL_VALUE)){
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_NO_CLKGEN);
    }

    // Initialize the TRX motherboard defaults
    if(trx.present()){
        // Transceiver board present
        trx_save = 0;
        trx.write(trx_save);
        trx.set_gpio_config(0x00); 
    }
    else { 
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_NO_TRX); // TRX board not present
    }



    // Test for the presence of the EEPROM
    have_trx_eeprom = false;
    if(trx_eeprom.present()) {
        // Read the header at page 0
        if(!trx_eeprom.read_page(RECNUM_EEPROM_MASTER, page_buffer))
            pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_EEPROM_READ);
        have_trx_eeprom = true;
    }
    else
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_EEPROM_PRESENT);

  

    // Test for the presence of the trx DAC
    have_trx_dac = false;
    if(!trx_dac.present()) {
       pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_DAC_PRESENT);
    } else {
        have_trx_dac = true;
    }

    //
    // Initialization of system constants
    //

    

    // If initialization is forced from config.hpp
    #ifdef INITIALIZE_TRX_EEPROM
    trx_master_info = trx_eeprom_master_init;
    trx_gain_info = trx_eeprom_txgain_init;
    trx_if_info = trx_eeprom_if_init;
    trx_eeprom.write_page(RECNUM_EEPROM_MASTER, &trx_master_info);
    trx_eeprom.write_page(RECNUM_EEPROM_TXGAIN, &trx_gain_info);
    trx_eeprom.write_page(RECNUM_EEPROM_IF, &trx_if_info);
    #else
    // Normal initialization
    if(have_trx_eeprom){
        trx_eeprom.read_page(RECNUM_EEPROM_MASTER, &trx_master_info);
        if(strncmp(trx_master_info.recordname, RECNAME_MASTER, sizeof(RECNAME_MASTER)))
            eeprom_invalid = true;

        trx_eeprom.read_page(RECNUM_EEPROM_IF, &trx_if_info);
        if(strncmp(trx_if_info.recordname, RECNAME_TRXIF, sizeof(RECNAME_TRXIF)))
            eeprom_invalid = true;
        
        trx_eeprom.read_page(RECNUM_EEPROM_TXGAIN, &trx_gain_info);
        if(strncmp(trx_gain_info.recordname, RECNAME_TXGAIN, sizeof(RECNAME_TXGAIN)))
            eeprom_invalid = true; 
    }

    if(!have_trx_eeprom || eeprom_invalid){
        // Load constants from config.hpp 
        trx_master_info = trx_eeprom_master_init;
        trx_gain_info = trx_eeprom_txgain_init;
        trx_if_info = trx_eeprom_if_init;
        // If we have the trx EEPROM, initialize it here
        if(have_trx_eeprom){
            trx_eeprom.write_page(RECNUM_EEPROM_MASTER, &trx_master_info);
            trx_eeprom.write_page(RECNUM_EEPROM_TXGAIN, &trx_gain_info);
            trx_eeprom.write_page(RECNUM_EEPROM_IF, &trx_if_info);
        }

    }   
    #endif



    
    for(i = 0; i < 3; i++) {
        // Default freq 10 MHz
        si5351.set_freq_hz(10000000, clock_outputs[i]);
        // 4mA Drive strength
        si5351.drive_strength(clock_outputs[i], drive_strengths[1]);
       

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

    
    tuning_knob_increment = 1000UL;
    is_txing = false;
    test_mode = true;
    update_display_tx(is_txing);
    set_freq(init_freq);
    agc_set(1);
    sideband_set(MODE_DEFAULT);
    si5351.output_enable(clock_outputs[FIRST_LO_ID], 1);
    si5351.output_enable(clock_outputs[SECOND_LO_ID], 1);
    si5351.output_enable(clock_outputs[THIRD_LO_ID], 1);

    digitalWrite(PIN_STM32_LED, 0); // LED on

    return true;

} 

