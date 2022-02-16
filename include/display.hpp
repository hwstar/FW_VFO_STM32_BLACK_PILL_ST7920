#ifndef __DISPLAY_HPP__

class DISPLAY_DRIVER
{
    public:
    void begin();
    void refresh_normal_operation();
    void refresh_error_message();
    void refresh();
    void events(event_data ed, uint32_t event_subtype);

    private:
    uint32_t freq;
    uint8_t trx_mode;
    uint8_t display_mode;
    uint8_t sideband;
    uint8_t agc_state;
    char *keypad_keys;
    ed_error_info *p_err_info;
    
    
};

#define __DISPLAY_HPP__

#endif
