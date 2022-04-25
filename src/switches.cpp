#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <switches.hpp>




void SWITCHES::handler(event_data ed, uint32_t event_subtype)
{

//
// Poll PTT,Tune and Encoder switches
//

    bool ptt,tune, encoder_switch;
    uint8_t press_event;
    static bool last_ptt,last_tune, last_encoder_switch;
    event_data ed_zero;
    


    if(event_subtype == EV_SUBTYPE_TICK_MS){
        if(ed.u32_val == time_slot) { // Only check switches when time slot matches ms_counter on a 10ms interval
            ed_zero.u32_val = 0UL;
            // Read switches
            ptt = !digitalRead(PIN_PTT);
            tune = !digitalRead(PIN_TUNE);
            encoder_switch = !digitalRead(PIN_ENCODER_SWITCH);

            // Track encoder switch down time.
            if(encoder_switch && (encoder_knob_down_timer < LONG_PRESS_TIME))
                encoder_knob_down_timer++;


            // Check for changes

            if(ptt != last_ptt){
                pubsub.fire(EVENT_VFO, (ptt) ? EV_SUBTYPE_PTT_PRESSED : EV_SUBTYPE_PTT_RELEASED, ed_zero );
                last_ptt = ptt;
            }
            if(tune != last_tune){
                pubsub.fire(EVENT_VFO, (tune) ? EV_SUBTYPE_TUNE_PRESSED : EV_SUBTYPE_TUNE_RELEASED, ed_zero );
                last_tune = tune;
            }
            if(encoder_switch != last_encoder_switch){
                // If the switch was just pressed, clear the knob down timer
                // Send the pressed event
                if(encoder_switch == true){
                    encoder_knob_down_timer = 0;
                    pubsub.fire(EVENT_ENCODER_KNOB, EV_SUBTYPE_ENCODER_PRESSED, ed_zero);
                }
                else {
                    // Fire on the desired release event.
                    // 2 possible events.
                    press_event = (encoder_knob_down_timer == LONG_PRESS_TIME) ? EV_SUBTYPE_ENCODER_RELEASED_LONG : EV_SUBTYPE_ENCODER_RELEASED;
                    pubsub.fire(EVENT_ENCODER_KNOB, press_event, ed_zero );
                }
            }
             last_encoder_switch = encoder_switch;
        }
    }
}