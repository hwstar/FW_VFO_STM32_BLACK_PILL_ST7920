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
    static bool last_ptt,last_tune, last_encoder_switch;
    event_data ed_zero;

    if(event_subtype == EV_SUBTYPE_TICK_MS){
        if(ms_counter < 10)
            ms_counter++;
        else
            ms_counter = 0;
        if(ms_counter == time_slot) { // Only check switches when time slot matches ms_counter on a 10ms interval
            ed_zero.u32_val = 0UL;
            // Read switches
            ptt = !digitalRead(PIN_PTT);
            tune = !digitalRead(PIN_TUNE);
            encoder_switch = !digitalRead(PIN_ENCODER_SWITCH);

            // Check for changes

            if(ptt != last_ptt){
                (*event_fire)(EVENT_VFO, (ptt) ? EV_SUBTYPE_PTT_PRESSED : EV_SUBTYPE_PTT_RELEASED, ed_zero );
                last_ptt = ptt;
            }
            if(tune != last_tune){
                (*event_fire)(EVENT_VFO, (tune) ? EV_SUBTYPE_TUNE_PRESSED : EV_SUBTYPE_TUNE_RELEASED, ed_zero );
                last_tune = tune;
            }
            if(encoder_switch != last_encoder_switch){
                (*event_fire)(EVENT_ENCODER_KNOB, ((encoder_switch) ? EV_SUBTYPE_ENCODER_PRESSED : EV_SUBTYPE_ENCODER_RELEASED), ed_zero );
                last_encoder_switch = encoder_switch;
            }
        }
    }
}