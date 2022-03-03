#ifndef __24CW640_HPP__

#define EEPROM_24CW640_PAGE_SIZE 32
#define EEPROM_24CW640_BYTE_COUNT 8192


class EEPROM_24CW640 
{
    public:
    
    EEPROM_24CW640(int i2c_addr) 
    {
        
        slave_addr = i2c_addr;
    
    }

    // Test presence
    bool present();

    // Return result code from previous transaction
    int get_result();
   
    // Read a page of data (32 bytes)
    bool read_page(uint16_t page_num, void *buffer);

    // Write a page of data (32 bytes)
    bool write_page(uint16_t page_num, void *buffer);

    private:
   
    int slave_addr;
    int result;

    const uint8_t EEPROM_WRTE = 0x40;
    const uint32_t EEPROM_TIMEOUT = 10;

};


#define __24CW640_HPP__
#endif
