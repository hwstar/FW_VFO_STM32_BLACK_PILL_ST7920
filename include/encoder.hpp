#ifndef __ENCODER_HPP__

#define ENCODER_EVENT_CW 1
#define ENCODER_EVENT_CCW 2
#define ENCODER_EVENT_SWITCH_DOWN 3
#define ENCODER_EVENT_SWITCH_UP 4


class ENCODER
{
    public:
    void begin(uint8_t gpio_i, uint8_t gpio_q, uint8_t gpio_switch, 
        void (*interrupt_callback)() = NULL, 
        void (*event_callback)(uint8_t event_type) = NULL,
        bool invert_encoder_signals = true, bool invert_switch_signal = true);
    void poll();
    void interrupt_handler();
   
    private:
    
    void (*interrupt_function)();
    void (*callback_function)(uint8_t event_type);
    bool phase_invert;
    bool switch_invert;
   
   
    volatile uint8_t int_iq_state;
    uint8_t last_iq_state;
    uint8_t i_gpio;
    uint8_t q_gpio;
    uint8_t switch_gpio;
    uint8_t i_interrupt_num;
    uint8_t q_interrupt_num;
    uint8_t encoder_error;
    uint8_t encoder_sync_count;
   
    
};

#define __ENCODER_HPP__
#endif