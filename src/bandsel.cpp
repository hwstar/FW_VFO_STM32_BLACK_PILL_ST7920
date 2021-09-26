  //
  // BPF and LPF band select class implementation
  //
  #include <Arduino.h>
  
  #include <config.hpp>
  #include <bandsel.hpp>
  #include <logger.hpp>

  
  
  bool BANDSEL::begin(class PCA9554 *p_bpf, class PCA9554 *p_lpf, BANDS p_init_band)
    {

        bpf = p_bpf;
        lpf = p_lpf;
    
        
        if(!lpf->present()){
            logger.error(ERR_NO_LPF);
            return false;
        }
         
        if(!bpf->present()){
            logger.error(ERR_NO_BPF);
            return false;
        }


        lpf->write(0x00);
        lpf->set_gpio_config(0x00);
        lpf->write(p_init_band);

        bpf->write(0x00);
        bpf->set_gpio_config(0x00);
        bpf->write(p_init_band);

        return true;
 
    }

    void BANDSEL::set(BANDS new_band)
    {
        bpf->write(new_band);
        lpf->write(new_band);
    }