#include <Arduino.h>
#include <Wire.h>

#include "si5351.h"

#include <pca9554.hpp>

#define LPF_I2C_ADDR 0x38
#define BPF_I2C_ADDR 0x39




class Ext_Si5351 : public Si5351
{
  public:
    Ext_Si5351(uint8_t i2c_addr = SI5351_BUS_BASE_ADDR) : Si5351 {i2c_addr}
    {
    }
    
    /*
    * Set frequency in Hz
    */
  
    uint8_t set_freq_hz(uint32_t freq_hz, enum si5351_clock output)
    {
      uint64_t freq = 100ULL * freq_hz;
      return set_freq(freq, output);
    }
};

const String version = "0.0";

const si5351_clock clock_outputs[3] = { SI5351_CLK0, SI5351_CLK1, SI5351_CLK2 };
const si5351_drive drive_strengths[4] = { SI5351_DRIVE_2MA, SI5351_DRIVE_4MA, SI5351_DRIVE_6MA, SI5351_DRIVE_8MA };

bool i2c_found;


uint8_t output_num = 0;
uint8_t drive_strength = 3;

PCA9554 lpf(LPF_I2C_ADDR);
Ext_Si5351 si5351;

void reset_to_defaults() {
  int i;
  for(i = 0; i < 3; i++) {
    // Default freq 10 MHz
    si5351.set_freq_hz(10000000, clock_outputs[i]);
    // 8mA Drive strength
    si5351.drive_strength(clock_outputs[i], drive_strengths[3]);
    // Note: setting up the previous commands turns the outputs on for some reason. Output disable must be the last thing we do.
    // All outputs off
    si5351.output_enable(clock_outputs[i], 0);
  }
}

void setup() {


  // Wait for power rails to stabilize before doing anything with the Si5351
  delay(100);

   // LED off
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, 1);


  // Initialize serial port
  Serial1.begin(115200);
  Serial1.setTimeout(10000);



i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, -4440);

  if(i2c_found){
     // LED on to signify Si5351 found
    digitalWrite(PC13, 0);
    // Reset to defaults
    reset_to_defaults();
  
   
  }


  // Initialize I2C slaves

 // Clear the band bits
 // Wire.beginTransmission(0x38);
 // Wire.write(0x01);
 // Wire.write(0x00);
 // res = Wire.endTransmission(1);

  lpf.write(0);
  

  // Set all gpio pins as outputs
  //Wire.beginTransmission(0x38);
  //Wire.write(0x03);
  //Wire.write(0x00);
  //res = Wire.endTransmission(1);
  lpf.set_gpio_config(0);

  // Select the test band
  //Wire.beginTransmission(0x38);
  //Wire.write(0x01);
  //Wire.write(0x20);
  //res = Wire.endTransmission(1);
  lpf.write(0x01);



/*
 
 */
}

/*
* Parse an unsigned integer, and check its range
*/

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



void loop() {
  String command;
  String arg;
  uint32_t res;
  uint8_t index;

  while(true){
    if(!Serial1.available()){
      continue;
    }
    // Wait for a command
    command = Serial1.readStringUntil('\n');
    res = res + 1;
    if(command.length() == 0){
      continue;
    }
    // If less than 2, the command is not valid
    if(command.length() < 2)
      continue;

    // Get argument
    arg = command.substring(1);
      
    switch(command[0]){
      case 'c': // Set Clock output to use for other commands
        if(isdigit(command[1])){
          // Second command character is the output number. Range is 0 to 2
          if(output_num <= 2){
            output_num = command[1] - 0x30;
            break;
          }
          else
            continue;
        }
        else
          continue;
      case 'd': // Drive strength

        if(!parse_uint32(arg, 2, 8, res)){
          index = 0;
          continue;
        }
        else {
          switch(res){

            case 2:
              index = 0;
              break;

            case 4:
              index = 1;
              break;

            case 6:
              index = 2;
              break;

            case 8:
              index = 3;
              break;

            default:
              continue;
          }
          si5351.drive_strength(clock_outputs[output_num], drive_strengths[index]);
          break;
        }

      case 'e': // Output control
        if(parse_uint32(arg, 0, 1, res)){
          si5351.output_enable(clock_outputs[output_num], res); 
          break; 
        }
        continue;

      case 'f': // Frequency in MHz
        if(parse_uint32(arg, 10, 150000000, res)){
          si5351.set_freq_hz(res, clock_outputs[output_num]);
          break;
        }
        continue;

       case 'i':
        // Info
        switch(command[1]){
          // Version
          case 'v':
            Serial1.println("iv" + version);
            
        }
        Serial1.flush();
        continue;


      case 'r':
        // Reset all outputs to defaults
        if(command[1] == 'd') {
           reset_to_defaults();
           break;
        }
        continue;

      default:
        // Not a recognized command
        continue;

    }
    Serial1.println(command);
    Serial1.flush();
  }

}