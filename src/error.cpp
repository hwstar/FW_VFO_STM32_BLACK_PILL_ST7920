#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <error.hpp>




typedef struct error_table_entry {
    const uint16_t error_code;
    const uint16_t error_level;
    const char *error_line_1;
    const char *error_line_2;

} error_table_entry;


static const error_table_entry error_table[] = {
    {1, ERROR_LEVEL_HARD, "No BPF board detected", "Check control conn."},
    {2, ERROR_LEVEL_HARD, "No LPF board detected", "Check control conn."},
    {3, ERROR_LEVEL_HARD, "No TRX board detected", "Check control conn."},
    {4, ERROR_LEVEL_HARD, "No SI5351 detected ","VFO H/W problem"},
    {5, ERROR_LEVEL_HARD, "TRX EEPROM not found", "TRX H/W problem"},
    {6, ERROR_LEVEL_HARD, "TRX EEPROM read error", "TRX H/W problem"},
    {7, ERROR_LEVEL_HARD, "TRX EEPROM write error", "TRX H/W problem"},
    {8, ERROR_LEVEL_HARD, "TRX DAC not present", "TRX H/W problem"},
    {0, ERROR_LEVEL_HARD, "Unhandled Error", "F/W bug"} // End sentinal
};


void ERROR_HANDLER::begin()
{
    hard_error = false;
}

void ERROR_HANDLER::handler(event_data ed, uint32_t event_subtype)
{
    static ed_error_info errinfo;
    uint16_t i;
    
    // If hard error posted previously, abort
    // If the MSB of the event subtype isn't set, abort.

    if(hard_error || (event_subtype & 0x80000000 == 0))
        return; 

    // Strip off the msb of the subtype to to get the error code.

    uint16_t error_code = (uint16_t) event_subtype & 0xFFFF;

    // Look for a match in the error table
    for(i = 0 ; error_table[i].error_code ; i++){
        if(error_code == error_table[i].error_code)
            break;
    }

    // Prepare display data
    
    errinfo.errcode = error_code;
    errinfo.errlevel = error_table[i].error_level;
    strncpy(errinfo.line_1,error_table[i].error_line_1, ERROR_MAX_LINE_LENGTH);
    errinfo.line_1[ERROR_MAX_LINE_LENGTH - 1] = 0;
    strncpy(errinfo.line_2,error_table[i].error_line_2, ERROR_MAX_LINE_LENGTH);
    errinfo.line_2[ERROR_MAX_LINE_LENGTH - 1] = 0;

    // Send the error to the display subscribers
    pubsub.fire(EVENT_DISPLAY, EV_SUBTYPE_DISPLAY_ERROR, (void *) &errinfo);

    // If it was a hard error, note that in case we are called with other errors later
    if(error_table[i].error_level == ERROR_LEVEL_HARD)
        hard_error = true;
}


