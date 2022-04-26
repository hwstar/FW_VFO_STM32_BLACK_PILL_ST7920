#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>


// 10ms Time slot
event_data Time_slot;

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

bool EVENT::subscribe(void (*callback)(event_data, uint32_t), uint32_t event_filter)
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

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, char value)
{
    event_data ed;
    ed.char_val = value;
    fire(event_type, event_subtype, ed);
    

}

//
// Fire an event with a char pointer value
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, char *value)
{
    event_data ed;
    ed.cp = value;
    fire(event_type, event_subtype, ed);
    

}

//
// Fire an event with a void pointer value
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, void *value)
{
    event_data ed;
    ed.vp = value;
    fire(event_type, event_subtype, ed);
    

}

//
// Fire an event with no value
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype)
{
    event_data ed;
    ed.u32_val = 0;
    fire(event_type, event_subtype, ed);
}


//
// Fire an event with a uint8_t value
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, uint8_t value)
{
    event_data ed;
    ed.u8_val = value;
    fire(event_type, event_subtype, ed);
    
}

//
// Fire an event with a 32 bit unsigned value
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, uint32_t value)
{
    event_data ed;
    ed.u32_val = value;
    fire(event_type, event_subtype, ed);

}

//
// Fire an event with a 16 bit unsigned value
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, uint16_t value)
{
    event_data ed;
    ed.u16_val = value;
    fire(event_type, event_subtype, ed);

}

//
// Generic fire function
//

void EVENT::fire(uint32_t event_type, uint32_t event_subtype, event_data ed)
{
    uint8_t i;

    for(i = 0; i < num_subs; i++){
        if((et[i].callback) && (et[i].filter & event_type)){
            (*et[i].callback)(ed, event_subtype);
        }
    }


}