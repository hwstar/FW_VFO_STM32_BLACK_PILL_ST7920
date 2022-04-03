#ifndef __SWITCHES_HPP__


class SWITCHES
{
    public:
    void begin(uint8_t ten_ms_time_slot = 0){
        time_slot = ten_ms_time_slot;
    }

   
    void handler(event_data ed, uint32_t event_subtype);
   
    private:
    uint8_t time_slot;

};

#define __SWITCHES_HPP__
#endif
