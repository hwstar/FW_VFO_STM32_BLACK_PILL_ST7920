#include <Arduino.h>
#include <TaskScheduler.h>


#include <config.hpp>
#include <vfo.hpp>
#include <logger.hpp>
#include <encoder.hpp>
#include <event.hpp>
#include <display.hpp>




// Scheduler object
Scheduler ts;

// Polling tasks
void task_poll_io();
void task_poll_encoder();
void task_refresh_display();
Task itask(1, -1, &task_poll_io, &ts, true);
Task etask(1, -1, &task_poll_encoder, &ts, true);
Task dtask(100, -1, &task_refresh_display, &ts, true);

// Event object
EVENT event;
// VFO object
VFO vfo;
// Encoder object
ENCODER encoder;
// Display object
DISPLAY_DRIVER display;


//
// Arduino setup function
//

void setup() {


  // Wait for power rails to stabilize before doing anything
  delay(100);


  // INPUTS
  pinMode(PIN_PTT, INPUT_PULLUP);
  pinMode(PIN_TUNE, INPUT_PULLUP);
  pinMode(PIN_ENCODER_I, INPUT_PULLUP);
  pinMode(PIN_ENCODER_Q, INPUT_PULLUP);
  pinMode(PIN_ENCODER_SWITCH, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_R1, INPUT_PULLDOWN);
  pinMode(PIN_KEYPAD_R2, INPUT_PULLDOWN);
  pinMode(PIN_KEYPAD_R3, INPUT_PULLDOWN);
  pinMode(PIN_KEYPAD_R4, INPUT_PULLDOWN);

  // Outputs
  pinMode(PIN_STM32_LED, OUTPUT);
  digitalWrite(PIN_STM32_LED, 1);
  
  pinMode(PIN_PA_FAN_ENABLE, OUTPUT);
  digitalWrite(PIN_PA_FAN_ENABLE, 0);

  pinMode(PIN_KEYPAD_C1, OUTPUT);
  pinMode(PIN_KEYPAD_C2, OUTPUT);
  pinMode(PIN_KEYPAD_C3, OUTPUT);
  pinMode(PIN_KEYPAD_C4, OUTPUT);
  

  
  // Initialize serial port
  Serial1.begin(115200);
  Serial1.setTimeout(10000);
  
  // Initialize Encoder
  void encoder_interrupt_handler();
  void encoder_callback(uint8_t event_type);
  encoder.begin(PIN_ENCODER_I, PIN_ENCODER_Q, PIN_ENCODER_SWITCH, [] () { encoder.interrupt_handler(); }, encoder_callback);

  // Initialize display object
  display.begin();

  // Add subscribers to the event object
  void encoder_subscriber(event_data, uint8_t);
  void switch_subscriber(event_data, uint8_t);
  void keypad_subscriber(event_data, uint8_t);
  void vfo_subscriber(event_data, uint8_t);
  void display_subscriber(event_data, uint8_t);
  void serial_output_subscriber(event_data, uint8_t );
  event.subscribe(encoder_subscriber, EVENT_ENCODER);
  event.subscribe(switch_subscriber, EVENT_SWITCH);
  event.subscribe(keypad_subscriber, EVENT_KEYPAD|EVENT_SERIAL);
  event.subscribe(vfo_subscriber, EVENT_VFO);
  event.subscribe(display_subscriber, EVENT_DISPLAY);
  event.subscribe(serial_output_subscriber, EVENT_DISPLAY);
  
  // Initialize vfo object
  // Events must be initialzed first for default freqency and mode to be displayed.
  void vfo_fire_event(uint32_t event_type, uint8_t event_subtype, event_data ed);
  if(!vfo.begin(14250000UL, vfo_fire_event))
    digitalWrite(PC13,1);


}


//
// Callback from encoder class
//

void encoder_callback(uint8_t event_type)
{
  event.fire(EVENT_ENCODER, event_type, 0UL);
}


//
// Action to perform when we get an encoder event
//

void encoder_subscriber(event_data ed, uint8_t event_subtype)
{

  if(event_subtype == ENCODER_EVENT_CW)
    event.fire(EVENT_VFO, EV_SUBTYPE_TUNE_CW, 0UL);
  else if(event_subtype == ENCODER_EVENT_CCW)
    event.fire(EVENT_VFO, EV_SUBTYPE_TUNE_CCW, 0UL);
}

//
// Action when a switch is pressed
//

void switch_subscriber(event_data ed, uint8_t event_subtype)
{
  uint32_t new_incr;
  uint32_t curr_incr;
  switch(event_subtype) {
    case EV_SUBTYPE_ENCODER_PRESSED:
      // Select VFO increment locally instead of putting this code in the VFO class.
      curr_incr = vfo.incr_get();
      if(curr_incr == 100UL)
        new_incr = 1000UL;
      else
        new_incr = 100UL;
      event.fire(EVENT_VFO, EV_SUBTYPE_SET_INCR, new_incr);

    default:
      break;
  }

}




//
// Act on keypad events
//
void keypad_subscriber(event_data ed, uint8_t event_subtype)
{
  char c = ed.char_val;
  event_data k_ed;
  uint8_t i;
  static uint8_t keypad_digits_index;
  static uint8_t state = 0;
  static uint8_t f_count;
  static uint32_t m,f,vf;
  static char keypad_digits[12];

  switch(state){
    case 0: // Start of parsing, first character
      f = m = 0;
      f_count = 0;
      if(isdigit(c)){
        m = (uint32_t) (c - '0');
        state = 1;
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
      if((f_count == 6) || (c == '#') || (c == '\r')) // Check for complete frequency entry
        if((c == '#')||(c == '\r')){ // Short entry by user
          // Multiply by 10 accordingly
          for(i = 0 ; i < 6 - f_count; i++){
            f *= 10;
          }
        // Calculate the frequency in Hz
        vf = (m * 1000000) + f;
        // Set the VFO frequency
        event.fire(EVENT_VFO, EV_SUBTYPE_SET_FREQ, vf);
        state = 0;
        }
      if(isdigit(c)){
        f_count++;
      } else if((c != '#')||(c == '\r')) {
        state = 0;
      }
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
  event.fire(EVENT_DISPLAY, EV_SUBTYPE_KEYPAD_ENTRY, k_ed);
}

//
// Fire an event from the VFO
// This is used to update fields on the display after the VFO object validates them
//

void vfo_fire_event(uint32_t event_type, uint8_t event_subtype, event_data ed){
    event.fire(event_type, event_subtype, ed);

}

//
// Act on VFO parameter change
//

void vfo_subscriber(event_data ed, uint8_t event_subtype)
{
    vfo.subscriber(ed, event_subtype);
}

//
// Act on display event
//

void display_subscriber(event_data ed, uint8_t event_subtype)
{
  display.events(ed, event_subtype);
}


void serial_output_subscriber(event_data ed, uint8_t event_subtype)
{
  switch(event_subtype){
      case EV_SUBTYPE_SET_FREQ:
        Serial1.printf("F:%lu\r\n", ed.u32_val);
        break;
      case EV_SUBTYPE_SET_MODE:
        Serial1.printf("M:%u\r\n", ed.u8_val);
        break;
      case EV_SUBTYPE_TX_MODE:
        Serial1.printf("T:%u\r\n", ed.u8_val);
        break;

      default:
        break;
  }
}

//
// Serial commands task
//

void serial_commands() 

{
  char c;
 
  if(!Serial1.available()){
    return;
  }
  c = Serial1.read();
  event.fire(EVENT_SERIAL, EV_SUBTYPE_NONE, c);
}


//
// Poll PTT and Tune switches
//

void poll_switches()
{
  bool ptt,tune, encoder_switch;
  static bool last_ptt,last_tune, last_encoder_switch;
  static uint8_t ms_counter = 0;
  static uint8_t active_column = 1;
  static uint8_t row_code;
  static uint8_t column_code;
  static uint8_t scan_code;
  static bool last_key_detect;
  bool new_key_detect;
  static const char scan_table[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};
  

  if(ms_counter < 10)
    ms_counter++;
  else 
    ms_counter = 0;


  if(ms_counter == 0){ // Only check switches every 10 calls to debounce them.
    ms_counter = 0;
    ptt = !digitalRead(PIN_PTT);
    tune = !digitalRead(PIN_TUNE);
    encoder_switch = !digitalRead(PIN_ENCODER_SWITCH);


    if(ptt != last_ptt){
      event.fire(EVENT_VFO, (ptt) ? EV_SUBTYPE_PTT_PRESSED : EV_SUBTYPE_PTT_RELEASED, 0UL);
      last_ptt = ptt;
    }
    if(tune != last_tune){
      event.fire(EVENT_VFO, (tune) ? EV_SUBTYPE_TUNE_PRESSED : EV_SUBTYPE_TUNE_RELEASED, 0UL);
      last_tune = tune;
    }
    if(encoder_switch != last_encoder_switch){
      event.fire(EVENT_SWITCH, ((encoder_switch) ? EV_SUBTYPE_ENCODER_PRESSED : EV_SUBTYPE_ENCODER_RELEASED), 0UL);
      last_encoder_switch = encoder_switch;
    }



  } else if(ms_counter == 2){ // Check the keypad
    // Check row inputs
    if( digitalRead(PIN_KEYPAD_R1)){
      new_key_detect = true;
      row_code = 0;
    } else if( digitalRead(PIN_KEYPAD_R2)){
      new_key_detect = true;
      row_code = 1;
    } else if( digitalRead(PIN_KEYPAD_R3)){
      new_key_detect = true;
      row_code = 2;
    } else if( digitalRead(PIN_KEYPAD_R4)){
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
      event.fire(EVENT_KEYPAD, EV_SUBTYPE_NONE, scan_table[scan_code]);
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

}
 

//
// Encoder polling task
//

void task_poll_encoder()
{
 encoder.poll();
}

//
// I/0 polling task
//

void task_poll_io(){
  poll_switches();
  serial_commands();
}

//
// Refresh display
//

void task_refresh_display()
{
  display.refresh();
}

 
 void loop()
 {
   ts.execute();
 }