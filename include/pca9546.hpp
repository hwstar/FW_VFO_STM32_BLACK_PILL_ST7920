
#ifndef __PCA9546_HPP__

class PCA9546
{
    public:
    PCA9546(uint8_t i2c_addr){
        i2c_address = i2c_addr;
    }

    bool present();
    
    bool bus_select(uint8_t select_bits);

    private:
    int result;
    uint8_t i2c_address;


};

#define __PCA9546_HPP__

#endif
