
#ifndef __TX_PROTECT_HPP__

class TX_PROTECT
{
    public:
    void begin(){
        fan_timer_increment = -1;
        fan_timer = 0;
        reference_count = 0;

    }

    void handler(event_data ed, uint32_t event_subtype);

    private:
    uint8_t reference_count;
    int8_t fan_timer_increment;
    uint16_t fan_timer;


    
};

#define __TX_PROTECT_HPP__
#endif

