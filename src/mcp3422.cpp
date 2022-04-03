#include <Arduino.h>
#include <Wire.h>
#include <config.hpp>
#include <mcp3422.hpp>

typedef union bufint32_u {
    uint8_t buffer[4];
    int32_t value;
} bufint32_u;

// Private function ti write the MCP3422 configuration register

bool MCP3422::write_config_reg()
{
    Wire.beginTransmission(device_address);
    Wire.write(config_reg);
    int res = Wire.endTransmission();
    if(res != 0){
        return false;
    }
    return true;
}

// 
// Configure the MCP3422
//

bool MCP3422::begin(uint8_t a_device_address, bool a_one_shot, uint8_t a_pga, uint8_t a_sample_rate)
{
    dev_present = false;

    device_address = a_device_address;
    // Set up the configuration register bits

    config_reg = (a_one_shot) ? 0 : 0x10;
    config_reg |= ((a_sample_rate & 0x03) << 2);
    config_reg |= (a_pga & 0x03);

    if(!write_config_reg())
        return false;

    dev_present = true;
  
    return true;
}

//
// Select an ADC channel
//


void MCP3422::select_channel(uint8_t channel)
{
    if(!dev_present)
        return;
    config_reg |= 0x80;
    config_reg &= ~0x60;
    config_reg |= ((channel & 0x03) << 5);
    write_config_reg();
    return;
}

//
// Test the ready flag
// Fills buffer with the ADC return data
//

bool MCP3422::ready()
{
    uint8_t trys;
    uint8_t index;
  
    if(!dev_present)
        return false;
    
    // Calculate bytes read in advance
    bytes_read = (config_reg & 0x60 == 0x60) ? 4 : 3;

    // Capture the read bytes as they become available
    trys = 3;
    do {
        if(!trys)
            return false;
        Wire.requestFrom(device_address, bytes_read);
        for(index = 0; index < bytes_read; index++){
            while(!Wire.available());
            buffer[index] = Wire.read();
        }
        trys--;
    } while (buffer[bytes_read - 1] & 0x80); // Last byte is the configuration register. Wait for ready bit to go low
    
    return ((buffer[bytes_read - 1] & 0x80) == 0);
}




//
// Read the result of the conversion
//


bool MCP3422::read(int32_t &result)
{
    bufint32_u bufint32;

    result = 0;

    if(!dev_present)
        return false;
    

    if(!ready())
        return false;
    
    #if (BYTE_ORDER == LITTLE_ENDIAN)
    if(bytes_read == 4){ // 18 bit
        bufint32.buffer[3] = (buffer[0] & 0x80) ? 0xFF : 0;
        bufint32.buffer[2] = buffer[0];
        bufint32.buffer[1] = buffer[1];
        bufint32.buffer[0] = buffer[2];
    }
    else { // 12, 14 or 16 bit
        bufint32.buffer[3] = (buffer[0] & 0x80) ? 0xFF : 0;
        bufint32.buffer[2] = bufint32.buffer[3];
        bufint32.buffer[1] = buffer[0];
        bufint32.buffer[0] = buffer[1];
    }
    #else
        return false; // Big endian not supported yet
    #endif
    
    result = bufint32.value;

    return true;
}

