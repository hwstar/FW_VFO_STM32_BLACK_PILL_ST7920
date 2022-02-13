
#include <Arduino.h>
#include <Wire.h>
#include <mcp4725.hpp>



bool DAC_MCP4725::present()
{ 

    if(!write_fast(0x0FF))
        return false;

    return true;
}


int DAC_MCP4725::get_result()
{
    return result;
}

bool DAC_MCP4725::write_fast(uint16_t value)
{
    char buffer[2];

    // Perform fast write transaction
    Wire.beginTransmission(slave_addr);
    Wire.write((uint8_t) ((value >> 8) & 0x0F));   // MSB: (D11, D10, D9, D8) 
    Wire.write((uint8_t) (value));  // LSB: (D7, D6, D5, D4, D3, D2, D1, D0)
    if((result = Wire.endTransmission()) != 0) 
        return false;

    return true;
}


