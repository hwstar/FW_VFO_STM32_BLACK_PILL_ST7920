#include <Arduino.h>
#include <Wire.h>

#include <config.hpp>
#include <pca9554.hpp>
#include <bandsel.hpp>
#include <vfo.hpp>




const String version = "0.0";




uint8_t output_num = 0;
uint8_t drive_strength = 3;



VFO vfo;

void setup() {


  // Wait for power rails to stabilize before doing anything with the Si5351
  delay(100);

   // LED off
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, 1);

  
  // Initialize serial port
  Serial1.begin(115200);
  Serial1.setTimeout(10000);

  

// Initialize band select object

  if(!vfo.begin(14280000UL))
    digitalWrite(PC13,1);





}
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



void loop() 

{
  String command;
  String arg;
  uint32_t res;
  bool param;
  static uint32_t last_increment = 0;

  if(!Serial1.available()){
    return;
  }
  // Wait for a command
  command = Serial1.readStringUntil('\r');
  res = res + 1;
  if(command.length() == 0){
    return;
  }
  
  // Get argument
  param = false;
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

    case 'f': // Frequency in MHz
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
 