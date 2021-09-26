#ifndef __PCA9554_HPP__


class PCA9554
{
  public:
    PCA9554(uint8_t i2c_addr)
    {
      slave_addr = i2c_addr;
    }

    // Returns true if PCA9554 is present at the i2c_addr givin at class initialization
    bool present();


    // Set gpio configuration. Default is all inputs
    bool set_gpio_config(uint8_t config = 0xff);


    // Set output port bits.
    bool write(uint8_t bits = 0xff);
   
  private:
    uint8_t slave_addr;
    uint8_t res;

};

#define __PCA9554_HPP__

#endif


