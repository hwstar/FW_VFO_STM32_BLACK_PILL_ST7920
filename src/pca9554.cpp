
#include <Arduino.h>
#include <Wire.h>
#include <pca9554.hpp>

 // Returns true if PCA9554 is present at the i2c_addr givin at class initialization
    bool PCA9554::present()
    {
      Wire.beginTransmission(slave_addr);
      res = Wire.endTransmission();
      return (res != 0) ? false : true;
    }

  
    bool PCA9554::set_gpio_config(uint8_t config)
    {
      Wire.beginTransmission(slave_addr);
      Wire.write(0x03);
      Wire.write(config);
      return ((res = Wire.endTransmission()) == 0);
    }

  
    bool PCA9554::write(uint8_t bits)
    {
      Wire.beginTransmission(slave_addr);
      Wire.write(0x01);
      Wire.write(bits);
      return ((res = Wire.endTransmission()) == 0);
    }