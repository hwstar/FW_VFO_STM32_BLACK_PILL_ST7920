#ifndef __KEYPAD_HPP__

class KEYPAD
{
    public:
    void begin(uint8_t timeslot){
        uint8_t ten_ms_time_slot = timeslot;
    }
    void handler(event_data ed, uint32_t event_subtype);

    private:
        uint8_t ten_ms_time_slot;

};

#define __KEYPAD_HPP__
#endif
