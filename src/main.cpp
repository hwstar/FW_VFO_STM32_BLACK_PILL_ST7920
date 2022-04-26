#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TaskScheduler.h>


#include <config.hpp>
#include <event.hpp>
#include <switches.hpp>
#include <keypad.hpp>
#include <comm.hpp>
#include <encoder.hpp>
#include <knob.hpp>
#include <error.hpp>
#include <display.hpp>
#include <vfo.hpp>
#include <cmdparse.hpp>
#include <txprotect.hpp>
#include <menu.hpp>

//
// Menu system
//


// LSB menu item

const menu_item sideband_lsb = {
  "LSB",
  MENU_ATYPE_COMMAND,
  EVENT_VFO,
  EV_SUBTYPE_SET_SIDEBAND,
  {.u8_val=MODE_LSB},
  NULL,
  NULL
};

// USB menu item

const menu_item sideband_usb = {
  "USB",
  MENU_ATYPE_COMMAND,
  EVENT_VFO,
  EV_SUBTYPE_SET_SIDEBAND,
  {.u8_val=MODE_USB},
  NULL,
  NULL
};

// Sideband menu

const menu_level sideband_menu = {
    2, // Number of selections
   "LSB/USB", // Menu name
   {&sideband_lsb, &sideband_usb, NULL, NULL} // Menu entries
   
 };

// AGC on menu item

const menu_item agc_on = {
  "ON",
  MENU_ATYPE_COMMAND,
  EVENT_VFO,
  EV_SUBTYPE_SET_AGC,
  {.u8_val=1}, // Enable
  NULL,
  NULL
};

// AGC off menu item

const menu_item agc_off = {
  "OFF",
  MENU_ATYPE_COMMAND,
  EVENT_VFO,
  EV_SUBTYPE_SET_AGC,
  {.u8_val=0}, // Disable
  NULL,
  NULL
};

// AGC on/off menu

const menu_level agc_menu = {
    2, // Number of selections
   "AGC MENU", // Menu name
   {&agc_on, &agc_off, NULL, NULL} // Menu entries
 };

 // TX gain calibrator
void main_tx_gain_action(menu_action_data *act_data);
 const menu_cal_item_u16 adjust_tx_gain = {
   "TX_GAIN",
   500,
   4095,
   main_tx_gain_action
 };

 // TX gain calibration menu item

 const menu_item cal_tx_gain = {
  "TX GAIN",
  MENU_ATYPE_CAL_U16,
  EVENT_VFO,
  EV_SUBTYPE_SET_AGC,
  {0},
  NULL,
  &adjust_tx_gain
};

 // Calibration menu

 const menu_level cal_menu = {
    1, // Number of selections
   "CALIB", // Menu name
   {&cal_tx_gain, NULL, NULL, NULL} // Menu entries
 };


// AGC menu selection

const menu_item top_agc {
  "AGC...", // Entry name
  MENU_ATYPE_LEVEL_PUSH, // Type of entry
  0, // Event type
  0, // Event subtype
  {0}, // Event data
  &agc_menu,// Lower menu level
  NULL
};

// Sideband menu selection

const menu_item top_sideband = {
  "Sideband...", // Entry name
  MENU_ATYPE_LEVEL_PUSH, // Type of entry
  0, 
  0,
  {0},
  &sideband_menu, // Lower menu level
  NULL
};

// Calibration menu selection

const menu_item top_cal = {
  "Calib...", // Entry name
  MENU_ATYPE_LEVEL_PUSH, // Type of entry
  0, 
  0,
  {0},
  &cal_menu, // Lower menu level
  NULL
};
 
 // Top level menu

 const menu_level top_menu = {
   3, // Number of selections
   "Main Menu", // Menu name
   {&top_sideband,&top_agc,&top_cal,NULL} // Menu entries
};

 




//
// Scheduler
//


// Scheduler object
Scheduler ts;

// Polling tasks
void task_poll_one_ms();
void task_poll_encoder();
void task_poll_hundred_ms();
Task ms_task(1, -1, &task_poll_one_ms, &ts, true);
Task hundred_ms_task(100, -1, &task_poll_hundred_ms, &ts, true);

//
// Objects
//

EVENT pubsub; // Event object
// VFO object
VFO vfo;
// Switches object
SWITCHES switches;
// Keypad object
KEYPAD keypad;
// Encoder object
ENCODER encoder;
// Knob object
KNOB knob(KM_TUNING);
// Display object
DISPLAY_DRIVER display;
// Command parser
CMDPARSE cmdparse;
// Error handler
ERROR_HANDLER error;
// Transmitter protection
TX_PROTECT txprotect;
// Menu system
MENU menu;



//
// Arduino setup function
//

void setup() 
{

 
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
  #ifdef S_METER_ADC
  pinMode(S_METER_ADC, INPUT);
  #endif

  // Outputs
  pinMode(PIN_STM32_LED, OUTPUT);
  digitalWrite(PIN_STM32_LED, 0);
  
  pinMode(PIN_PA_FAN_ENABLE, OUTPUT);
  digitalWrite(PIN_PA_FAN_ENABLE, 0);
  pinMode(PIN_DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_DISPLAY_BACKLIGHT, 1);
  pinMode(PIN_TEST_OUTPUT, OUTPUT);
  digitalWrite(PIN_TEST_OUTPUT, 0);


  pinMode(PIN_KEYPAD_C1, OUTPUT);
  pinMode(PIN_KEYPAD_C2, OUTPUT);
  pinMode(PIN_KEYPAD_C3, OUTPUT);
  pinMode(PIN_KEYPAD_C4, OUTPUT);

  #ifdef S_METER_ADC
  // ADC setup
  analogReadResolution(12);
  analogRead(S_METER_ADC);
  #endif


 // Peripheral pin mux setup

  Wire.setSCL(PIN_I2C_SCL); 
  Wire.setSDA(PIN_I2C_SDA); 
  Wire.begin();

  SPI.setSCLK(PIN_SPI_CLK);
  SPI.setMOSI(PIN_SPI_MOSI);
  SPI.setMISO(PIN_SPI_MISO);

  Serial.setRx(PIN_UART_RX);
  Serial.setTx(PIN_UART_TX);



  
  digitalWrite(PIN_STM32_LED, 1);

  

  
  // Initialize serial port
  Serial1.begin(115200);
  Serial1.setTimeout(10000);
  Serial1.println("POR");

  // Error handler initialization
  error.begin();
  
  // Initialize Switches
  switches.begin(ETS_SWITCHES);
  // Initialize Keypad
  keypad.begin(ETS_KEYPAD);
  // Initialize txprotect
  txprotect.begin();
  // Initialize Encoder
  void encoder_interrupt_handler();
  encoder.begin(PIN_ENCODER_I, PIN_ENCODER_Q, PIN_ENCODER_SWITCH, [] () { encoder.interrupt_handler(); });
  // Initialize display 
  display.begin();
  // Initialize menu
  void command_handler(uint32_t command);
  menu.begin(&top_menu);

  // Add subscribers to the event object
  void encoder_subscriber(event_data, uint32_t);
  void encoder_knob_subscriber(event_data, uint32_t);
  void display_subscriber(event_data, uint32_t);
  void serial_output_subscriber(event_data, uint32_t );
  void vfo_subscriber(event_data, uint32_t);
  void keypad_switches_subscriber(event_data ed, uint32_t event_subtype);
  void cmdparse_subscriber(event_data ed, uint32_t event_subtype);
  void error_subscriber(event_data ed, uint32_t event_subtype);
  void txprotect_subscriber(event_data ed, uint32_t event_subtype);

  pubsub.subscribe(encoder_subscriber, EVENT_TICK);
  pubsub.subscribe(encoder_knob_subscriber, EVENT_TICK|EVENT_ENCODER|EVENT_ENCODER_KNOB); 
  pubsub.subscribe(display_subscriber, EVENT_DISPLAY|EVENT_TICK);
  pubsub.subscribe(serial_output_subscriber, EVENT_DISPLAY);
  pubsub.subscribe(vfo_subscriber, EVENT_VFO|EVENT_TICK);
  pubsub.subscribe(keypad_switches_subscriber, EVENT_TICK);
  pubsub.subscribe(cmdparse_subscriber, EVENT_KEYPAD_PARSER|EVENT_SERIAL|EVENT_TICK);
  pubsub.subscribe(error_subscriber, EVENT_ERROR);
  pubsub.subscribe(txprotect_subscriber, EVENT_VFO|EVENT_TICK);
  
  // Initialize vfo object
  // Events must be initialzed first for default freqency and mode to be displayed.
  if(!vfo.begin(14250000UL))
    digitalWrite(PC13,1);


}

//
// Action when a encoder knob is pressed or turned
//

void encoder_knob_subscriber(event_data ed, uint32_t event_subtype)
{
  knob.subscriber(ed, event_subtype);

}

//
// Act on vfo event
//

void vfo_subscriber(event_data ed, uint32_t event_subtype)
{
  vfo.subscriber(ed, event_subtype);
}

//
// Act on display event
//

void display_subscriber(event_data ed, uint32_t event_subtype)
{
  display.events(ed, event_subtype);
}

//
// Act on encoder event
//

void encoder_subscriber(event_data ed, uint32_t event_subtype)
{
  encoder.handler(ed, event_subtype);
}

//
// Act on tick event for switches and keypad
//

void keypad_switches_subscriber(event_data ed, uint32_t event_subtype)
{
  switches.handler(ed, event_subtype);
  keypad.handler(ed, event_subtype);
}

//
// Send keypad and serial events to the command parser
//

void cmdparse_subscriber(event_data ed, uint32_t event_subtype)
{
  cmdparse.handler(ed, event_subtype);
}

//
// Send error events to the error handler
//

void error_subscriber(event_data ed, uint32_t event_subtype)
{
  error.handler(ed, event_subtype);
}

//
// Send events to txprotect
//

void txprotect_subscriber(event_data ed, uint32_t event_subtype)
{
  txprotect.handler(ed, event_subtype);
}


//
// Send serial output on certain events
//

void serial_output_subscriber(event_data ed, uint32_t event_subtype)
{
  switch(event_subtype){
      case EV_SUBTYPE_SET_FREQ:
        Serial1.printf("F:%lu\r\n", ed.u32_val);
        break;
      case EV_SUBTYPE_SET_SIDEBAND:
        Serial1.printf("M:%u\r\n", ed.u8_val);
        break;
      case EV_SUBTYPE_TRX_MODE:
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
  pubsub.fire(EVENT_SERIAL, EV_SUBTYPE_NONE, c);
}


//
// 1 millisecond polling task
//

void task_poll_one_ms(){
  static uint8_t toggle = 0;
  // Toggle test output every millisecond
  digitalWrite(PIN_TEST_OUTPUT, (toggle ^= 1));
  // Send 1 millisecond event
  pubsub.fire(EVENT_TICK, EV_SUBTYPE_TICK_MS, Time_slot);
  // Advance to next time slot
  Time_slot.u32_val++;
  if(Time_slot.u32_val > 9)
    Time_slot.u32_val = 0;
  serial_commands(); // TODO Refactor move to comm.cpp and Remove
}

//
// 100 millisecond polling task
//

void task_poll_hundred_ms()
{
  pubsub.fire(EVENT_TICK, EV_SUBTYPE_TICK_HUNDRED_MS);
}

 
//
// Action function for TX gain
//

void main_tx_gain_action(menu_action_data *act_data){

  
  if(act_data->command == MENU_CAL_RETRIEVE){
    act_data->value = vfo.get_txgain();
    // If str_param is not null, copy the band name to the parameter provided using value as the length
    act_data->const_str = vfo.get_band();

  }
  else if(act_data->command == MENU_CAL_STORE){
    pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_TXGAIN, act_data->value);
  }

}


 void loop()
 {
   ts.execute();
 }