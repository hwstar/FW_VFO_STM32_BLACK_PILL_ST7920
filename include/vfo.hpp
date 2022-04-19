//
// VFO class definition
//

#include <assert.h>

#ifndef __VFO_HPP__

//
// EEPROM data structures
//

typedef struct trx_eeprom_master_info {
    char recordname[8];
    char boardname[16];
    char serialnum[5];
    uint8_t pad[3];
} trx_eeprom_master_info;
static_assert(sizeof(trx_eeprom_master_info) == 32, "Size of trx_eeprom_master_info not equal to eeprom page size");

typedef struct vfo_eeprom_master_info {
    char recordname[8];
    char boardname[16];
    char serialnum[5];
    uint8_t pad[3];
} vfo_eeprom_master_info;
static_assert(sizeof(vfo_eeprom_master_info) == 32, "Size of vfo_eeprom_master_info not equal to eeprom page size");

#define RECNAME_MASTER "MASTER"
#define RECNUM_EEPROM_MASTER 0

typedef struct trx_eeprom_if_info { 
    char recordname[8];
    uint32_t second_if_carrier;
    uint32_t second_if_bw_6_db;
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
    uint16_t tx_gain_values[8]; // 16 bytes total
    uint8_t pad[8];
} trx_eeprom_txgain_info;
#define RECNAME_TXGAIN "TXGAIN"
#define RECNUM_EEPROM_TXGAIN 24

typedef struct trx_eeprom_smeter_info { 
    char recordname[8];
    uint16_t s_meter_values[10]; // 20 bytes total
    uint8_t pad[4];
} trx_eeprom_smeter_info;
#define RECNAME_SMETER "SMETER"
#define RECNUM_EEPROM_SMETER 25

static_assert(sizeof(trx_eeprom_smeter_info) == 32, "Size of trx_eeprom_smeter_info not equal to eeprom page size");


typedef struct trx_eeprom_txpower_info { 
    char recordname[8];
    uint16_t txpower_values[8]; // 20 bytes total
    uint8_t pad[8];
} trx_eeprom_txpower_info;
#define RECNAME_TXPOWER "TXPOWER"
#define RECNUM_EEPROM_TXPOWER 26

static_assert(sizeof(trx_eeprom_txpower_info) == 32, "Size of trx_eeprom_txpower_info not equal to eeprom page size");



typedef struct vfo_eeprom_cal_info {
    char recordname[8];
    int16_t cal_value;
    uint8_t pad[22];
} vfo_eeprom_cal_info;
#define RECNAME_VFOCAL "VFOCAL"
#define RECNUM_EEPROM_VFO_CAL 16

static_assert(sizeof(vfo_eeprom_cal_info) == 32, "Size of vfo_eeprom_cal_info not equal to eeprom page size");

typedef struct vfo_eeprom_channel_info {
    char recordname[8];
    uint32_t vfo_freq_a;
    uint32_t vfo_freq_b;
    uint8_t mode;
    uint8_t pad[15];
} vfo_eeprom_channel_info;
#define RECNAME_CHANNEL "CHANNEL"
#define RECNUM_BASE_EEPROM_VFO_CHANNEL 128 // From 128 to 255
#define RECNUM_LAST 255

static_assert(sizeof(vfo_eeprom_channel_info) == 32, "Size of vfo_eeprom_channel_info not equal to eeprom page size");



//
// VFO class
//

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
    bool have_trx_eeprom;
    bool have_trx_dac;
    bool have_vfo_eeprom;
    BANDS last_band;
    uint8_t trx_save;
    uint8_t last_ptt_mode;
    uint8_t band_index;
    uint8_t metering_tx_state;
    uint16_t vmon_adc_reading;
    uint32_t vfo_freq;
    uint32_t high_injection_freq;
    uint32_t low_injection_freq;
    uint32_t tuning_knob_increment;
    uint32_t tx_filter_offset;
    uint32_t s_meter_reading;
    uint32_t vmon_millivolts;
    int32_t swr_forward_voltage;
    int32_t swr_reverse_voltage;
    
    float swr;
    float vmon_value;
  
    trx_eeprom_master_info trx_master_info;
    trx_eeprom_if_info trx_if_info;
    trx_eeprom_txgain_info trx_gain_info;
    trx_eeprom_smeter_info trx_smeter_info;
    trx_eeprom_txpower_info trx_txpower_info;
    vfo_eeprom_master_info vfo_master_info;
    vfo_eeprom_cal_info vfo_cal_info;
    vfo_eeprom_channel_info vfo_channel_info;
    
    void update_clock_gen();
    void update_display_tx(uint8_t val);
    bool set_freq (uint32_t freq); 
    void agc_set(uint8_t state);
    void ptt_set(uint8_t mode);
    void sideband_set(uint8_t mode);
    void set_tx_gain(uint16_t gain);
    void store_tx_gain();
    void display_tuning_increment(uint32_t value);
    void initialize_eeprom();
    void service_metering(event_data ed);
    
};

#define __VFO_HPP__

#endif

