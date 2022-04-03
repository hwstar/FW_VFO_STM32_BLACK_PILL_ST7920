#include <Arduino.h>
#include <config.hpp>
#include <event.hpp>
#include <keypad.hpp>


void KEYPAD::handler(event_data ed, uint32_t event_subtype)
{

  static uint8_t active_column = 1;
  static uint8_t row_code;
  static uint8_t column_code;
  static uint8_t scan_code;
  static bool last_key_detect;
  bool new_key_detect;
  static const char scan_table[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};

if(event_subtype != EV_SUBTYPE_TICK_MS || ed.u32_val != ten_ms_time_slot)
  return;

  // Check row inputs
  if( digitalRead(PIN_KEYPAD_R1) ){
    new_key_detect = true;
    row_code = 0;
  } else if( digitalRead(PIN_KEYPAD_R2) ){
    new_key_detect = true;
    row_code = 1;
  } else if( digitalRead(PIN_KEYPAD_R3) ){
    new_key_detect = true;
    row_code = 2;
  } else if( digitalRead(PIN_KEYPAD_R4) ){
    new_key_detect = true;
    row_code = 3;
  } else {
      new_key_detect = false;
  }
  // Down transition
  if((true == new_key_detect) && (false == last_key_detect)){
    scan_code = (column_code << 2) + row_code;
    last_key_detect = new_key_detect;
  // Up transition
  } else if((false == new_key_detect) && (true == last_key_detect)){
    last_key_detect = new_key_detect;
    // Fire keypad event
    pubsub.fire(EVENT_KEYPAD_PARSER, EV_SUBTYPE_NONE, scan_table[scan_code]);
  }

  if(!new_key_detect) { // Stop scanning when a key is pressed
    // Select the next column
    digitalWrite(PIN_KEYPAD_C1, 0);
    digitalWrite(PIN_KEYPAD_C2, 0);
    digitalWrite(PIN_KEYPAD_C3, 0);
    digitalWrite(PIN_KEYPAD_C4, 0);

    switch(active_column){
      case 1:
        column_code = 0;
        digitalWrite(PIN_KEYPAD_C1, 1);
        break;

      case 2:
        column_code = 1;
        digitalWrite(PIN_KEYPAD_C2, 1);
        break;

      case 4:
        column_code = 2;
        digitalWrite(PIN_KEYPAD_C3, 1);
        break;

      case 8:
        column_code = 3;
        digitalWrite(PIN_KEYPAD_C4, 1);
        break;
    }
    // Calculate next active column when we run again.
    active_column <<= 1;
    if(active_column & 0x10)
      active_column = 1;
  }
}
