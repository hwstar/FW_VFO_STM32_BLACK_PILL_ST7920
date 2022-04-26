#ifndef __MENU_HPP__

#define MENU_ATYPE_COMMAND 0
#define MENU_ATYPE_LEVEL_PUSH 1
#define MENU_ATYPE_CAL_U16 2

#define MENU_CAL_RETRIEVE 0
#define MENU_CAL_STORE 1

#define MENU_CS_IDLE 0
#define MENU_CS_ACTIVE 1



// Structure for describing the type of menu item
typedef struct menu_item
{
    const char *menu_text;
    const uint8_t a_type;
    const uint32_t event_type;
    const uint32_t event_subtype;
    const event_data ed;
    const struct menu_level *lower_level;
    const struct menu_cal_item_u16 *cal_lower_level;
} menu_item;

// Structure for a set of menu items
typedef struct menu_level
{
    const uint8_t item_count;
    const char *name;
    const menu_item *items[MAX_MENU_ITEMS];
} menu_level;



typedef struct menu_action_data {
    uint8_t command;
    uint16_t value;
    const char *const_str;
} menu_action_data;


// Structure for 16 bit unsigned integer entry for calibration values
typedef struct menu_cal_item_u16 {
    const char *name;
    const uint16_t min;
    const uint16_t max;
    void (*action_function)(menu_action_data *act_data);
} cal_menu_item;


class MENU
{
    public:
        void reset();
        void draw(uint8_t atype = MENU_ATYPE_LEVEL_PUSH);
        void begin(const menu_level *top);
        bool handler(event_data ed, uint32_t event_subtype);
    
    private:
        void push(const menu_level *existing);
        const menu_level *pop();
        uint8_t cal_state;
        uint8_t selection;
        uint8_t menu_stack_pointer;
        uint16_t cal_increment;
        uint16_t cur_cal_value;
        char extra_info[6];
        const menu_level *menu_stack[MAX_MENU_LEVELS];
        const menu_level *top_level;
        const menu_level *cur_level;
        const cal_menu_item  *cur_cal_item;
};


#define __MENU_HPP__
#endif
