#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <menu.hpp>

static ed_menu_info menu_info;
static ed_cal_menu_info cal_menu_info_u16;


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
    cal_state = MENU_CS_IDLE;
    cur_level = top_level;
}

//
// Draw a menu level
// Builds a data structure
// and sends it to the display module
//

void MENU::draw(uint8_t atype)
{
    uint8_t i;
    switch(atype){
        case MENU_ATYPE_LEVEL_PUSH: // For a menu entry
            menu_info.selection = selection;
            menu_info.item_count = cur_level->item_count;
            menu_info.menu_name = cur_level->name;
            for(i = 0; i < cur_level->item_count; i++)
                menu_info.items[i] = cur_level->items[i]->menu_text;
            pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_DISPLAY_MENU, &menu_info);
            break;

        case MENU_ATYPE_CAL_U16: // For calibration menu entry
            cal_menu_info_u16.extra_info = extra_info;
            cal_menu_info_u16.name = cur_cal_item->name;
            cal_menu_info_u16.value = cur_cal_value;
            cal_menu_info_u16.increment = cal_increment;
            pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_DISPLAY_CAL_VALUE, &cal_menu_info_u16);
            break;
        default:
            break;
    }
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
    menu_action_data act_data;
    // Return if a tick
    if(event_subtype == EV_SUBTYPE_TICK_MS || event_subtype == EV_SUBTYPE_TICK_HUNDRED_MS) 
        return res;

    // If a long press message is received
    else if(event_subtype == EV_SUBTYPE_ENCODER_RELEASED_LONG){
        if(cur_level == top_level)
            res = true; // Exit menu mode
        else{
            if(cal_state == MENU_CS_ACTIVE){
                // Exit menu mode
                res = true;
            }
            // Move up one menu level
            cal_state = MENU_CS_IDLE;
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
        else if(cur_level->items[selection]->a_type == MENU_ATYPE_CAL_U16){
            if(cal_state == MENU_CS_IDLE){
                // Retrive current value to start from
                push(cur_level); // Extra push so we can pop the same thing on a long press.
                cal_state = MENU_CS_ACTIVE;
                cal_increment = 100;
                cur_cal_item = cur_level->items[selection]->cal_lower_level;
                extra_info[0] = 0;
                act_data.command = MENU_CAL_RETRIEVE;
                act_data.const_str = NULL;
                cur_cal_item->action_function(&act_data);
                cur_cal_value = act_data.value;
                if(act_data.const_str){
                    strncpy(extra_info, act_data.const_str, sizeof(extra_info));
                    extra_info[sizeof(extra_info) - 1] = 0;
                }
                //clip to minimum
                if(cur_cal_value < cur_cal_item->min){
                    cur_cal_value = cur_cal_item->min;
                    act_data.command = MENU_CAL_SET;
                    act_data.value = cur_cal_value;
                    cur_cal_item->action_function(&act_data);
                }   
                // clip to maximum
                else if (cur_cal_value > cur_cal_item->max){
                    cur_cal_value = cur_cal_item->max;
                    act_data.command = MENU_CAL_SET;
                    act_data.value = cur_cal_value;
                    cur_cal_item->action_function(&act_data);
                }

                draw(MENU_ATYPE_CAL_U16); // Special case for cal value entry
            }
            // Else in cal, so change cal increment here
            else {
                if(cal_increment == 100){
                    cal_increment = 10;
                }
                else if (cal_increment == 10){
                    cal_increment = 1;
                }
                else{
                    cal_increment = 100;
                }
                draw(MENU_ATYPE_CAL_U16);
            }
        }
    }
    // If rotate CW event is received
    else if (event_subtype == EV_SUBTYPE_ENCODER_CW){
        if(cal_state == MENU_CS_IDLE){
            if(selection < cur_level->item_count - 1)
                selection++;
            else
                selection = 0; // Wrap around to first selection
            draw();
        }
        // Else in cal so increment cal value here
        else {
            uint16_t new_val = cal_increment + cur_cal_value;
            cur_cal_value = (new_val > cur_cal_item->max) ? cur_cal_item->max : new_val;
            act_data.command = MENU_CAL_SET;
            act_data.value = cur_cal_value;
            cur_cal_item->action_function(&act_data);
            draw(MENU_ATYPE_CAL_U16);
        }
    }
    // If rotate CCW event is received
    else if (event_subtype == EV_SUBTYPE_ENCODER_CCW){
        if(cal_state == MENU_CS_IDLE){
            if(selection)
                selection--;
            else
                selection = cur_level->item_count - 1; // Wrap around to last selection
            draw();
        }
        // Else in cal, so decrement cal value here
        else {
            int32_t new_val = ((int32_t) cur_cal_value) - cal_increment;
            cur_cal_value =(new_val < cur_cal_item->min) ? cur_cal_item->min : (uint16_t) new_val;
            act_data.command = MENU_CAL_SET;
            act_data.value = cur_cal_value;
            cur_cal_item->action_function(&act_data);
            draw(MENU_ATYPE_CAL_U16);
        }
    }
    
    return res; // Return result

}
    