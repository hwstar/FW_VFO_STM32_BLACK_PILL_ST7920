  //
  // BPF and LPF band select class implementation
  //
  #include <Arduino.h>
  
  #include <config.hpp>
  #include <bandsel.hpp>
  #include <event.hpp>


  bool BANDSEL::begin(class PCA9554 *p_bpf, class PCA9554 *p_lpf, BANDS p_init_band)
    {

        bpf = p_bpf;
        lpf = p_lpf;
        bool res = true;
    
        #ifndef TRX_MODULE_TEST_MODE
        
        if(!lpf->present()){
            res = false;
            pubsub.fire(EVENT_ERROR, EV_SUBTYPE_ERR_NO_LPF);
          
        }
         
        if(!bpf->present()){
            res = false;
            pubsub.fire(EVENT_ERROR, EV_SUBTYPE_ERR_NO_BPF);
        }    


        lpf->write(0x00);
        lpf->set_gpio_config(0x00);
        lpf->write(p_init_band);

        bpf->write(0x00);
        bpf->set_gpio_config(0x00);
        bpf->write(p_init_band);
        #endif

        return res;
 
    }

    void BANDSEL::set(BANDS new_band)
    {
        #ifndef TRX_MODULE_TEST_MODE
        bpf->write(new_band);
        lpf->write(new_band);
        #endif
    }