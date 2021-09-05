#include <Arduino.h>
#include <U8g2lib.h>

#include <config.hpp>
#include <display.hpp>
#include <event.hpp>



U8G2_ST7920_128X64_1_HW_SPI st7920(U8G2_R0, PIN_SPI_CS, U8X8_PIN_NONE);

//
// Initialize the display
//

void DISPLAY_DRIVER::begin()
{
    st7920.begin();
    keypad_keys = NULL;

}

//
// Refresh the display
//

void DISPLAY_DRIVER::refresh()
{
    const char *modestr;
    const char *radiostr;
    const char *clear12 = "            ";
    char freqall[20];
    char line40[20];

    

    uint32_t mhz = freq/1000000UL;
    uint32_t modulus = freq % 1000000UL;

    strncpy(line40, (keypad_keys && keypad_keys[0]) ? keypad_keys : clear12, sizeof(line40) - 1) ;

    snprintf(freqall, sizeof(freqall) - 1,"%lu.%06lu", mhz, modulus);

    if(mode == MODE_USB)
        modestr = "USB";
    else
        modestr = "LSB";
    
    switch(tx_mode){
        case RADIO_RX:
            radiostr = "RX";
            break;
        case RADIO_TX:
            radiostr = "TX";
            break;
        case RADIO_TUNE:
            radiostr = "TU";
            break;
        default:
            radiostr = "";
    }

    st7920.firstPage();
    do {
        /* all graphics commands have to appear within the loop body. */    
        st7920.setFont(u8g2_font_ncenB14_tr);
        st7920.drawStr(0, 20, freqall);
        st7920.setFont(u8g2_font_ncenB08_tr);
        st7920.drawStr(100, 20, modestr);
        st7920.drawStr(0,40,line40);
        st7920.drawStr(100, 60, radiostr);
    } while ( st7920.nextPage() );

}

//
// Called when there is something to update
//

void DISPLAY_DRIVER::events(event_data ed, uint8_t event_subtype)
{
    switch(event_subtype){
        case EV_SUBTYPE_SET_FREQ:
            freq = ed.u32_val;
            break;
        case EV_SUBTYPE_SET_MODE:
            mode = ed.u8_val;
            break;
        case EV_SUBTYPE_TX_MODE:
            tx_mode = ed.u8_val;
            break;
        case EV_SUBTYPE_KEYPAD_ENTRY:
            keypad_keys = ed.cp;

            break;
        default:
            break;
    }


}
