#ifndef __DISPLAY_HPP__

class DISPLAY_DRIVER
{
    public:
    void begin();
    void refresh();
    void events(event_data ed, uint32_t event_subtype);

    private:
    char *strncpy_z(char *dest, const char *source, uint8_t max_length);
    char *copy_string_field(char *dest, const char *source, uint8_t max_length);
    char *format_frequency(char *dest, uint32_t freq, uint8_t max_length);
    char *blank_field(char *dest, uint8_t max_length);
    void refresh_normal_operation();
    void refresh_error_message();


    uint32_t freq;
    uint32_t freq_b;
    uint32_t tuning_increment;
    uint8_t trx_mode;
    uint8_t display_mode;
    uint8_t sideband;
    uint8_t agc_state;
    char *keypad_keys;
    char *misc_text;
    ed_error_info *p_err_info;

};

#define __DISPLAY_HPP__

#endif
