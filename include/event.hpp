#ifndef __EVENT_HPP__

#define EVENT_ALL 0xFFFFFFFF
#define EVENT_KEYPAD_PARSER 0x00000001
#define EVENT_SERIAL 0x00000002
#define EVENT_ENCODER 0x00000004
#define EVENT_PTT  0x00000008
#define EVENT_VFO  0x00000010
#define EVENT_DISPLAY 0x00000020
#define EVENT_ENCODER_KNOB 0x00000040
#define EVENT_SWITCHES 0x00000080
#define EVENT_TICK 0x40000000
#define EVENT_ERROR 0x80000000

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
#define EV_SUBTYPE_TICK_MS 15
#define EV_SUBTYPE_TICK_HUNDRED_MS 16
#define EV_SUBTYPE_ENCODER_CW 17
#define EV_SUBTYPE_ENCODER_CCW 18

#define EV_SUBTYPE_ERR_NO_BPF 0x80000001
#define EV_SUBTYPE_ERR_NO_LPF 0x80000002
#define EV_SUBTYPE_ERR_NO_TRX 0x80000003  
#define EV_SUBTYPE_ERR_NO_CLKGEN 0x80000004

#define MAX_SUBS 32

typedef union event_data {
    uint8_t u8_val;
    char char_val;
    uint32_t u32_val;
    void *vp;
    char *cp;
} event_data;

typedef struct event_table {
    void (*callback)(event_data, uint32_t);
    uint32_t filter;
} event_table;


class EVENT 
{
    public:
    EVENT();

    void fire(uint32_t event_type, uint32_t event_subtype, uint8_t value);
    void fire(uint32_t event_type, uint32_t event_subtype, char value);
    void fire(uint32_t event_type, uint32_t event_subtype, uint32_t value);
    void fire(uint32_t event_type, uint32_t event_subtype, event_data ed);
    void fire(uint32_t event_type, uint32_t event_subtype);
    void fire(uint32_t event_type, uint32_t event_subtype, char *value);
    void fire(uint32_t event_type, uint32_t event_subtype, void *value);


    bool subscribe(void (*callback)(event_data, uint32_t), uint32_t event_filter = EVENT_ALL);

    private:
   
    event_table et[MAX_SUBS];
    uint8_t num_subs;
};

#define __EVENT_HPP__
#endif