#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <menu.hpp>

static ed_menu_info menu_info;

//
// Push a pointer to the upper menu level on the stack so
// we know where to return to
//

void MENU::push(const menu_level *existing)
{
    if(menu_stack_pointer){
        menu_stack[menu_stack_pointer] = existing;
        menu_stack_pointer--;
    }


}

//
// Pop a pointer to the upper level menut from the stack 
//

const menu_level *MENU::pop()
{
    if(menu_stack_pointer < MAX_MENU_LEVELS - 1)
        menu_stack_pointer++;
    return menu_stack[menu_stack_pointer];
}

//
// Reset the menu system to a known state
//

void MENU::reset()
{
    menu_stack_pointer = MAX_MENU_LEVELS - 1;
    selection = 0;
    cur_level = top_level;
}

//
// Draw a menu level
// Builds a data structure
// and sends it to the display module
//

void MENU::draw()
{
    uint8_t i;
    menu_info.selection = selection;
    menu_info.item_count = cur_level->item_count;
    menu_info.menu_name = cur_level->name;
    for(i = 0; i < cur_level->item_count; i++)
        menu_info.items[i] = cur_level->items[i]->menu_text;
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_DISPLAY_MENU, &menu_info);

}

//
// This is called at initialization
// and is used to pass in a pointer to
// the top level menu structure.

void MENU::begin(const menu_level *top)
{
    top_level = top;
    reset();
}

//
// This is where we receive messages from the knob module while in menu mode.
//

bool MENU::handler(event_data ed, uint32_t event_subtype)
{
    bool res = false;
    // Return if a tick
    if(event_subtype == EV_SUBTYPE_TICK_MS || event_subtype == EV_SUBTYPE_TICK_HUNDRED_MS) 
        return res;

    // If a long press message is received
    else if(event_subtype == EV_SUBTYPE_ENCODER_RELEASED_LONG){
        if(cur_level == top_level)
            res = TIM_DIER_UDE; // Exit menu mode
        else{
            // Move up one menu level
            cur_level = pop();
            selection = 0;
            draw();
        }

    }
    else if(event_subtype == EV_SUBTYPE_ENCODER_RELEASED){
        if(cur_level->items[selection]->a_type == MENU_ATYPE_LEVEL_PUSH){
            // Go down one menu level
            push(cur_level);
            cur_level = cur_level->items[selection]->lower_level;
            selection = 0;
            draw();
        }
        else if(cur_level->items[selection]->a_type == MENU_ATYPE_COMMAND){
            // Execute a command using pubsub
            pubsub.fire(cur_level->items[selection]->event_type,
            cur_level->items[selection]->event_subtype,
            cur_level->items[selection]->ed);
            res = true; // Exit menu system
        }
    }
    // If rotate CW event is received
    else if (event_subtype == EV_SUBTYPE_ENCODER_CW){
        if(selection < cur_level->item_count - 1)
            selection++;
        else
            selection = 0; // Wrap around to first selection
        draw();

    }
    // If rotate CCW event is received
    else if (event_subtype == EV_SUBTYPE_ENCODER_CCW){
        if(selection)
            selection--;
        else
            selection = cur_level->item_count - 1; // Wrap around to last selection
        draw();

    }
    
    return res; // Return result

}
    