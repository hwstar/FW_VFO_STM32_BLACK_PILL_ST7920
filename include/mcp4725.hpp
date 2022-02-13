#ifndef __MCP4725_HPP__


class DAC_MCP4725 
{
    public:
    
    DAC_MCP4725(int i2c_addr) 
    {
        
        slave_addr = i2c_addr;
    
    }

    // Test presence
    bool present();

    // Return result code from previous transaction
    int get_result();

    // Write to the DAC without saving in the EEPROM
    bool write_fast(uint16_t value);

    // Read the DAC register
    bool read_val(uint16_t &value);
   

    private:
    bool read_raw(uint8_t *buffer);
    int slave_addr;
    int result;

   
};


#define __MCP4725_HPP__
#endif
