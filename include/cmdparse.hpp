#ifndef __CMDPARSE_HPP__

typedef struct kp_commands
{
    char c_num[5];
    char c_str[11];
    bool (*cmd_function)(char *command_string, uint8_t command_string_index);
} kp_commands;


class CMDPARSE
{
    public:
        void handler(event_data ed, uint32_t event_subtype);

    private:
        void reset();
        bool parse_command(char *keypad_string, uint8_t keypad_string_index);

        event_data k_ed;
        uint8_t keypad_digits_index;
        bool cp_match;
        uint8_t state = 0;
        uint8_t f_count;
        uint32_t m,f,vf;
        uint8_t command_timer = 0;
        char keypad_digits[16];
    
};





#define __CMDPARSE_HPP__
#endif


