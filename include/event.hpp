#ifndef __EVENT_HPP__

#define EVENT_ALL 0xFFFF
#define EVENT_KEYPAD 0x0001
#define EVENT_SERIAL 0x0002
#define EVENT_ENCODER 0x0004
#define EVENT_PTT  0x0008
#define EVENT_VFO_SET 0x0010

#define MAX_SUBS 16

typedef union event_data {
    uint8_t u8_val;
    char char_val;
    uint32_t u32_val;
} event_data;

typedef struct event_table {
    void (*callback)(event_data);
    uint8_t filter;
} event_table;


class EVENT 
{
    public:
    EVENT();

    void fire(uint8_t event_type, char value);
    void fire(uint8_t event_type, uint8_t value);
    void fire(uint8_t event_type, uint32_t value);

    bool subscribe(void (*callback)(event_data ed), uint16_t event_filter = EVENT_ALL);

    private:
    void _fire(uint8_t event_type);
    event_table et[MAX_SUBS];
    event_data ed;
    uint8_t num_subs;
};

#define __EVENT_HPP__
#endif