#ifndef MCP3422_HPP


class MCP3422 {
    public:
    bool begin(uint8_t a_device_address, bool a_one_shot, uint8_t a_pga, uint8_t a_sample_rate);
    bool ready();
    bool present() { return dev_present; };
    void select_channel(uint8_t channel);
    bool read(int32_t &result);
  

    private:
    bool write_config_reg();
    uint8_t device_address;
    uint8_t config_reg;
    uint8_t dev_present;
    uint8_t bytes_read;
    uint8_t buffer[4];
};

#endif