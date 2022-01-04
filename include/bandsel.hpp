#ifndef __BANDSEL_HPP__

//
// Band select class defintion
//

#include <event.hpp>
#include <pca9554.hpp>


// Band Definitions


// Band Select Class

class BANDSEL
{
  public:
    // Initialization function.
    bool begin(class PCA9554 *p_bpf, class PCA9554 *p_lpf, void (*event_callback)(uint32_t, uint32_t, event_data), BANDS p_init_band = BAND_20M );
    
    // Set the desired band for BPF and LPF.
    void set(BANDS new_band);
  
  private:
    void fire_event(uint32_t event_type, uint32_t event_subtype);
    void (*ev_cb)(uint32_t, uint32_t, event_data);
    class PCA9554 *bpf;
    class PCA9554 *lpf;

};

#define __BANDSEL_HPP__

#endif



