#ifndef __KNOB_HPP__
#define KM_TUNING 0 // Tuning mode

class KNOB {
public:
    KNOB(uint8_t initial_mode){
        mode = initial_mode;
    }

    void set_mode(uint8_t new_mode){
        mode = new_mode;
    }

    void subscriber(event_data ed, uint32_t event_subtype);
    

private:
    uint8_t mode;
    void tuning(event_data ed, uint32_t event_subtype);

};


#define __KNOB_HPP__

#endif
