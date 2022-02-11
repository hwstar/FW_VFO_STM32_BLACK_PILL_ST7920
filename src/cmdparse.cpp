
#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <cmdparse.hpp>


//
// Throwaway code to test AGC on/off
//

bool keypad_agc_set(char *command_string, uint8_t command_string_index)
{
  if(4 == command_string_index){
    event_data ed;
    ed.u8_val = (command_string[3] == '1');
    pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_AGC, ed);
    return true;
  }
  else
    return false;
}

typedef struct kp_commands {
  char c_num[5];
  char c_str[11];
  bool (*cmd_function)(char *command_string, uint8_t command_string_index);
} kp_commands;

bool keypad_parse_command(char *command_string, uint8_t command_string_index, bool *cp_match)
{
  static kp_commands command_table[] = {
  {"242","AGC", keypad_agc_set},
  {"",""}
  };
  bool res = false;

  static kp_commands *p_cte;
  uint8_t cte_cmd_len;

  if(!*cp_match){
    for(p_cte = &command_table[0]; (cte_cmd_len = strlen(p_cte->c_num)); p_cte++){
      if(cte_cmd_len == command_string_index){
        if(!strncmp(p_cte->c_num, command_string, cte_cmd_len)){
          *cp_match = true;
          break;
        }
      }
    }
  }
  else {
    res = (*p_cte->cmd_function)(command_string, command_string_index);
  }

  return res;
}

//
// End of throwaway code
//


//
// Parse incoming command tokens
//
void CMDPARSE::handler(event_data ed, uint32_t event_subtype)
{

    char c = ed.char_val;
    event_data k_ed;
    uint8_t i;
    static uint8_t keypad_digits_index;
    static uint8_t command_string_index;
    static bool cp_match;
    static uint8_t state = 0;
    static uint8_t f_count;
    static uint32_t m,f,vf;
    static char keypad_digits[16];
    static char command_string[16];

    if(EV_SUBTYPE_TICK_MS == event_subtype)
        return; // 1 mS not used
    if(EV_SUBTYPE_TICK_HUNDRED_MS == event_subtype)
        return; // Reserved for parser timers in the future

    switch(state){
        case 0: // Start of parsing, first character
            f = m = 0;
            f_count = 0;
            if(isdigit(c)){
                m = (uint32_t) (c - '0');
                state = 1;
            }
            else if ('C' == c) { // Command?
                cp_match = false;
                command_string_index = 0;
                command_string[0] = 0;
                state = 100;
            }
            break;

        case 1: // Second frequency digit or decimal point
            if(isdigit(c)){
                m *= 10;
                m += (uint32_t) (c - '0');
                state = 2;
            } else if ((c == '*')||(c == '.')){ // Decimal point or star?
                state = 3;
            
            } else { // Abort
                state = 0;
            }
            break;

        case 2: // Expect star or decimal point
            if((c == '*')||(c == '.')){
                state = 3;
            } else {
                state = 0;
            }
            break;

        case 3: // Start parsing fractional part
        if(isdigit(c)){
            f *= 10;
            f += (uint32_t) (c - '0');
        }
        if((f_count == 6) || (c == '#') || (c == '\r')){ // Check for complete frequency entry
            if((c == '#')||(c == '\r')){ // Short entry by user
                // Multiply by 10 accordingly
                for(i = 0 ; i < 6 - f_count; i++){
                    f *= 10;
                }
                // Calculate the frequency in Hz
                vf = (m * 1000000) + f;
                // Set the VFO frequency
                pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_FREQ, vf);
                state = 0;
            }
        }
        if(isdigit(c)){
            f_count++;
        } else if((c != '#')||(c == '\r')) {
            state = 0;
        }
        break;

        //
        // Command state
        //

        case 100: 
            if(c == '#') // # aborts the command
                state = 0;
            else{
                command_string[command_string_index++] = c;
                command_string[command_string_index] = 0;
                if(command_string_index == sizeof(command_string) - 1)
                    state = 0;
                else{
                    if(keypad_parse_command(command_string, command_string_index, &cp_match))
                        state = 0;
                }
            }
            break;

        default:
            state = 0;
    }

    //
    // This code handles echoing the keypad digits to the display
    //

    if(state != 0){
        if( keypad_digits_index < sizeof(keypad_digits) - 1){}
        keypad_digits[keypad_digits_index++] = c;
        k_ed.cp = keypad_digits;
    } else {
        // Done or error
        memset(keypad_digits, 0, sizeof(keypad_digits));
        keypad_digits_index = 0;
        k_ed.cp = NULL;
    }
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_KEYPAD_ENTRY, k_ed);
}
