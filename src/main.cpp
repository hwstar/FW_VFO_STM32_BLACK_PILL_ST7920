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
#include <error.hpp>
#include <display.hpp>
#include <vfo.hpp>
#include <cmdparse.hpp>
#include <txprotect.hpp>




// Scheduler object
Scheduler ts;

// Polling tasks
void task_poll_one_ms();
void task_poll_encoder();
void task_poll_hundred_ms();
Task ms_task(1, -1, &task_poll_one_ms, &ts, true);
Task hundred_ms_task(100, -1, &task_poll_hundred_ms, &ts, true);


EVENT pubsub; // Event object
// VFO object
VFO vfo;
// Switches object
SWITCHES switches;
// Keypad object
KEYPAD keypad;
// Encoder object
ENCODER encoder;
// Display object
DISPLAY_DRIVER display;
// Command parser
CMDPARSE cmdparse;
// Error handler
ERROR_HANDLER error;
// Transmitter protection
TX_PROTECT txprotect;


// 10ms Time slot
event_data Time_slot;

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

 // Peripheral pin mux setup

  Wire.setSCL(PIN_I2C_SCL); 
  Wire.setSDA(PIN_I2C_SDA); 

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
  switches.begin(0);
  // Initialize Keypad
  keypad.begin(2);
  // Initialize txprotect
  txprotect.begin();
  // Initialize Encoder
  void encoder_interrupt_handler();
  encoder.begin(PIN_ENCODER_I, PIN_ENCODER_Q, PIN_ENCODER_SWITCH, [] () { encoder.interrupt_handler(); });
  // Initialize display 
  display.begin();

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

  pubsub.subscribe(encoder_subscriber, EVENT_ENCODER|EVENT_TICK);
  pubsub.subscribe(encoder_knob_subscriber, EVENT_ENCODER_KNOB); // Knob press = increment change
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
// Action when a encoder knob is pressed
//

void encoder_knob_subscriber(event_data ed, uint32_t event_subtype)
{
  uint32_t new_incr;
  uint32_t curr_incr;
  switch(event_subtype) {
    case EV_SUBTYPE_ENCODER_PRESSED:
      // Select VFO increment locally instead of putting this code in the VFO class.
      curr_incr = vfo.incr_get();
      if(curr_incr == 1000)
        new_incr = 100;
      else if(curr_incr == 100)
        new_incr = 10;
      else if(curr_incr == 10)
        new_incr = 1000;
      else
        new_incr = 1000;
      pubsub.fire(EVENT_VFO, EV_SUBTYPE_SET_INCR, new_incr);

    default:
      break;
  }

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

 
 void loop()
 {
   ts.execute();
 }