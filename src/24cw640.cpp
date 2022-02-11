
#include <Arduino.h>
#include <Wire.h>
#include <24cw640.hpp>


bool EEPROM_24CW640::present()
{
    uint8_t value;

    return get_wpr(value);
}

int EEPROM_24CW640::get_result()
{
    return result;
}


bool EEPROM_24CW640::get_wpr(uint8_t &value)
{
    uint8_t addr[2] = {0x80, 0x00};

    Wire.beginTransmission(slave_addr); // Send configuration register address
    Wire.write(addr, 2);
    if((result = Wire.endTransmission(false)) != 0) // Restart for read
        return false;
    Wire.requestFrom(slave_addr, 1);
    while(!Wire.available());
    value = Wire.read(); // Read the register value
    return ((result = Wire.endTransmission()) == 0);
}

bool EEPROM_24CW640::set_wpr(uint8_t value)
{
    uint8_t res;
    uint8_t addr_val[3] = {0x80, 0x00, 0x00};
    addr_val[2] = value;
    // Send the write protect register value
    Wire.beginTransmission(slave_addr); // Send configuration register address
    Wire.write(addr_val, 3);
    return ((result = Wire.endTransmission()) == 0);
}

   
bool EEPROM_24CW640::read_page(uint16_t page_num, uint8_t *buffer)
{   
    uint8_t addr[2];
    uint8_t index;
    uint16_t page_base_addr = (page_num * EEPROM_PAGE_SIZE) & (EEPROM_BYTE_COUNT - 1);

    // Calculate the address bytes
    addr[1] = (uint8_t) (page_base_addr & 0xFF);
    addr[0] = (uint8_t) (page_base_addr >> 8);

    // Send the page address
    Wire.beginTransmission(slave_addr); // Send page base address
    Wire.write(addr, 2);
    if((result = Wire.endTransmission(false)) != 0) // Restart for read
        return false;

    // Request the page
    Wire.requestFrom(slave_addr, EEPROM_PAGE_SIZE);
   
    // Capture the read bytes as they become available
    for(index = 0; index < EEPROM_PAGE_SIZE; index++){
        while(!Wire.available());
        buffer[index] = Wire.read();
    }
    return true;
}

bool EEPROM_24CW640::write_page(uint16_t page_num, uint8_t *buffer)
{
    uint8_t addr[2];
    uint8_t index;
    uint16_t page_base_addr = (page_num * EEPROM_PAGE_SIZE) & (EEPROM_BYTE_COUNT - 1);
    uint8_t wpr;
    bool ret;
    uint32_t millis_timeout;

    // Calculate the address bytes
    addr[1] = (uint8_t) (page_base_addr & 0xFF);
    addr[0] = (uint8_t) (page_base_addr >> 8);

    
    // Wait for device to finish any previous write cycle
    // Fetch the current write protection register value
    // Enforce time out period for this transaction
    millis_timeout = millis() + EEPROM_TIMEOUT;
    while(!get_wpr(wpr)){
        if(millis() >= millis_timeout){
            result = -1; // -1 signifies timeout
            return false; 
        }
    }
    
    // Set write enable bit, and send to EEPROM

    wpr |= EEPROM_WRTE;
    ret = set_wpr(wpr);


    // Perform write transaction
    Wire.beginTransmission(slave_addr); // Send page base address
    Wire.write(addr, 2);
    Wire.write(buffer, EEPROM_PAGE_SIZE);
    if((result = Wire.endTransmission()) != 0) // Restart for read
        ret = false;

    
    
    // Disable write enable bit
    wpr &= ~EEPROM_WRTE;
    ret = set_wpr(wpr);

    return ret;

}
