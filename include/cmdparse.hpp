#ifndef __CMDPARSE_HPP__

class CMDPARSE
{
    public:
        void handler(event_data ed, uint32_t event_subtype);

    private:
        void reset();
        event_data k_ed;
        uint8_t keypad_digits_index;
        uint8_t command_string_index;
        bool cp_match;
        uint8_t state = 0;
        uint8_t f_count;
        uint32_t m,f,vf;
        uint8_t command_timer = 0;
        char keypad_digits[16];
        char command_string[16];
};





#define __CMDPARSE_HPP__
#endif


