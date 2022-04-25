#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <menu.hpp>
#include <knob.hpp>


extern MENU menu;

void KNOB::tuning(event_data ed, uint32_t event_subtype)
{
    uint32_t new_incr;
    switch(event_subtype) {
        case EV_SUBTYPE_ENCODER_RELEASED:
            pubsub.fire(EVENT_VFO, EV_SUBTYPE_ADVANCE_INCR);
            break;

        case EV_SUBTYPE_ENCODER_RELEASED_LONG:
            // Select menu mode for knob
            mode = KM_MENU;
            // Draw the top level menu
            menu.draw();
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

        case KM_MENU:
            if(menu.handler(ed, event_subtype)){
                // Knob mode back to tuning
                mode = KM_TUNING;
                // Stop displaying a menu
                pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_DISPLAY_NORMAL);
                // Reset the menu system
                menu.reset();
            }
            break;
        
        default:
            break;
    }


}