#ifndef __SWITCHES_HPP__

#define LONG_PRESS_TIME 200 // 2 Seconds defines a long press

class SWITCHES
{
    public:
    void begin(uint8_t ten_ms_time_slot = 0){
        time_slot = ten_ms_time_slot;
    }

   
    void handler(event_data ed, uint32_t event_subtype);
   
    private:
    uint8_t time_slot;
    uint16_t encoder_knob_down_timer;

};

#define __SWITCHES_HPP__
#endif
