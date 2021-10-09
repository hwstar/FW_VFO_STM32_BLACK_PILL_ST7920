#ifndef __LOGGER_HPP__

#define ERR_NO_BPF 100
#define ERR_NO_LPF 101
#define ERR_NO_TRX 102
#define ERR_NO_CLK_GEN 103

class LOGGER 
{
    public:
    void error(uint16_t error_code);

    private:
    bool lookup_error(uint16_t error_code, const char **long_str, const char **short_str);
};

// External reference to logger class
extern class LOGGER logger;

#define __LOGGER_HPP__

#endif
