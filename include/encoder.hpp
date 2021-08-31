#ifndef __ENCODER_HPP__
#include <Arduino.h>

#define ENCODER_EVENT_UP 1
#define ENCODER_EVENT_DOWN 2
#define ENCODER_EVENT_SWITCH_DOWN 3
#define ENCODER_EVENT_SWITCH_UP 4


class ENCODER
{
    public:
    void begin(uint8_t gpio_i, uint8_t gpio_q, void (*event_callback)(uint8_t event_type));
    void poll();

    private:

    void interrupt_handler();

    uint8_t i_gpio;
    uint8_t q_gpio;
    uint8_t i_interrupt_num;
    uint8_t q_interrupt_num;

};

#define __ENCODER_HPP__
#endif