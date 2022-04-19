#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <knob.hpp>

void KNOB::tuning(event_data ed, uint32_t event_subtype)
{
    uint32_t new_incr;
    switch(event_subtype) {
        case EV_SUBTYPE_ENCODER_PRESSED:
            pubsub.fire(EVENT_VFO, EV_SUBTYPE_ADVANCE_INCR);
            break;
 
        case EV_SUBTYPE_ENCODER_CW:
            pubsub.fire(EVENT_VFO, EV_SUBTYPE_TUNE_CW, ed);
            break;

        case EV_SUBTYPE_ENCODER_CCW:
            pubsub.fire(EVENT_VFO, EV_SUBTYPE_TUNE_CCW, ed);
            break;

        default:
            break;
    }

}

void KNOB::subscriber(event_data ed, uint32_t event_subtype)
{
    switch(mode) {
        case KM_TUNING:
            tuning(ed, event_subtype);
            break;
        
        default:
            break;
    }


}