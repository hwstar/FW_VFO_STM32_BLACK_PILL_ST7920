#ifndef __SI5351_EK_WRAPPER__


#include <si5351.h>

//
// Extend Etherkit Si5351 class
//

class SI5351_EK_WRAPPER : public Si5351
{
  public:
    SI5351_EK_WRAPPER(uint8_t i2c_addr = SI5351_BUS_BASE_ADDR) : Si5351 {i2c_addr}
    {
      i2c_bus_addr = i2c_addr;
    }

    /*
    * Initialize the SI5351 without calling Wire.begin
    */

    bool init_sans_wire_begin(uint8_t xtal_load_c, uint32_t xo_freq, int32_t corr);
    
    /*
    * Set frequency in Hz
    */

  
    uint8_t set_freq_hz(uint32_t freq_hz, enum si5351_clock output);

  private:
    uint8_t i2c_bus_addr; // Need a local copy. Defined as private in the base class


};

#define __SI5351_EK_WRAPPER__

#endif
