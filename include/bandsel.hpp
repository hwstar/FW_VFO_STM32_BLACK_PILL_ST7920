#ifndef __BANDSEL_HPP__

//
// Band select class defintion
//

#include <pca9554.hpp>


// Band Definitions


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



