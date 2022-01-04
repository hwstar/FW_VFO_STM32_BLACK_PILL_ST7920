#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <error.hpp>

void ERROR::begin(void (*event_callback)(uint32_t, uint32_t, event_data))
{
    ev_cb = event_callback;
}

void ERROR::handler(event_data ed, uint32_t event_subtype)
{


}