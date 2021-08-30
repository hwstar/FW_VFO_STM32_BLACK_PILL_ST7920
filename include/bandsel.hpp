#ifndef __BANDSEL_HPP__

//
// Band select class defintion
//

#include <pca9554.hpp>

// Band Definitions

enum BANDS {BAND_10M = 0x1, BAND_12M = 0x2, BAND_15M = 0x04, BAND_17M = 0x08, BAND_20M = 0x10, BAND_40M = 0x20, BAND_80M = 0x40, BAND_160M = 0x80};

// Band Select Class

class BANDSEL
{
  public:
    // Initialization function.
    bool begin(class PCA9554 *p_bpf, class PCA9554 *p_lpf, BANDS p_init_band = BAND_20M);
    
    // Set the desired band for BPF and LPF.
    void set(BANDS new_band);
  
  private:
    class PCA9554 *bpf;
    class PCA9554 *lpf;

};

#define __BANDSEL_HPP__

#endif



