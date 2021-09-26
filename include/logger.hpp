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
    const char *lookup_error(uint16_t error_code);
};

// External reference to logger class
extern class LOGGER logger;

#define __LOGGER_HPP__

#endif
