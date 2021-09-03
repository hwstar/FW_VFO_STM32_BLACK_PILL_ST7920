//
// VFO class definition
//

#ifndef __VFO_HPP__

#include <bandsel.hpp>

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_DEFAULT 100

class VFO
{
    public:
    
    bool begin(uint32_t init_freq = 14150000UL);
    bool set_freq (uint32_t freq);
    uint32_t get_freq();
    void ptt_set(uint8_t mode);
    uint8_t ptt_get();
    void mode_set(uint8_t mode);
    uint8_t mode_get();

    private:
    bool is_usb;
    bool is_txing;

    BANDS last_band;
    uint32_t vfo_freq;
    uint32_t high_injection_freq;
    uint32_t low_injection_freq;
    uint32_t bfo_carrier_freq;
    int32_t tx_filter_offset;
    uint8_t trx_save;
    uint8_t last_ptt_mode;
    uint8_t band_index;


    void update_clock_gen();
};

#define __VFO_HPP__

#endif

