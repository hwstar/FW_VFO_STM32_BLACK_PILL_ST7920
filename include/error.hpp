
#ifndef __ERROR_HPP__

#define ERROR_LEVEL_HARD 1
#define ERROR_LEVEL_SOFT 0


class ERROR_HANDLER
{

public:
    void handler(event_data ed, uint32_t event_subtype);
    void begin();
private:
    bool hard_error;
    
};

#endif
