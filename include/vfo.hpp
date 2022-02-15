//
// VFO class definition
//

#include <assert.h>

#ifndef __VFO_HPP__

typedef struct trx_eeprom_master_info {
    char recordname[8];
    char boardname[16];
    uint8_t pad[8];
} trx_eeprom_master_info;
#define RECNAME_MASTER "MASTER"
#define RECNUM_EEPROM_MASTER 0

static_assert(sizeof(trx_eeprom_master_info) == 32, "Size of trx_eeprom_master_info not equal to eeprom page size");


typedef struct trx_eeprom_if_info { 
    char recordname[8];
    uint32_t second_if_carrier;
    uint32_t second_if_bw;
    uint32_t first_if_bw_6_db;
    uint32_t first_if_fcenter;
    uint32_t first_to_second_if_delta; // 28 bytes total
    uint8_t pad[4];
} trx_eeprom_if_info;
#define RECNAME_TRXIF "TRXIF"
#define RECNUM_EEPROM_IF 16

static_assert(sizeof(trx_eeprom_if_info) == 32, "Size of trx_eeprom_if_info not equal to eeprom page size");

typedef struct trx_eeprom_txgain_info { 
    char recordname[8];
    uint16_t tx_gain_values[8]; // 24 bytes total
    uint8_t pad[8];
} trx_eeprom_txgain_info;
#define RECNAME_TXGAIN "TXGAIN"
#define RECNUM_EEPROM_TXGAIN 24

static_assert(sizeof(trx_eeprom_txgain_info) == 32, "Size of trx_eeprom_txgain_info not equal to eeprom page size");


class VFO
{
    public:
    
    bool begin(uint32_t init_freq = 14250000UL);
    uint32_t get_freq(); 
    uint8_t ptt_get();
    uint8_t mode_get();
    uint32_t incr_get(); 
    void subscriber(event_data ed, uint32_t event_subtype);


    private:
    bool is_usb;
    bool is_txing;
    bool test_mode;
    BANDS last_band;
    uint32_t vfo_freq;
    uint32_t high_injection_freq;
    uint32_t low_injection_freq;
    uint32_t tuning_knob_increment;
    int32_t tx_filter_offset;
    uint8_t trx_save;
    uint8_t last_ptt_mode;
    uint8_t band_index;
    uint8_t have_trx_eeprom;
    uint8_t have_trx_dac;
    trx_eeprom_master_info trx_master_info;
    trx_eeprom_if_info trx_if_info;
    trx_eeprom_txgain_info trx_gain_info;

    void update_clock_gen();
    void update_display_tx(uint8_t val);
    bool set_freq (uint32_t freq); 
    void agc_set(uint8_t state);
    void ptt_set(uint8_t mode);
    void mode_set(uint8_t mode);
    void set_tx_gain(uint16_t gain);
    void store_tx_gain();
  

};

#define __VFO_HPP__

#endif

