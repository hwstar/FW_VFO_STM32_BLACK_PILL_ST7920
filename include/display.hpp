#ifndef __DISPLAY_HPP__
//#include <Arduino.h>
//#include <U8g2lib.h>
//#include <event.hpp>

class DISPLAY_DRIVER
{
    public:
    void begin();
    void refresh_normal_operation();
    void refresh();
    void events(event_data ed, uint32_t event_subtype);

    private:
    uint32_t freq;
    uint8_t trx_mode;
    uint8_t display_mode;
    uint8_t sideband;
    uint8_t agc_state;
    char *keypad_keys;
    
    
};

#define __DISPLAY_HPP__

#endif
