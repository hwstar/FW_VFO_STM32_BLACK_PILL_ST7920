#include <Arduino.h>
#include <TaskScheduler.h>

#include <config.hpp>
#include <vfo.hpp>
#include <logger.hpp>
#include <encoder.hpp>




const String version = "0.0";




uint8_t output_num = 0;
uint8_t drive_strength = 3;

// Static objects

// Scheduler
Scheduler ts;

void serial_commands();
void switch_polling();

// 1mS Polling tasks
void poll_io();
void poll_encoder();
Task itask(1, -1, &poll_io, &ts, true);
Task etask(1, -1, &poll_encoder, &ts, true);



// VFO code
VFO vfo;
ENCODER encoder;

void setup() {


  // Wait for power rails to stabilize before doing anything with the Si5351
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
  
  void encoder_interrupt_handler();
  void encoder_callback(uint8_t event_type);
  encoder.begin(PIN_ENCODER_I,PIN_ENCODER_Q,PIN_ENCODER_SWITCH,[] () { encoder.interrupt_handler(); }, encoder_callback);

  // Initialize vfo object

  if(!vfo.begin(14250000UL))
    digitalWrite(PC13,1);

}


//
// Callback from encoder class
//


void encoder_callback(uint8_t event_type)
{
  if(event_type == ENCODER_EVENT_CW)
    vfo.set_freq(vfo.get_freq()+100);
  else if(event_type == ENCODER_EVENT_CCW)
    vfo.set_freq(vfo.get_freq()-100);

  Serial1.println(vfo.get_freq(), DEC);
  Serial1.flush();
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

  if(ms_counter < 10)
    ms_counter++;
  else{ // Only check switches every 10 calls to debounce them.
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
  }
}

//
// Encoder polling
//

void poll_encoder()
{
 encoder.poll();
}

//
// I/0 polling task
//

void poll_io(){
  poll_switches();
  serial_commands();
}

 

 void loop()
 {
   ts.execute();
 }