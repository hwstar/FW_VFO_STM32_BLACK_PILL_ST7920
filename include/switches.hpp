#ifndef __SWITCHES_HPP__


class SWITCHES
{
    public:
    void begin(void (*fire_event)(uint32_t event, uint8_t event_subtype, event_data ed), uint8_t ten_ms_time_slot = 0){
        event_fire = fire_event;
        time_slot = ten_ms_time_slot;
    }

   
    void handler(event_data ed, uint8_t event_subtype);
   
    private:
    uint8_t time_slot;
    uint8_t ms_counter;
    void (*event_fire)(uint32_t event, uint8_t event_subtype, event_data ed);

};

#define __SWITCHES_HPP__
#endif
