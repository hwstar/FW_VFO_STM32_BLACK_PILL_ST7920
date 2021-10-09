#include <Arduino.h>
#include <logger.hpp>
#include <config.hpp>
#include <event.hpp>


bool LOGGER::lookup_error(uint16_t error_code, const char **long_str, const char **short_str )
{
    char res = true;

    if(!long_str || !short_str)
        return false;

    switch(error_code){

        case ERR_NO_BPF:
            *long_str = "No Bandpass Filter Board Detected";
            *short_str = "No BPF I2C Comm";
            break;

        case ERR_NO_LPF:
            *long_str = "No Low Pass Filter Board Detected";
            *short_str = "No LPF I2C Comm";

            
            break;

        case ERR_NO_TRX:
            *long_str = "No TRX Motherboard Detected";
            *short_str = "No TRX I2C Comm";
            break;

        case ERR_NO_CLK_GEN:
            *long_str = "No Clock Generator Detected";
            *short_str = "No Clk Gen I2C Comm";
            break;

        default:
            *long_str = *short_str = NULL;
            res = false;
            break;
    }
    return res;

}


void LOGGER::error(uint16_t error_code)
{
    const char *long_str, *short_str;
    
    if(lookup_error(error_code, &long_str, &short_str))
        Serial1.printf("Error %d: %s\r\n", long_str);
    else
        Serial1.printf("Error %d\r\n", error_code);
       

}

LOGGER logger;