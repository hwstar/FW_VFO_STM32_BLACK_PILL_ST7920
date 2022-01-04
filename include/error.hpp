#include <event.hpp>

#ifndef __BANDSEL_HPP__

class ERROR
{

public:
    void begin(void (*event_callback)(uint32_t, uint32_t, event_data));
    void handler(event_data ed, uint32_t event_subtype);
private:
    void (*ev_cb)(uint32_t, uint32_t, event_data);

};

#endif
