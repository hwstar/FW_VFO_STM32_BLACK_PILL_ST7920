#ifndef __EVENT_HPP__

#define EVENT_ALL 0xFFFFFFFF
#define EVENT_KEYPAD 0x00000001
#define EVENT_SERIAL 0x00000002
#define EVENT_ENCODER 0x00000004
#define EVENT_PTT  0x00000008
#define EVENT_VFO  0x00000010

#define EV_SUBTYPE_NONE 0
#define EV_SUBTYPE_SET_FREQ 1
#define EV_SUBTYPE_SET_INCR 2


#define MAX_SUBS 32

typedef union event_data {
    uint8_t u8_val;
    char char_val;
    uint32_t u32_val;
} event_data;

typedef struct event_table {
    void (*callback)(event_data, uint8_t);
    uint32_t filter;
} event_table;


class EVENT 
{
    public:
    EVENT();

    void fire(uint32_t event_type, uint8_t event_subtype, uint8_t value = 0);
    void fire(uint32_t event_type, uint8_t event_subtype, char value = 0);
    void fire(uint32_t event_type, uint8_t event_subtype, uint32_t value = 0);

    bool subscribe(void (*callback)(event_data, uint8_t), uint32_t event_filter = EVENT_ALL);

    private:
    void _fire(uint32_t event_type, uint8_t event_subtype, event_data ed);
    event_table et[MAX_SUBS];
    uint8_t num_subs;
};

#define __EVENT_HPP__
#endif