#ifndef __EVENT_HPP__

#define EVENT_ALL 0xFFFFFFFF
#define EVENT_KEYPAD 0x00000001
#define EVENT_SERIAL 0x00000002
#define EVENT_ENCODER 0x00000004
#define EVENT_PTT  0x00000008
#define EVENT_VFO  0x00000010
#define EVENT_DISPLAY 0x00000020
#define EVENT_SWITCH 0x00000040

#define EV_SUBTYPE_NONE 0
#define EV_SUBTYPE_SET_FREQ 1
#define EV_SUBTYPE_SET_INCR 2
#define EV_SUBTYPE_PTT_PRESSED 3
#define EV_SUBTYPE_PTT_RELEASED 4
#define EV_SUBTYPE_TUNE_PRESSED 5
#define EV_SUBTYPE_TUNE_RELEASED 6
#define EV_SUBTYPE_ENCODER_PRESSED 7
#define EV_SUBTYPE_ENCODER_RELEASED 8
#define EV_SUBTYPE_SET_MODE 9
#define EV_SUBTYPE_TX_MODE 10
#define EV_SUBTYPE_TUNE_CW 11
#define EV_SUBTYPE_TUNE_CCW 12
#define EV_SUBTYPE_KEYPAD_ENTRY 13
#define EV_SUBTYPE_POST_ERROR 14



#define MAX_SUBS 32

typedef union event_data {
    uint8_t u8_val;
    char char_val;
    uint32_t u32_val;
    char *cp;
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
    void fire(uint32_t event_type, uint8_t event_subtype, event_data ed);

    bool subscribe(void (*callback)(event_data, uint8_t), uint32_t event_filter = EVENT_ALL);

    private:
   
    event_table et[MAX_SUBS];
    uint8_t num_subs;
};

#define __EVENT_HPP__
#endif