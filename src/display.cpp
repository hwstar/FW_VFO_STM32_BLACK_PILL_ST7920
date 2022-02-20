#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <config.hpp>
#include <event.hpp>
#include <error.hpp>
#include <display.hpp>


#define DISPLAY_NORMAL 0
#define DISPLAY_ERROR 100

/******************************************
* Definitions
******************************************/

//
// Custom delay callback
//

extern "C" uint8_t u8x8_gpio_and_delay_stm32f411_black_pill(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr);


//
//
// Define custome U8G2 class to use delay callback to fix ST7920 display artifacts
//
//

class U8G2_ST7920_128X64_1_HW_SPI_STM32F411_BLACK_PILL : public U8G2 {
  public: U8G2_ST7920_128X64_1_HW_SPI_STM32F411_BLACK_PILL(const u8g2_cb_t *rotation, uint8_t cs, uint8_t reset = U8X8_PIN_NONE) : U8G2() {
    u8g2_Setup_st7920_s_128x64_1(&u8g2, rotation, u8x8_byte_arduino_hw_spi, u8x8_gpio_and_delay_stm32f411_black_pill);
    u8x8_SetPin_ST7920_HW_SPI(getU8x8(), cs, reset);
  }
};

/******************************************
* Variables
******************************************/

//
// U8G2 Display class instantiation
//
U8G2_ST7920_128X64_1_HW_SPI_STM32F411_BLACK_PILL st7920(U8G2_R0, PIN_SPI_CS, U8X8_PIN_NONE);


/******************************************
* Start of code
******************************************/



//
// STM32F411 GPIO and Delay Callback
//

extern "C" uint8_t u8x8_gpio_and_delay_stm32f411_black_pill(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  uint8_t i;
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
    
      for( i = 0; i < U8X8_PIN_CNT; i++ )
	if ( u8x8->pins[i] != U8X8_PIN_NONE )
	{
	  if ( i < U8X8_PIN_OUTPUT_CNT )
	  {
	    pinMode(u8x8->pins[i], OUTPUT);
	  }
	  else
	  {
#ifdef INPUT_PULLUP
	    pinMode(u8x8->pins[i], INPUT_PULLUP);
#else
	    pinMode(u8x8->pins[i], OUTPUT);
	    digitalWrite(u8x8->pins[i], 1);
#endif 
	  }
	}
	  
      break;

#ifndef __AVR__	
    /* this case is not compiled for any AVR, because AVR uC are so slow */
    /* that this delay does not matter */
    case U8X8_MSG_DELAY_NANO:
      //delayMicroseconds(arg_int==0?0:1);
      delayMicroseconds(5); // Had to add this small delay for STM32F411 black pill to display without artifacts on ST7920
      break;
#endif
    
    case U8X8_MSG_DELAY_10MICRO:
      /* not used at the moment */
      break;
    
    case U8X8_MSG_DELAY_100NANO:
      /* not used at the moment */
      break;
   
    case U8X8_MSG_DELAY_MILLI:
      delay(arg_int);
      break;
    case U8X8_MSG_DELAY_I2C:
      /* arg_int is 1 or 4: 100KHz (5us) or 400KHz (1.25us) */
      delayMicroseconds(arg_int<=2?5:2);
      break;
    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
      if ( arg_int == 0 )
      {
	      pinMode(u8x8_GetPinValue(u8x8, msg), OUTPUT);
	      digitalWrite(u8x8_GetPinValue(u8x8, msg), 0);
      }
      else
      {
#ifdef INPUT_PULLUP
	pinMode(u8x8_GetPinValue(u8x8, msg), INPUT_PULLUP);
#else
	pinMode(u8x8_GetPinValue(u8x8, msg), OUTPUT);
	digitalWrite(u8x8_GetPinValue(u8x8, msg), 1);
#endif 
      }
      break;
    default:
      if ( msg >= U8X8_MSG_GPIO(0) )
      {
	i = u8x8_GetPinValue(u8x8, msg);
	if ( i != U8X8_PIN_NONE )
	{
	  if ( u8x8_GetPinIndex(u8x8, msg) < U8X8_PIN_OUTPUT_CNT )
	  {
	    digitalWrite(i, arg_int);
	  }
	  else
	  {
	    if ( u8x8_GetPinIndex(u8x8, msg) == U8X8_PIN_OUTPUT_CNT )
	    {
	      // call yield() for the first pin only, u8x8 will always request all the pins, so this should be ok
	      yield();
	    }
	    u8x8_SetGPIOResult(u8x8, digitalRead(i) == 0 ? 0 : 1);
	  }
	}
	break;
      }
      
      return 0;
  }
  return 1;
}



//
// Initialize the display
//

void DISPLAY_DRIVER::begin()
{
    st7920.begin();
    keypad_keys = NULL;
    display_mode = DISPLAY_NORMAL;

}

//
// Refresh the display
//

void DISPLAY_DRIVER::refresh_normal_operation()
{
    const char *modestr;
    const char *radiostr;
    const char *agcstr;
    const char *clear12 = "            ";
    char freqall[20];
    char line40[20];
    char tuning_increment_str[6];

    

    uint32_t mhz = freq/1000000UL;
    uint32_t modulus = freq % 1000000UL;

    // Convert variables to be displayed to strings

    strncpy(line40, (keypad_keys && keypad_keys[0]) ? keypad_keys : clear12, sizeof(line40) - 1) ;

    snprintf(freqall, sizeof(freqall) - 1,"%lu.%06lu", mhz, modulus);

    if(sideband == MODE_USB)
        modestr = "USB";
    else
        modestr = "LSB";

    if(agc_state)
        agcstr = "AGC";
    else
        agcstr = "";
    
    switch(trx_mode){
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
    if(tuning_increment <= 1000 && tuning_increment != 0){
      if(tuning_increment == 1000)
        strncpy(tuning_increment_str,"1k", 3);
      else
        sprintf(tuning_increment_str, "%d", tuning_increment);
    }
    else
      strncpy(tuning_increment_str, "TIE", 4); // Tuning increment error


    // Update the display

    st7920.firstPage();
    do {
        /* all graphics commands have to appear within the loop body. */    
        st7920.setFont(u8g2_font_ncenB14_tr);
        st7920.drawStr(0, 20, freqall);
        st7920.setFont(u8g2_font_ncenB08_tr);
        st7920.drawStr(100, 20, modestr);
        st7920.drawStr(0, 40, line40);
        st7920.drawStr(55, 60, tuning_increment_str);
        st7920.drawStr(80, 60, agcstr);
        st7920.drawStr(110, 60, radiostr);
    } while ( st7920.nextPage() );

}

//
// Display error message
//

void DISPLAY_DRIVER::refresh_error_message()
{
  // Update the display
  char errnum_str[20];
  snprintf(errnum_str, sizeof(errnum_str) - 1,"%u", p_err_info->errcode);

  st7920.firstPage();
  do {
      /* all graphics commands have to appear within the loop body. */    
      st7920.setFont(u8g2_font_ncenB14_tr);
      st7920.drawStr(0, 20, "Error");
      st7920.drawStr(100, 20, errnum_str);
      st7920.setFont(u8g2_font_ncenB08_tr);
      st7920.drawStr(0, 40, p_err_info->line_1);
      st7920.drawStr(0, 60, p_err_info->line_2);
  } while ( st7920.nextPage() );


  if(p_err_info->errlevel == ERROR_LEVEL_HARD)
    for(;;); // Loop forever on hard error
}

void DISPLAY_DRIVER::refresh()
{
  switch(display_mode){
    case DISPLAY_NORMAL:
      refresh_normal_operation();
      break;

    case DISPLAY_ERROR:
      refresh_error_message();
      break;

    default:
      refresh_normal_operation();
      break;
  }
}

//
// Called from event.cpp when there is something to update
//

void DISPLAY_DRIVER::events(event_data ed, uint32_t event_subtype)
{
    switch(event_subtype){
        case EV_SUBTYPE_SET_FREQ:
            freq = ed.u32_val;
            break;
        case EV_SUBTYPE_TRX_MODE:
            trx_mode = ed.u8_val;
            break;
        case EV_SUBTYPE_SET_SIDEBAND:
            sideband = ed.u8_val;
            break;
        case EV_SUBTYPE_SET_AGC:
            agc_state = ed.u8_val;
            break;
        case EV_SUBTYPE_KEYPAD_ENTRY:
            keypad_keys = ed.cp;
            break;
        case EV_SUBTYPE_POST_ERROR:
            break;
        case EV_SUBTYPE_TICK_HUNDRED_MS:
            this->refresh(); 
            break;
        case EV_SUBTYPE_DISPLAY_ERROR:
            p_err_info = (ed_error_info *) ed.vp;
            display_mode = DISPLAY_ERROR;
            break;
        case EV_SUBTYPE_SET_TUNING_INCREMENT:
            tuning_increment = ed.u16_val;
            break;
            
        default:
            break;
    }


}
