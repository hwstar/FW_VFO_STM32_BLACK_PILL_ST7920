#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <config.hpp>
#include <event.hpp>
#include <error.hpp>
#include <display.hpp>


#define DISPLAY_NORMAL 0
#define DISPLAY_ERROR 100

#define VALID_STR(s) (s && s[0])

/******************************************
* Definitions
******************************************/

//
// Custom delay callback
//

extern "C" uint8_t u8x8_gpio_and_delay_stm32f411_black_pill(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, U8X8_UNUSED void *arg_ptr);


//
//
// Define custom U8G2 class to use delay callback to fix ST7920 display artifacts
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
// Blank field inserts max_length - 1 spaces and a zero terminator
//


char *DISPLAY_DRIVER::blank_field(char *dest, uint8_t max_length)
{
    int i;
    for(i = 0; i < max_length - 2; i++)
      dest[i] = ' ';
    dest[i] = 0;
    return dest;
}

//
// strncpy_wrapper. Inserts a zero at the end of the string in all cases.
//

char *DISPLAY_DRIVER::strncpy_z(char *dest, const char *source, uint8_t max_length)
{
    strncpy(dest, source, max_length);
    dest[max_length - 1] = 0;
    return dest;
}


//
// Store a new copy of the string in dest_storage if it is not NULL or zero length else return NULL
//


char *DISPLAY_DRIVER::store_string_field(char *dest_storage, const char *source, uint8_t max_length)
{
  uint8_t i;

  if(VALID_STR(source))
    strncpy_z(dest_storage, source, max_length);
  else{
    return NULL;
  }
  return dest_storage;
}

//
// Format frequency as a string from an unsigned 32 bit integer
//

char *DISPLAY_DRIVER::format_frequency(char *dest, uint32_t freq, uint8_t max_length)
{
  uint32_t mhz = freq/1000000UL;
  uint32_t modulus = freq % 1000000UL;
  snprintf(dest, max_length - 1,"%lu.%06lu", mhz, modulus);
  dest[max_length - 1] = 0;

  return dest;

}

//
// Create a string with a prescrbed number of dots.
// String length is N dots + 1
//

char *DISPLAY_DRIVER::gen_dots(char *dots, uint8_t number)
{
  uint8_t i;
  for (i = 0; i < number; i++)
    dots[i] = '.';
  dots[i] = 0;
  return dots;
}
//
// Refresh the display
//

void DISPLAY_DRIVER::refresh_normal_operation()
{
    const char *p_modestr;
    const char *p_radiostr;
    const char *p_agcstr;
    char *p_freq_b_str;
    char *p_command_str;
    char freqall[20];
    char freqall_b[20];
    char commandstr[20];
    char tuning_increment_str[6];
    char meter_value_str[6];
    char dot_str[11];
  

  

    


    // Convert variables to be displayed to strings


    // Command line
  
    p_command_str = store_string_field(commandstr, keypad_keys, sizeof(commandstr));

    // Frequency 
    format_frequency(freqall, freq, sizeof(freqall));
    // Mode

    if(sideband == MODE_USB)
        p_modestr = "USB";
    else
        p_modestr = "LSB";


    // VFO B line (Only display if freq is nonzero)
    if(freq_b)
      p_freq_b_str = format_frequency(freqall_b, freq_b, sizeof(freqall_b));
    else
      p_freq_b_str = NULL;

  

    // Status line

    if(agc_state)
        p_agcstr = "AGC";
    else
        p_agcstr = "";
    
    switch(trx_mode){
        case RADIO_RX:
            p_radiostr = "RX";
            break;
        case RADIO_TX:
            p_radiostr = "TX";
            break;
        case RADIO_TUNE:
            p_radiostr = "TU";
            break;
        default:
            p_radiostr = "";
    }
    if(tuning_increment <= 1000 && tuning_increment != 0){
      if(tuning_increment == 1000)
        strncpy(tuning_increment_str,"1k", 3);
      else
        sprintf(tuning_increment_str, "%d", tuning_increment);
    }
    else
      strncpy(tuning_increment_str, "TIE", 4); // Tuning increment error

    // Format meter value
    if(meter_info.mode == EVMM_SWR){
      uint8_t scaled_value;
      // Clip value to full scale if at full scale value or higher, or less than 0.
      if(meter_info.value > meter_info.full_scale || meter_info.value < 0)
        meter_info.value = meter_info.full_scale;
      scaled_value = (uint8_t) (meter_info.value *(10.0/meter_info.full_scale));
      if(scaled_value > 10)
        scaled_value = 10;
      dtostrf(meter_info.value, 5, 2, meter_value_str);
    }
    else if(meter_info.mode == EVMM_SMETER){
      const char *s_unit_str;
      gen_dots(dot_str, meter_info.value_u16);
      switch(meter_info.peak_value_u16){
        case 3:
          s_unit_str = "S3";
          break;
        case 4:
          s_unit_str = "S4";
          break;
        case 5:
          s_unit_str = "S5";
          break;
        case 6:
          s_unit_str = "S6";
          break;
        case 7:
          s_unit_str = "S7";
          break;
        case 8:
          s_unit_str = "S8";
          break;
        case 9:
          s_unit_str = "S9";
          break;
        case 10:
          s_unit_str = "+10";
          break;
        case 20:
          s_unit_str = "+20";
          break;
        default:
          s_unit_str = "S0";
          break;
      }
      strncpy(meter_value_str, s_unit_str, sizeof(meter_value_str));
      
    }
    else{
      meter_value_str[0] = 0;
    }

    // Update the display

    st7920.firstPage();
    do {
        /* all graphics commands have to appear within the loop body. */    
        // VFO A and mode
        st7920.setFont(u8g2_font_ncenB14_tr);
        st7920.drawStr(0, 20, freqall);
        st7920.setFont(u8g2_font_ncenB08_tr);
        st7920.drawStr(100, 20, p_modestr);
        // VFO B line
        if(VALID_STR(p_freq_b_str))
          st7920.drawStr(0, 30, p_freq_b_str );
        // Meter line
        if(VALID_STR(meter_value_str) && VALID_STR(meter_info.legend)){
            st7920.drawStr(0, 40, meter_info.legend);
            st7920.drawStr(30,40, meter_value_str);
            st7920.drawStr(60,40, dot_str);
        }
        // Command line
        if(VALID_STR(p_command_str))
          st7920.drawStr(0, 50, p_command_str);
        // Status line
        st7920.drawStr(55, 60, tuning_increment_str);
        st7920.drawStr(80, 60, p_agcstr);
        st7920.drawStr(110, 60, p_radiostr);
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
        case EV_SUBTYPE_UPDATE_VFO_B_FREQ:
            freq_b = ed.u32_val;
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
        
        case EV_SUBTYPE_METER_UPDATE:
            meter_info = *((ed_meter_info *) ed.vp);
            break;
            
        default:
            break;
    }


}
