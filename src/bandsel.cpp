  //
  // BPF and LPF band select class implementation
  //
  #include <Arduino.h>
  
  #include <config.hpp>
  #include <bandsel.hpp>
  #include <event.hpp>

  void BANDSEL::fire_event(uint32_t event_type, uint32_t event_subtype)
{
    
    if(ev_cb != NULL){
        event_data ed;
        ed.u32_val = 0L;
        (*ev_cb)(event_type, event_subtype, ed);
    }
}


  bool BANDSEL::begin(class PCA9554 *p_bpf, class PCA9554 *p_lpf, void (*event_callback)(uint32_t, uint32_t, event_data), BANDS p_init_band)
    {

        bpf = p_bpf;
        lpf = p_lpf;
        ev_cb = event_callback;
        bool res = true;
    
        
        if(!lpf->present()){
            res = false;
            fire_event(EVENT_ERROR, EV_SUBTYPE_ERR_NO_LPF);
          
        }
         
        if(!bpf->present()){
            res = false;
            fire_event(EVENT_ERROR, EV_SUBTYPE_ERR_NO_BPF);
        }    


        lpf->write(0x00);
        lpf->set_gpio_config(0x00);
        lpf->write(p_init_band);

        bpf->write(0x00);
        bpf->set_gpio_config(0x00);
        bpf->write(p_init_band);

        return res;
 
    }

    void BANDSEL::set(BANDS new_band)
    {
        bpf->write(new_band);
        lpf->write(new_band);
    }