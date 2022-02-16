#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <txprotect.hpp>


//
// Event handler
//

void TX_PROTECT::handler(event_data ed, uint32_t event_subtype)
{
    switch(event_subtype){

        case EV_SUBTYPE_TICK_HUNDRED_MS:
            // If Fan timer is nonzero, increment or decrement its value
            if(fan_timer != 0  || fan_timer_increment > 0){
                fan_timer += fan_timer_increment;
            }
            // Test for rollover when counting up
            if(fan_timer_increment > 0 && fan_timer == 0)
                fan_timer = 0xFFFF; // Clip at maxint

            // Turn on the fan at the timer threshold
            if(fan_timer > CONFIG_TX_FAN_THRESH_SEC*10){
                digitalWrite(PIN_PA_FAN_ENABLE, 1);
            }

            // Turn off the fan when the timer reaches 0
            if(fan_timer == 0)
                digitalWrite(PIN_PA_FAN_ENABLE, 0);

            break;

        case EV_SUBTYPE_PTT_PRESSED:
        case EV_SUBTYPE_TUNE_PRESSED:
            fan_timer_increment = 1;
            reference_count++;
            break;

        case EV_SUBTYPE_PTT_RELEASED:
        case EV_SUBTYPE_TUNE_RELEASED:
            if(reference_count)
                reference_count--;
            if(reference_count == 0)
                fan_timer_increment = -1;
            break;


        default:
            break;
    }

}