#include <Arduino.h>
#include <Wire.h>

#include <config.hpp>
#include <event.hpp>
#include <bandsel.hpp>
#include <vfo.hpp>
#include <24cw640.hpp>
#include <mcp4725.hpp>
#include <mcp3422.hpp>
#include <si5351_ek_wrapper.hpp>
#ifdef QUAD_OUTPUT_VFO_BOARD
#include <pca9546.hpp>
#endif

// Output port bits on the TRX board
#define TRX_PTT 0x80
#define TRX_SPKR_MUTE 0x08 
#define TRX_DISABLE_AGC 0x2
#define TRX_ENA_TUNE_OSC 0x1

// SI5351 Clock output definitions
#define SI5351_LOGICAL_CLK2 2
#define SI5351_LOGICAL_CLK1 1
#define SI5351_LOGICAL_CLK0 0

// I2C bus select macros
#ifdef QUAD_OUTPUT_VFO_BOARD
#define SEL_I2C_BUS_EXT pca9546.bus_select(0x01)
#define SEL_I2C_BUS_5351A pca9546.bus_select(0x02)
#define SEL_I2C_BUS_5351B pca9546.bus_select(0x04)
#define SEL_I2C_BUS_VFO_EEPROM pca9546.bus_select(0x08)
#endif


// TRX EEPROM initialization values for IF information

static const trx_eeprom_master_info trx_eeprom_master_init = {
        RECNAME_MASTER,
        TRX_BOARD_NAME      
};


static const vfo_eeprom_cal_info vfo_eeprom_calibration_init = {
    RECNAME_VFOCAL,
    CLK_SOURCE_CAL_VALUE,
};

static const vfo_eeprom_channel_info vfo_eeprom_channel_init  ={
    RECNAME_CHANNEL
};

#ifdef QUAD_OUTPUT_VFO_BOARD
// VFO EEPROM initialization values for VFO calibration, and transceiver settings
static const vfo_eeprom_master_info vfo_eeprom_master_init = {
        RECNAME_MASTER,
        VFO_BOARD_NAME      
};
#endif

// TRX EEPROM initialization values for the crystal filters

static const trx_eeprom_if_info trx_eeprom_if_init = {
    RECNAME_TRXIF, // Record name
    SECOND_IF_CARRIER, // Second IF carrier
    SECOND_IF_BW6DB, // Second IF 6dB bandwidth
    FIRST_IF_BW6DB, // First IF 6dB bandwidth
    FIRST_IF_FCENTER, // First IF center frequency
    FIRST_TO_SECOND_IF_DELTA //  First IF to second IF delta
};

// TRX EEPROM initialization values for TX gain

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

// Band information data structure

typedef struct band_info {
    BANDS band;
    uint32_t lower_edge;
    uint32_t upper_edge;
    bool usb_def;
} band_info;

// Class instantiations

const si5351_clock clock_outputs[3] = { SI5351_CLK0, SI5351_CLK1, SI5351_CLK2 };
const si5351_drive drive_strengths[4] = { SI5351_DRIVE_2MA, SI5351_DRIVE_4MA, SI5351_DRIVE_6MA, SI5351_DRIVE_8MA };
uint8_t page_buffer[EEPROM_24CW640_PAGE_SIZE];
band_info band_table[8];
SI5351_EK_WRAPPER si5351a;
#ifdef QUAD_OUTPUT_VFO_BOARD
SI5351_EK_WRAPPER si5351b;
PCA9546 pca9546(MUX_I2C_ADDRESS);
#endif
PCA9554 lpf(LPF_I2C_ADDR);
PCA9554 bpf(BPF_I2C_ADDR);
PCA9554 trx(TRX_I2C_ADDR);
BANDSEL band_select;
EEPROM_24CW640 eeprom(EEPROM_I2C_ADDR);
DAC_MCP4725 trx_dac(TRX_DAC_I2C_ADDR);
MCP3422 swr_adc;



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
    static uint32_t prev_first_lo_freq, prev_second_lo_freq, prev_third_lo_freq;
    
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
    #ifndef QUAD_OUTPUT_VFO_BOARD
    // Only update LO's if something changed to reduce I2C bus traffic
    if(first_lo_freq != prev_first_lo_freq)
        si5351a.set_freq_hz(first_lo_freq, clock_outputs[SI5351_LOGICAL_CLK0]); // Etherkit CLK0
    if(second_lo_freq != prev_second_lo_freq)
        si5351a.set_freq_hz(second_lo_freq, clock_outputs[SI5351_LOGICAL_CLK1]); // Etherkit CLK1
    if(third_lo_freq != prev_third_lo_freq)
        si5351a.set_freq_hz(third_lo_freq, clock_outputs[SI5351_LOGICAL_CLK2]); // Etherkit CLK2
    #else
    // Only update LO's if something changed to reduce I2C bus traffic
    SEL_I2C_BUS_5351A;
    if(first_lo_freq != prev_first_lo_freq)
        si5351a.set_freq_hz(first_lo_freq, clock_outputs[SI5351_LOGICAL_CLK2]); // J301 (SI5351A CLK2)
    if(second_lo_freq != prev_second_lo_freq)
        si5351a.set_freq_hz(second_lo_freq, clock_outputs[SI5351_LOGICAL_CLK0]); // J302 (SI5351A CLK0)
    SEL_I2C_BUS_5351B;
    if(third_lo_freq != prev_third_lo_freq)
        si5351b.set_freq_hz(third_lo_freq, clock_outputs[SI5351_LOGICAL_CLK2]); // J303 (SI5351B CLK2)
    SEL_I2C_BUS_EXT;
    #endif
    // Save values to check for differences next time
    prev_first_lo_freq = first_lo_freq;
    prev_second_lo_freq = second_lo_freq;
    prev_third_lo_freq = third_lo_freq;
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
// Finction to fire display update on tuning increment change
//

void VFO::display_tuning_increment(uint32_t value)
{
    event_data ed;

    ed.u32_val = value;

    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_SET_TUNING_INCREMENT, ed);
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

    #ifndef VFO_MODULE_TEST_MODE
    // Set the band filters if a new band
    if(last_band != band_table[i].band){
        band_select.set(band_table[i].band);
        last_band = band_table[i].band;
    }
    band_index = i;

    // Set correct TX gain for selected band
    trx_dac.write_fast(trx_gain_info.tx_gain_values[i]);
    #endif
    
    update_clock_gen();

    // Fire display event to update frequency
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_SET_FREQ, freq);

    return true;
}

//
// Service ADC sources which are shown on the display as meters
//


void VFO::service_metering(event_data ed)
{
    if(ed.u32_val != ETS_METERING)
        return;

    float swr_gamma;

    ed_meter_info minfo = {0};

    minfo.mode = EVMM_CLEAR;
    
    if(!is_txing){
    #ifdef S_METER_ADC
        metering_tx_state = 0;
        if(ed.u32_val == ETS_METERING){
            s_meter_reading = analogRead(S_METER_ADC);
        }
    #endif
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_METER_UPDATE, &minfo);
    }
    else if (swr_adc.present()){
        if(ed.u32_val == ETS_METERING ){
            switch(metering_tx_state){
                case 0: // Select forward voltage channel
                    swr_adc.select_channel(0);
                    metering_tx_state++;
                    break;

                case 1: // Read forward voltage, select reverse channel
                    swr_adc.read(swr_forward_voltage);
                    swr_adc.select_channel(1);
                    metering_tx_state++;
                    break;

                case 2:
                    swr_adc.read(swr_reverse_voltage);
                    metering_tx_state++;
                    break; // DEBUG

                case 3:
                    
                    // Calculate SWR
                    if(last_ptt_mode == RADIO_TUNE){
                        minfo.mode = EVMM_SWR;
                        swr_gamma = ((float) swr_reverse_voltage)/((float) swr_forward_voltage);
                        swr = (1.0 + abs(swr_gamma))/(1.0 - abs(swr_gamma));
                        minfo.value =  swr;
                        minfo.full_scale = 5.0;
                        minfo.legend = "SWR";
                    } else if (last_ptt_mode == RADIO_TX) {
                        minfo.mode = EVMM_TX_POWER;
                        minfo.value_u16 = minfo.full_scale_u16 = minfo.peak_value_u16 = 0; // TODO map output power to watt numbers
                        minfo.legend = "PWR";
                    }
                    // Send the meter info
                    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_METER_UPDATE, &minfo);
                    // Select channel 0, and start the conversion.
                    swr_adc.select_channel(0);
                    metering_tx_state = 1;
        
                    break;

                default:
                    metering_tx_state = 0;
                    break;
            }
        }

    }

}


//
// VFO event subscriber
//

void VFO::subscriber(event_data ed, uint32_t event_subtype )
{
    uint16_t vfo_incr;
    
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
            display_tuning_increment(tuning_knob_increment);
            break;
        case EV_SUBTYPE_TICK_MS:
            service_metering(ed);
            break;

        case EV_SUBTYPE_TICK_HUNDRED_MS:
            break;

        default:
            break;

    }
}

void VFO::initialize_eeprom()
{
    int recnum;
    SEL_I2C_BUS_VFO_EEPROM;
    eeprom.write_page(RECNUM_EEPROM_MASTER, &vfo_master_info);
    eeprom.write_page(RECNUM_EEPROM_VFO_CAL, &vfo_cal_info);
    // Initialize channel memory
    for(recnum = RECNUM_BASE_EEPROM_VFO_CHANNEL; recnum < RECNUM_LAST; recnum++){
        eeprom.write_page(recnum, &vfo_channel_info);

    }

    SEL_I2C_BUS_EXT;
}


//
// Initialize VFO
//

bool VFO::begin(uint32_t init_freq) 
{
    digitalWrite(PIN_STM32_LED,1); // LED off

    uint8_t i;
    bool trx_eeprom_invalid = false;
    bool vfo_eeprom_invalid = false;
    
    // If quad output VFO, select external I2C bus.
    #ifdef QUAD_OUTPUT_VFO_BOARD
    SEL_I2C_BUS_EXT;
    #endif

    // If not standalone VFO test mode
    #ifndef VFO_MODULE_TEST_MODE
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

    // SWR ADC initialization
    // One shot operation, gain = 0, 12 bits.
    swr_adc.begin(SWR_BRIDGE_DAC_ADDR, true, 0, 0);
    
    #ifdef QUAD_OUTPUT_VFO_BOARD
    // Check for presence of VFO EEPROM U602
    have_vfo_eeprom = false;
    SEL_I2C_BUS_VFO_EEPROM;
    if(eeprom.present()){
        if(!eeprom.read_page(RECNUM_EEPROM_MASTER, page_buffer)){
            SEL_I2C_BUS_EXT;
            pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_VFO_EEPROM_READ);
        }
        else {
            SEL_I2C_BUS_EXT;
            have_vfo_eeprom = true;
        }
    }
    else{
        SEL_I2C_BUS_EXT;
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_VFO_EEPROM_PRESENT);    
    }
    #endif


    // Test for the presence of the trx EEPROM
    have_trx_eeprom = false;
    if(eeprom.present()) {
        // Read the header at page 0
        if(!eeprom.read_page(RECNUM_EEPROM_MASTER, page_buffer))
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

    #endif
    //
    // Initialization of system constants
    //
    #ifdef QUAD_OUTPUT_VFO_BOARD
    #ifdef INITIALIZE_VFO_EEPROM
    vfo_master_info = vfo_eeprom_master_init;
    vfo_cal_info = vfo_eeprom_calibration_init;
    vfo_channel_info = vfo_eeprom_channel_init;
    initialize_eeprom();
    #else

    //
    // Check VFO eeprom contents
    //
    SEL_I2C_BUS_VFO_EEPROM;
    if(have_vfo_eeprom){
        if(!eeprom.verify_header(RECNUM_EEPROM_MASTER, RECNAME_MASTER, &vfo_master_info))
            vfo_eeprom_invalid = true;
        if(!eeprom.verify_header(RECNUM_EEPROM_VFO_CAL, RECNAME_VFOCAL, &vfo_cal_info))
            vfo_eeprom_invalid = true;
        if(!eeprom.verify_header(RECNUM_BASE_EEPROM_VFO_CHANNEL, RECNAME_CHANNEL, &vfo_cal_info))
            vfo_eeprom_invalid = true;
    }
    SEL_I2C_BUS_EXT;
    
    if(!have_vfo_eeprom || vfo_eeprom_invalid){
        vfo_master_info = vfo_eeprom_master_init;
        vfo_cal_info = vfo_eeprom_calibration_init;
        vfo_channel_info = vfo_eeprom_channel_init;
        // If we have the vfo EEPROM, initialize it here
        if(have_trx_eeprom){
            initialize_eeprom();
        }
    }

    #endif
    #endif


    // If initialization is forced from config.hpp
    #ifdef INITIALIZE_TRX_EEPROM
    trx_master_info = trx_eeprom_master_init;
    trx_gain_info = trx_eeprom_txgain_init;
    trx_if_info = trx_eeprom_if_init;
    eeprom.write_page(RECNUM_EEPROM_MASTER, &trx_master_info);
    eeprom.write_page(RECNUM_EEPROM_TXGAIN, &trx_gain_info);
    eeprom.write_page(RECNUM_EEPROM_IF, &trx_if_info);
    #else
    // Normal initialization
    if(have_trx_eeprom){
        if(!eeprom.verify_header(RECNUM_EEPROM_MASTER, RECNAME_MASTER, &trx_master_info))
            trx_eeprom_invalid = true;

        if(!eeprom.verify_header(RECNUM_EEPROM_IF, RECNAME_TRXIF, &trx_if_info))
            trx_eeprom_invalid = true;
        
        if(!eeprom.verify_header(RECNUM_EEPROM_TXGAIN, RECNAME_TXGAIN, &trx_gain_info))
            trx_eeprom_invalid = true; 
    }

    if(!have_trx_eeprom || trx_eeprom_invalid){
        // Load constants from config.hpp 
        trx_master_info = trx_eeprom_master_init;
        trx_gain_info = trx_eeprom_txgain_init;
        trx_if_info = trx_eeprom_if_init;
        // If we have the trx EEPROM, initialize it here
        if(have_trx_eeprom){
            eeprom.write_page(RECNUM_EEPROM_MASTER, &trx_master_info);
            eeprom.write_page(RECNUM_EEPROM_TXGAIN, &trx_gain_info);
            eeprom.write_page(RECNUM_EEPROM_IF, &trx_if_info);
        }

    }   
    #endif

    // Initialize si5351a
    #ifndef QUAD_OUTPUT_VFO_BOARD
    if(!si5351a.init_sans_wire_begin(SI5351_CRYSTAL_LOAD_8PF, REF_TCXO_FREQ_HZ, CLK_SOURCE_CAL_VALUE)){
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_NO_CLKGEN);
    }
    #else // Quad output VFO board
    SEL_I2C_BUS_5351A;
    if(!si5351a.init_sans_wire_begin(SI5351_CRYSTAL_LOAD_8PF, REF_TCXO_FREQ_HZ , vfo_cal_info.cal_value)){
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_NO_CLKGEN);
    }
    SEL_I2C_BUS_5351B;
    if(!si5351b.init_sans_wire_begin(SI5351_CRYSTAL_LOAD_8PF, REF_TCXO_FREQ_HZ , vfo_cal_info.cal_value)){
        pubsub.fire(EVENT_ERROR,EV_SUBTYPE_ERR_NO_SECOND_CLKGEN);
    }  
    SEL_I2C_BUS_EXT;
    #endif

    #ifndef QUAD_OUTPUT_VFO_BOARD


    for(i = 0; i < 3; i++) {
        // Single SI5351 - All 3 outputs at 4 mA
        // Default freq 10 MHz
        si5351a.set_freq_hz(10000000, clock_outputs[i]);
        // 4mA Drive strength
        si5351a.drive_strength(clock_outputs[i], SI5351_DRIVE_4MA);
       

        // Note: setting up the previous commands turns the outputs on for some reason. Output disable must be the last thing we do.
        // All outputs off
        si5351a.output_enable(clock_outputs[i], 0);
    }

    #else
        // Dual SI5351 - All outputs at 8 mA
        
        // First SI5351
        // Output 0 is the first output (Mixer 1)
        SEL_I2C_BUS_5351A;
        si5351a.set_freq_hz(10000000, clock_outputs[SI5351_LOGICAL_CLK2]);
        si5351a.drive_strength(clock_outputs[SI5351_LOGICAL_CLK2], SI5351_DRIVE_8MA);
       
        // Output 2 is the second output (Mixer 2)
        si5351a.set_freq_hz(10000000, clock_outputs[SI5351_CLK0]);
        si5351a.drive_strength(clock_outputs[SI5351_LOGICAL_CLK0], SI5351_DRIVE_8MA);
     
        // SI5351A All outputs off
        for(i = 0; i < 3; i++)
            si5351a.output_enable(clock_outputs[i], 0);
        
        // Second SI5351
        // Output 2 on si3551b is the third output. (Mixer 3)
        SEL_I2C_BUS_5351B;
        si5351b.set_freq_hz(10000000, clock_outputs[SI5351_LOGICAL_CLK2]);
        si5351b.drive_strength(clock_outputs[SI5351_LOGICAL_CLK2], SI5351_DRIVE_8MA);

        // SI5351B All outputs off
        for(i = 0; i < 3; i++)
            si5351b.output_enable(clock_outputs[i], 0);
        // Reset I2C bus multplexer to the external I2C bus
        SEL_I2C_BUS_EXT;

    #endif

  

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
    #ifndef VFO_MODULE_TEST_MODE
    band_select.begin(&bpf, &lpf);
    #endif

    
    tuning_knob_increment = 1000UL;
    display_tuning_increment(tuning_knob_increment);
    is_txing = false;
    test_mode = true;
    update_display_tx(is_txing);
    set_freq(init_freq);
    agc_set(1);
    sideband_set(MODE_DEFAULT);

    #ifndef QUAD_OUTPUT_VFO_BOARD
    // Enable all 3 outputs on the Etherkit Si5351 board
    si5351a.output_enable(clock_outputs[0], 1);
    si5351a.output_enable(clock_outputs[1], 1);
    si5351a.output_enable(clock_outputs[2], 1);
    #else
    // For Quad output VFO board, enable outputs 0 and 2
    // on the first SI5351, and output 0 on the second 
    // SI5351
    SEL_I2C_BUS_5351A;
    si5351a.output_enable(clock_outputs[0], 1);
    si5351a.output_enable(clock_outputs[2], 1);
    SEL_I2C_BUS_5351B;
    si5351b.output_enable(clock_outputs[2], 1);
    SEL_I2C_BUS_EXT;
    #endif

    digitalWrite(PIN_STM32_LED, 0); // LED on

    return true;

} 

