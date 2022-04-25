#ifndef __MENU_HPP__

#define MENU_ATYPE_COMMAND 0
#define MENU_ATYPE_LEVEL_PUSH 1



typedef struct menu_item
{
    const char *menu_text;
    const uint8_t a_type;
    const uint32_t event_type;
    const uint32_t event_subtype;
    const event_data ed;
    const struct menu_level *lower_level;
} menu_item;


typedef struct menu_level
{
    const uint8_t item_count;
    const char *name;
    const menu_item *items[MAX_MENU_ITEMS];
} menu_level;

class MENU
{
    public:
        void reset();
        void draw();
        void begin(const menu_level *top);
        bool handler(event_data ed, uint32_t event_subtype);
    
    private:
        void push(const menu_level *existing);
        const menu_level *pop();

    
        uint8_t selection;
        uint8_t menu_stack_pointer;
        const menu_level *menu_stack[MAX_MENU_LEVELS];
        const menu_level *top_level;
        const menu_level *cur_level;
        

};


#define __MENU_HPP__
#endif
