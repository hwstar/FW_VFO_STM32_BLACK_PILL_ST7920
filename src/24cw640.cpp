
#include <Arduino.h>
#include <Wire.h>
#include <24cw640.hpp>


bool EEPROM_24CW640::present()
{
    uint8_t addr[2] = {0x80, 0x00};

    Wire.beginTransmission(slave_addr); // Send configuration register address
    Wire.write(addr, 2);
    if((result = Wire.endTransmission(false)) != 0) // Restart for read
        return false;
    return true;
}

int EEPROM_24CW640::get_result()
{
    return result;
}


bool EEPROM_24CW640::read_page(uint16_t page_num, void *buffer)
{ 
    char *b = (char *) buffer;  
    uint8_t addr[2];
    uint8_t index;
    uint16_t page_base_addr = (page_num * EEPROM_24CW640_PAGE_SIZE) & (EEPROM_24CW640_BYTE_COUNT - 1);

    // Calculate the address bytes
    addr[1] = (uint8_t) (page_base_addr & 0xFF);
    addr[0] = (uint8_t) (page_base_addr >> 8);

    // Send the page address
    Wire.beginTransmission(slave_addr); // Send page base address
    Wire.write(addr, 2);
    if((result = Wire.endTransmission(false)) != 0) // Restart for read
        return false;

    // Request the page
    Wire.requestFrom(slave_addr, EEPROM_24CW640_PAGE_SIZE);
   
    // Capture the read bytes as they become available
    for(index = 0; index < EEPROM_24CW640_PAGE_SIZE; index++){
        while(!Wire.available());
        b[index] = Wire.read();
    }
    return true;
}

bool EEPROM_24CW640::write_page(uint16_t page_num, void *buffer)
{
    char *b = (char *) buffer;  
    uint8_t addr[2];
    uint8_t index;
    uint16_t page_base_addr = (page_num * EEPROM_24CW640_PAGE_SIZE) & (EEPROM_24CW640_BYTE_COUNT - 1);
    uint8_t wpr;
    bool ret = true;
    uint32_t millis_timeout;

    // Calculate the address bytes
    addr[1] = (uint8_t) (page_base_addr & 0xFF);
    addr[0] = (uint8_t) (page_base_addr >> 8);


    // Perform write transaction
    Wire.beginTransmission(slave_addr); // Send page base address
    Wire.write(addr, 2);
    Wire.write(b, EEPROM_24CW640_PAGE_SIZE);
    if((result = Wire.endTransmission()) != 0) 
        return false;
    // Wait for write to finish
    delay(10);

    return true;

}
