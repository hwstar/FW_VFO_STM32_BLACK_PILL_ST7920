#include <Arduino.h>
#include <event.hpp>

//
// Constructor
//

EVENT::EVENT()
{
    num_subs = 0;
}

//
// Subscribe to the event bus
//

bool EVENT::subscribe(void (*callback)(event_data ed), uint16_t event_filter)
{
    if(num_subs < MAX_SUBS){
        et[num_subs].filter = event_filter;
        et[num_subs].callback = callback;
        num_subs++;
        return true;
    } else {
        return false;
    }
}

//
// Fire an event with a char value
//

void EVENT::fire(uint8_t event_type, char value)
{
    ed.char_val = value;
    _fire(event_type);
    

}

//
// Fire an event with a uint8_t value
//

void EVENT::fire(uint8_t event_type, uint8_t value)
{
    ed.char_val = value;
    _fire(event_type);
    
}

//
// Fire an event with a 32 bit unsigned value
//

void EVENT::fire(uint8_t event_type, uint32_t value)
{

}

//
// Private fire function
//

void EVENT::_fire(uint8_t event_type)
{
    uint8_t i;

    for(i = 0; i < num_subs; i++){
        if((et[i].callback) && (et[i].filter & event_type)){
            (*et[i].callback)(ed);
        }
    }


}