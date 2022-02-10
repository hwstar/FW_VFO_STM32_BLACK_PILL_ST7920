//
// VFO class definition
//

#ifndef __VFO_HPP__

//#include <bandsel.hpp>
//#include <config.hpp>
//#include <event.hpp>


class VFO
{
    public:
    
    bool begin(uint32_t init_freq = 14250000UL, void (*event_callback)(uint32_t, uint32_t, event_data) = NULL);
    uint32_t get_freq(); 
    uint8_t ptt_get();
    uint8_t mode_get();
    uint32_t incr_get(); 

    void subscriber(event_data ed, uint32_t event_subtype);


    private:
    bool is_usb;
    bool is_txing;
    bool test_mode;
    BANDS last_band;
    uint32_t vfo_freq;
    uint32_t high_injection_freq;
    uint32_t low_injection_freq;
    uint32_t bfo_carrier_freq;
    uint32_t tuning_knob_increment;
    int32_t tx_filter_offset;
    uint8_t trx_save;
    uint8_t last_ptt_mode;
    uint8_t band_index;

    void (*_event_callback)(uint32_t, uint32_t, event_data );


    void update_clock_gen();
    void update_display_tx(uint8_t val);
    bool set_freq (uint32_t freq); 
    void agc_set(uint8_t state);
    void ptt_set(uint8_t mode);
    void mode_set(uint8_t mode);
    void fire_event(uint32_t event_type, uint32_t event_subtype, uint32_t ev_data);
    void fire_event(uint32_t event_type, uint32_t event_subtype, uint8_t ev_data);
    void fire_event(uint32_t event_type, uint32_t event_subtype);

};

#define __VFO_HPP__

#endif

