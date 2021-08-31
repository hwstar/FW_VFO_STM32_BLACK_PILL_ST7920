#include <logger.hpp>
#include <config.hpp>


const char *LOGGER::lookup_error(uint16_t error_code)
{
    const char *res = NULL;
    switch(error_code){

        case ERR_NO_BPF:
            res = "No Bandpass Filter Board Detected";
            break;
        case ERR_NO_LPF:
            res = "No Low Pass Filter Board Detected";
            break;

        case ERR_NO_TRX:
            res = "No TRX Motherboard Detected";
            break;

        case ERR_NO_CLK_GEN:
            res = "No Clock Generator Detected";
            break;

        default:
            break;
    }
    return res;

}


void LOGGER::error(uint16_t error_code)
{
    const char *s = lookup_error(error_code);
    if(s)
        Serial1.printf("Error %d\r\n");
    else
        Serial1.printf("Error %d: %s\r\n", s);

}

LOGGER logger;