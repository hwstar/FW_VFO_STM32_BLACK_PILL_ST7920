#include <Arduino.h>
#include <Wire.h>
#include <config.hpp>
#include <pca9546.hpp>

bool PCA9546::present()
{
    Wire.beginTransmission(i2c_address);
    result = Wire.endTransmission();
    return (result != 0) ? false : true;
}


bool PCA9546::bus_select(uint8_t select_bits)
{
    Wire.beginTransmission(i2c_address);
    Wire.write(select_bits & 0x0f);
    return ((result = Wire.endTransmission()) == 0);
}


