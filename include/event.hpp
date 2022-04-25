#ifndef __EVENT_HPP__

// Time slot defintiions
#define ETS_METERING 2
#define ETS_KEYPAD 4
#define ETS_SWITCHES 5

// Event mask bits
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


// Event Subtypes
#define EV_SUBTYPE_NONE 0
#define EV_SUBTYPE_SET_FREQ 1
#define EV_SUBTYPE_ADVANCE_INCR 2
#define EV_SUBTYPE_PTT_PRESSED 3
#define EV_SUBTYPE_PTT_RELEASED 4
#define EV_SUBTYPE_TUNE_PRESSED 5
#define EV_SUBTYPE_TUNE_RELEASED 6
#define EV_SUBTYPE_ENCODER_PRESSED 7
#define EV_SUBTYPE_ENCODER_RELEASED 8
#define EV_SUBTYPE_SET_SIDEBAND 9
#define EV_SUBTYPE_TRX_MODE 10
#define EV_SUBTYPE_TUNE_CW 11
#define EV_SUBTYPE_TUNE_CCW 12
#define EV_SUBTYPE_KEYPAD_ENTRY 13
#define EV_SUBTYPE_DISPLAY_NORMAL 14
#define EV_SUBTYPE_TICK_MS 15
#define EV_SUBTYPE_TICK_HUNDRED_MS 16
#define EV_SUBTYPE_ENCODER_CW 17
#define EV_SUBTYPE_ENCODER_CCW 18
#define EV_SUBTYPE_SET_AGC 19
#define EV_SUBTYPE_SET_TXGAIN 20
#define EV_SUBTYPE_STORE_TXGAIN 21
#define EV_SUBTYPE_DISPLAY_ERROR 22
#define EV_SUBTYPE_SET_TUNING_INCREMENT 23
#define EV_SUBTYPE_UPDATE_VFO_B_FREQ 24
#define EV_SUBTYPE_METER_UPDATE 25
#define EV_SUBTYPE_ADVANCE_BAND 26
#define EV_SUBTYPE_ENCODER_RELEASED_LONG 27
#define EV_SUBTYPE_DISPLAY_MENU 28




// Event error codes
#define EV_SUBTYPE_ERR_NO_BPF 0x80000001
#define EV_SUBTYPE_ERR_NO_LPF 0x80000002
#define EV_SUBTYPE_ERR_NO_TRX 0x80000003  
#define EV_SUBTYPE_ERR_NO_CLKGEN 0x80000004
#define EV_SUBTYPE_ERR_EEPROM_PRESENT 0x80000005
#define EV_SUBTYPE_ERR_EEPROM_READ 0x80000006
#define EV_SUBTYPE_ERR_EEPROM_WRITE 0x80000007
#define EV_SUBTYPE_ERR_DAC_PRESENT 0x80000008
#define EV_SUBTYPE_ERR_NO_SECOND_CLKGEN 0x80000009
#define EV_SUBTYPE_ERR_NO_BUS_SWITCH 0x8000000A
#define EV_SUBTYPE_ERR_VFO_EEPROM_PRESENT 0x8000000B
#define EV_SUBTYPE_ERR_VFO_EEPROM_READ 0x8000000C
#define EV_SUBTYPE_ERR_VFO_EEPROM_WRITE 0x8000000D





#define MAX_SUBS 32

typedef union event_data {
    uint8_t u8_val;
    char char_val;
    uint16_t u16_val;
    uint32_t u32_val;
    float float_val;
    void *vp;
    char *cp;
} event_data;

typedef struct event_table {
    void (*callback)(event_data, uint32_t);
    uint32_t filter;
} event_table;

// Meter modes
#define EVMM_CLEAR 0
#define EVMM_SMETER 1
#define EVMM_SWR 2
#define EVMM_TX_POWER 3
#define EVMM_VMON 4

typedef struct ed_meter_info {
    uint8_t mode;
    uint16_t value_u16;
    uint16_t peak_value_u16;
    uint16_t full_scale_u16;
    uint16_t smeter_adc_value;
    uint16_t forward_power_adc_value;
    #ifdef VMON_ADC
    uint16_t vmon_value;
    #endif
    float value;
    float peak_value;
    float full_scale;
    const char *legend;
} ed_meter_info;

typedef struct ed_menu_info {
    uint8_t selection;
    uint8_t item_count;
    const char *menu_name;
    const char *items[MAX_MENU_ITEMS];
} ed_menu_info;


#define ERROR_MAX_LINE_LENGTH 30
typedef struct ed_error_info {
    uint16_t errcode;
    uint16_t errlevel;
    char line_1[ERROR_MAX_LINE_LENGTH];
    char line_2[ERROR_MAX_LINE_LENGTH];
} ed_error_info;

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

extern EVENT pubsub;
extern event_data Time_slot;

#define __EVENT_HPP__
#endif