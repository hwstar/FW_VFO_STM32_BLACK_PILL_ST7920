
#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <cmdparse.hpp>


static bool parse_base_10_num(char digit, uint8_t digit_index, uint8_t max_digits, uint16_t &result)
{

    if(1 == digit_index){
        result = 0;
    }

    if(isdigit(digit)){
        result = result * 10;
        result = result + (digit - 0x30);
    }
    else
        return true;
    
    if(digit_index >= max_digits)
        return true;

    return false;
}

// Set agc on or off

static bool cmd_agc_set(char *command_characters, uint8_t command_character_count)
{

    if (1 == command_character_count)
    {
        event_data ed;
        ed.u8_val = (command_characters[0] == '1'); // 1 = AGC on.
        pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_AGC, ed);
        return true;
    }
    else
        return false;
}

// Set tx gain

static bool cmd_txgain_set(char *command_characters, uint8_t command_character_count)
{
    static uint16_t num;

    if (parse_base_10_num(command_characters[command_character_count - 1], command_character_count, 4, num))
    {
        event_data ed;
        ed.u16_val = num;
        pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_TXGAIN, ed);
        return true;
    }
    else
        return false;
}

static bool cmd_mode_usb(char *command_characters, uint8_t command_character_count)
{
    event_data ed;
    ed.u8_val = MODE_USB;
    pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_SIDEBAND, ed);
    return true;

}

static bool cmd_mode_lsb(char *command_characters, uint8_t command_character_count)
{
    event_data ed;
    ed.u8_val = MODE_LSB;
    pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_SIDEBAND, ed);
    return true;
}

//
// Parse the command digits one at a time
//

bool CMDPARSE::parse_command(char *keypad_string, uint8_t keypad_string_index)
{
    static const kp_commands command_table[] = {
        {"242", "AGC", cmd_agc_set, true},
        {"572", "LSB", cmd_mode_lsb, false},
        {"872", "USB", cmd_mode_usb, false},
        {"894", "TXG", cmd_txgain_set, true},
        {"", ""}
        };
    bool res = false;
    static char command_string[16];
    uint8_t command_string_index;
    static const kp_commands *p_cte;
    static uint8_t cte_cmd_len;

    if (keypad_string_index < 2)
        return false; // Must have prefix and at least 1 command digit

    // Make a copy of the keypad string without the command prefix character
    strncpy(command_string, keypad_string + 1, sizeof(command_string));
    command_string[sizeof(command_string) - 1] = 0; // Zero the last buffer byte.
    command_string_index = keypad_string_index - 1; // Create a new index value for the command

    if (!cp_match)
    {
        for (p_cte = (&command_table[0]); (cte_cmd_len = strlen(p_cte->c_num)); p_cte++)
        {
            if (cte_cmd_len == command_string_index)
            {
                if (!strncmp(p_cte->c_num, command_string, cte_cmd_len))
                {
                    cp_match = true;
                    break;
                }
            }
        }
        if(cp_match && (false == p_cte->param_required)){ 
            // No parameter reqired
            (*p_cte->cmd_function)(command_string + cte_cmd_len, command_string_index - cte_cmd_len);
            return true; // Done with this command
        }
            

    }
    else
    {
        res = (*p_cte->cmd_function)(command_string + cte_cmd_len, command_string_index - cte_cmd_len);
    }

    return res;
}

void CMDPARSE::reset()
{
    f = m = 0;
    f_count = 0;
    state = 0;
    cp_match = false;

    keypad_digits_index = 0;

    memset(keypad_digits, 0, sizeof(keypad_digits));
    k_ed.cp = NULL;
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_KEYPAD_ENTRY, k_ed);
}

//
// Parse incoming command tokens
//

void CMDPARSE::handler(event_data ed, uint32_t event_subtype)
{

    char c = ed.char_val;
    uint8_t i;

    if (EV_SUBTYPE_TICK_MS == event_subtype)
        return; // 1 mS not used

    // Reset command timer for each keypress
    if (EV_SUBTYPE_NONE == event_subtype)
        command_timer = CONFIG_COMMAND_TIMEOUT;

    // Force reset after a timeout if state is non-zero
    if (EV_SUBTYPE_TICK_HUNDRED_MS == event_subtype)
    {
        if (state && command_timer)
        {
            command_timer--;
            if (!command_timer)
            {
                reset();
            }
        }
        return;
    }

    // Buffer the keypad digits
    if (keypad_digits_index < sizeof(keypad_digits) - 1)
    {
        keypad_digits[keypad_digits_index++] = c;
        keypad_digits[keypad_digits_index] = 0;
    }
    else // Keypad buffer full, discard the command
        reset();

    //
    // State machine for command parser
    //

    switch (state)
    {
    case 0: // Start of parsing, first character
        if (isdigit(c))
        {
            m = (uint32_t)(c - '0');
            state = 1;
        }
        else if ('B' == c){
            pubsub.fire(EVENT_VFO, EV_SUBTYPE_ADVANCE_BAND);
            reset();
        }
        else if ('C' == c)
        { // Command prefix?
            state = 100;
        }
        else{
            reset();
        }   
        break;

        //
        // Frequency entry states
        //

    case 1: // Second frequency digit or decimal point
        if (isdigit(c))
        {
            m *= 10;
            m += (uint32_t)(c - '0');
            state = 2;
        }
        else if ((c == '*') || (c == '.'))
        { // Decimal point or star?
            state = 3;
        }
        else
        { // Abort
            reset();
        }
        break;

    case 2: // Expect star or decimal point
        if ((c == '*') || (c == '.'))
        {
            state = 3;
        }
        else
        {
            reset();
        }
        break;

    case 3: // Start parsing fractional part
        if (isdigit(c))
        {
            f *= 10;
            f += (uint32_t)(c - '0');
        }
        if ((f_count == 6) || (c == '#') || (c == '\r'))
        { // Check for complete frequency entry
            if ((c == '#') || (c == '\r'))
            { // Short entry by user
                // Multiply by 10 accordingly
                for (i = 0; i < 6 - f_count; i++)
                {
                    f *= 10;
                }
                // Calculate the frequency in Hz
                vf = (m * 1000000) + f;
                // Set the VFO frequency
                pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_FREQ, vf);
                reset();
            }
        }
        if (isdigit(c))
        {
            f_count++;
        }
        else if ((c != '#') || (c == '\r'))
        {
            reset();
        }
        break;

        //
        // Command Entry state
        //

    case 100:
        if (c == '#') // # aborts the command
            reset();
        else
        {
            if (parse_command(keypad_digits, keypad_digits_index))
            {
                reset(); // Command completed
            }
        }
        break;

    default:
        reset();
    }

    //
    // This code handles updating the display
    //

    if (state != 0)
    {
        k_ed.cp = keypad_digits;
    }
    else
    {
        k_ed.cp = NULL;
        command_timer = 0;
    }
    // Update display
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_KEYPAD_ENTRY, k_ed);
}
