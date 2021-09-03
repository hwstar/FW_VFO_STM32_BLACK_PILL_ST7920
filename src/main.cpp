#include <Arduino.h>
#include <TaskScheduler.h>
#include <U8g2lib.h>

#include <config.hpp>
#include <vfo.hpp>
#include <logger.hpp>
#include <encoder.hpp>
#include <event.hpp>




uint8_t output_num = 0;
uint8_t drive_strength = 3;

// Static objects

// Scheduler object
Scheduler ts;

void serial_commands();
void switch_polling();

// 1mS Polling tasks
void task_poll_io();
void task_poll_encoder();
void task_update_display();
Task itask(1, -1, &task_poll_io, &ts, true);
Task etask(1, -1, &task_poll_encoder, &ts, true);
Task dtask(100, -1, &task_update_display, &ts, true);

// Event object
EVENT event;
// VFO object
VFO vfo;
// Encoder object
ENCODER encoder;
// Display object
U8G2_ST7920_128X64_1_HW_SPI st7920(U8G2_R0, PIN_SPI_CS, U8X8_PIN_NONE);

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
  st7920.begin();

  // Initialize vfo object

  if(!vfo.begin(14250000UL))
    digitalWrite(PC13,1);

  // Add subscribers to the event object
  void encoder_subscriber(event_data, uint8_t);
  void keypad_subscriber(event_data, uint8_t);
  void vfo_subscriber(event_data, uint8_t);
  event.subscribe(encoder_subscriber, EVENT_ENCODER);
  event.subscribe(keypad_subscriber, EVENT_KEYPAD|EVENT_SERIAL);
  event.subscribe(vfo_subscriber, EVENT_VFO);

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
    vfo.set_freq(vfo.get_freq()+100);
  else if(event_subtype == ENCODER_EVENT_CCW)
    vfo.set_freq(vfo.get_freq()-100);

  Serial1.println(vfo.get_freq(), DEC);
  Serial1.flush();

}

//
// Act on keypad events
//
void keypad_subscriber(event_data ed, uint8_t event_subtype)
{
  char c = ed.char_val;
  uint8_t i;
  static uint8_t state = 0;
  static uint8_t f_count;
  static uint32_t m,f,vf;



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
      } else if (c == '*'){ // "Decimal point"
        state = 3;
        
      } else { // Abort
        state = 0;
      }
      break;

    case 2: // Expect decimal point
      if(c == '*'){
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
      if((f_count == 6) || (c == '#')) // Check for complete frequency entry
        if(c == '#'){ // Short entry by user
          // Multiply by 10 accordingly
          for(i = 0 ; i < 6 - f_count; i++){
            f *= 10;
          }
        // Calculate the frequency in Hz
        vf = (m * 1000000) + f;
        // Set the VFO freqency
        event.fire(EVENT_VFO, EV_SUBTYPE_SET_FREQ, vf);
        state = 0;
        }
      if(isdigit(c)){
        f_count++;
      } else if( c != '#') {
        state = 0;
      }
  }

}

//
// Act on VFO parameter change
//

void vfo_subscriber(event_data ed, uint8_t event_subtype)
{
  switch(event_subtype){
    case EV_SUBTYPE_SET_FREQ:
      // Set frequency
      vfo.set_freq(ed.u32_val);
      // Set default mode for band
      vfo.mode_set(MODE_DEFAULT);
      break;
    default:
      break;
  }
}


//
// Parse an unsigned integer
//

bool parse_uint32(String str, uint32_t min, uint32_t max, uint32_t &res) {
  res = 0;
  unsigned i;
  for(i = 0; i < str.length(); i++){
    if(!isdigit(str[i]))
      return false;
    res *= 10;  
    res += (str[i] - 0x30);
  }
  if((res < min) || (res > max))
    return false;
  else
    return true;
}

//
// Serial commands task
//

void serial_commands() 

{
  static String buffer;
  String command;
  String arg;
  bool param;
  uint32_t res;
  static uint32_t last_increment = 0;
  

  if(!Serial1.available()){
    return;
  }
  // Wait for a command
  char c = Serial1.read();
  if((c != '\r') && (buffer.length() < 16)){
    buffer+=c;
    return;
  }

  param = false;
  command = buffer;
  buffer.remove(0);
  arg.remove(0);

  // Get argument
  
  if(command.length() > 1){
    param = true;
    arg = command.substring(1);
  }
    
  switch(command[0]){
  
    case 'd': // Down a small amount
      if(param && parse_uint32(arg, 1, 1000, res))
        last_increment = res;
      vfo.set_freq(vfo.get_freq()-last_increment);
      Serial1.println(vfo.get_freq(), DEC);
      Serial1.flush();
      return;

    case 'f': // Frequency in Hz
      if(param && parse_uint32(arg, 1800000, 30000000, res)){
        vfo.set_freq(res);
      }
      Serial1.println(vfo.get_freq(), DEC);
      Serial1.flush();
      return;
  
      break;
    
    case 'm':
      if(param && parse_uint32(arg, 0, 1, res)){
         vfo.mode_set((uint8_t) res);
         }
      Serial1.println(vfo.mode_get(), DEC);
      Serial1.flush();
      return; 

    case 'r':
      Serial1.println(vfo.get_freq(), DEC);
      Serial1.flush();
      return;

    case 't': // Transmit enable: 2 = TX tune, 1 = TX 0 = Rx
       if(param && parse_uint32(arg, 0, 2, res)){
         vfo.ptt_set((uint8_t) res);
         }
      Serial1.println(vfo.ptt_get(), DEC);
      Serial1.flush();
      return;
   
    case 'u': // Up a small amount
      if(param && parse_uint32(arg, 1, 1000, res))
        last_increment = res;
      vfo.set_freq(vfo.get_freq()+last_increment);
      Serial1.println(vfo.get_freq(), DEC);
      Serial1.flush();
      return;

    default:
      return;

    }
}

//
// Poll PTT and Tune switches
//

void poll_switches()
{
  bool ptt,tune;
  static bool last_ptt,last_tune;
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

    if(ptt != last_ptt){
      if(ptt){
          vfo.ptt_set(1);
      } else {
          vfo.ptt_set(0);
      }
      last_ptt = ptt;
    }
    if(tune != last_tune){
      if(tune){
          vfo.ptt_set(2);
      } else {
          vfo.ptt_set(0);
      }
      last_tune = tune;
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
// Update information on the display
//

void task_update_display()
{
  const char *mode;
  uint32_t freq = vfo.get_freq();

  uint32_t mhz = freq/1000000UL;
  uint32_t modulus = freq % 1000000UL;

  String fmhz(mhz);
  String fmod(modulus);
  String fmhzdot(fmhz + ".");
  String freqall(fmhzdot+fmod);


  if(vfo.mode_get() == MODE_USB)
    mode = "USB";
  else
    mode = "LSB";

 
 st7920.firstPage();
 do {
    /* all graphics commands have to appear within the loop body. */    
    st7920.setFont(u8g2_font_ncenB14_tr);
    st7920.drawStr(0, 20, freqall.c_str());
    st7920.drawStr(0, 40, mode);

  } while ( st7920.nextPage() );
}

 
 void loop()
 {
   ts.execute();
 }