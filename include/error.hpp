
#ifndef __ERROR_HPP__

#define ERROR_LEVEL_HARD 1
#define ERROR_LEVEL_SOFT 0

#define ERROR_MAX_LINE_LENGTH 30

typedef struct ed_error_info {
    uint16_t errcode;
    uint16_t errlevel;
    char line_1[ERROR_MAX_LINE_LENGTH];
    char line_2[ERROR_MAX_LINE_LENGTH];
} ed_error_info;

class ERROR_HANDLER
{

public:
    void handler(event_data ed, uint32_t event_subtype);
    void begin();
private:
    bool hard_error;
    
};

#endif
