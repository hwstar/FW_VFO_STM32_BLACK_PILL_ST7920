#include <Arduino.h>
#include <event.hpp>
#include <encoder.hpp>


//
// Interrupt handler
// Called from interrupt_function given in begin
//

void ENCODER::interrupt_handler()
{
    int_iq_state = ((((uint8_t) digitalRead(q_gpio)) << 1)|((uint8_t) digitalRead(i_gpio)));
}


//
// Initialization
//
void ENCODER::begin(uint8_t gpio_i, uint8_t gpio_q, uint8_t gpio_switch, 
        void (interrupt_callback)(), 
        void (*event_callback)(uint32_t, uint8_t, event_data),
        bool invert_encoder_signals, bool invert_switch_signal)
{
    // Save args
    i_gpio = gpio_i;
    q_gpio = gpio_q;
    switch_gpio = gpio_switch;
    phase_invert = invert_encoder_signals;
    switch_invert = invert_switch_signal;
    interrupt_function = interrupt_callback;
    callback_function = event_callback;
    encoder_sync_count = encoder_error = 0;
    
    // Set up encoder interrupt
    i_interrupt_num = digitalPinToInterrupt(i_gpio);
    q_interrupt_num = digitalPinToInterrupt(q_gpio);

    attachInterrupt(i_interrupt_num, interrupt_callback, CHANGE);
    attachInterrupt(q_interrupt_num, interrupt_callback, CHANGE);

}

void ENCODER::poll()
{
    uint8_t curr_iq_state;
    uint8_t encoder_event = 0;
    
    event_data ed;
    ed.u32_val = 0UL;

    noInterrupts();
    curr_iq_state = int_iq_state;
    interrupts();


    // *** ENCODER ***
    // Set inverted phase if configured
    curr_iq_state = (phase_invert) ? ((~curr_iq_state) & 3) : curr_iq_state & 3;

    if(curr_iq_state != last_iq_state){
        switch(last_iq_state){
            case 0:
                if(1 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CW;
                else if(2 ==curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CCW;
                break;

            case 1:
                if(3 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CW;
                else if(0 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CCW;
                break;

            case 3:
                if(2 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CW;
                else if(1 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CCW;
                break;
            
            case 2:
                if(0 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CW;
                else if(3 == curr_iq_state)
                    encoder_event = EV_SUBTYPE_ENCODER_CCW;
                break;
        }
        // Throw away transitions until we have moved at least 5 positions initially.
        if(encoder_sync_count < 5)
            encoder_sync_count++;
        else if((!encoder_event) & (encoder_error < 255)){
            //Serial1.println("Encoder Error");
            encoder_error++;
        }

        if(encoder_event && (encoder_sync_count > 4) && callback_function){
            event_data ed;
            (*callback_function)(EVENT_ENCODER, encoder_event, ed);
        }
        last_iq_state = curr_iq_state;
    }

}

//
// Handle Encoder Event
//

void ENCODER::handler(event_data ed, uint8_t event_subtype)
{

        switch(event_subtype){
            case EV_SUBTYPE_TICK_MS:
                this->poll();
                break;
            case EV_SUBTYPE_ENCODER_CW:
                callback_function(EVENT_VFO, EV_SUBTYPE_TUNE_CW, ed);
                break;
            case EV_SUBTYPE_ENCODER_CCW:
                callback_function(EVENT_VFO, EV_SUBTYPE_TUNE_CCW, ed);
                break;

            default:
                break;
        }

}