#ifndef __CONFIG_HPP__

//
// Black Pill pins used
//

// Rotary Encoder
#define PIN_ENCODER_I PB0
#define PIN_ENCODER_Q PB1


// Keypad
#define PIN_KEYPAD_C1 PB12
#define PIN_KEYPAD_C2 PB13
#define PIN_KEYPAD_C3 PB14
#define PIN_KEYPAD_C4 PB15
#define PIN_KEYPAD_R1 PB4
#define PIN_KEYPAD_R2 PB5
#define PIN_KEYPAD_R3 PB8
#define PIN_KEYPAD_R4 PB9

// Switches
#define PIN_PTT PB10 
#define PIN_TUNE PA1
#define PIN_ENCODER_SWITCH PB2

// I2C 
#define PIN_I2C_SDA PB7 // I2C1
#define PIN_I2C_SCL PB6 // I2C1

// SPI Display
#define PIN_SPI_CLK PA5 // SPI1
#define PIN_SPI_MOSI PA7 // SPI1
#define PIN_SPI_MISO PA6 // SPI1
#define PIN_SPI_CS PA8 // Display chip select

// UART
#define PIN_UART_TX PA9 // SERIAL1
#define PIN_UART_RX PA10 // SERIAL1

// Outputs
#define PIN_DISPLAY_BACKLIGHT PA11 
#define PIN_PA_FAN_ENABLE PB3
#define PIN_STM32_LED PC13 // Black pill LED.
#define PIN_TEST_OUTPUT PA3

// ADC inputs
#define S_METER_ADC PA0 // Define ADC pin for S Meter if it is going to be used
//#define VMON_ADC PA4    // Define ADC pin for Voltage Monitor if it is going to be used.

//

//
// Unused pins
//
// PA2
// PA4
// PA11 - USB DM
// PA12 - USB DP
// PA15
// PC14
// PC15
// 
   
/*
* I2C Device addresses
* 7 bit I2C addresses
*/

#define MUX_I2C_ADDRESS 0x70        // I2C mux on dual SI5351 board
#define TRX_I2C_ADDR 0x38           // Transceiver control
#define EEPROM_I2C_ADDR 0x50        // VFO and TRX eeproms
// 0x60 reserved for the Si5351s
#define TRX_DAC_I2C_ADDR 0x62       // Transceiver MCP4725 TXGAIN DAC
#define BPF_I2C_ADDR 0x39           // Band pass filter control
#define LPF_I2C_ADDR 0x3A           // Low pass filter control
#define SWR_BRIDGE_DAC_ADDR 0x68    // SWR bridge DAC 


/*
* Band configuration
*/

#define MAX_BANDS 8

enum BANDS {BAND_10M = 0x1, BAND_12M = 0x2, BAND_15M = 0x04, BAND_17M = 0x08, BAND_20M = 0x10, BAND_40M = 0x20, BAND_80M = 0x40, BAND_160M = 0x80};


// Band 1
#define BAND_FILTER_1       BAND_160M
#define BAND_NAME_1         "160M"
#define BAND_EDGE_LOW_1     1800000UL // Values are in Hz.
#define BAND_LANDING_1      1900000UL
#define BAND_EDGE_HIGH_1    2000000UL
#define BAND_DEF_USB_1      false

// Band 2
#define BAND_FILTER_2       BAND_80M
#define BAND_NAME_2         "80M"
#define BAND_EDGE_LOW_2     3500000UL
#define BAND_LANDING_2      3900000UL
#define BAND_EDGE_HIGH_2    4000000UL
#define BAND_DEF_USB_2      false

// Band 3
#define BAND_FILTER_3       BAND_40M
#define BAND_NAME_3         "40M"
#define BAND_EDGE_LOW_3     7000000UL
#define BAND_LANDING_3      7200000UL
#define BAND_EDGE_HIGH_3    7300000UL
#define BAND_DEF_USB_3      false

// Band 4
#define BAND_FILTER_4       BAND_20M
#define BAND_NAME_4         "20M"
#define BAND_EDGE_LOW_4     14000000UL
#define BAND_LANDING_4      14250000UL
#define BAND_EDGE_HIGH_4    14350000UL
#define BAND_DEF_USB_4      true

// Band 5
#define BAND_FILTER_5       BAND_17M
#define BAND_NAME_5         "17M"
#define BAND_EDGE_LOW_5     18068000UL
#define BAND_LANDING_5      18130000UL
#define BAND_EDGE_HIGH_5    18168000UL
#define BAND_DEF_USB_5      true

// Band 6
#define BAND_FILTER_6       BAND_15M
#define BAND_NAME_6         "15M"
#define BAND_EDGE_LOW_6     21000000UL
#define BAND_LANDING_6      21250000UL
#define BAND_EDGE_HIGH_6    21450000UL
#define BAND_DEF_USB_6      true

// Band 7
#define BAND_FILTER_7       BAND_12M
#define BAND_NAME_7         "12M"
#define BAND_EDGE_LOW_7     24890000UL
#define BAND_LANDING_7      24970000UL
#define BAND_EDGE_HIGH_7    24990000UL 
#define BAND_DEF_USB_7      true

// Band 8
#define BAND_FILTER_8       BAND_10M
#define BAND_NAME_8         "10M"
#define BAND_EDGE_LOW_8     28000000UL
#define BAND_LANDING_8      28300000UL
#define BAND_EDGE_HIGH_8    30000000UL
#define BAND_DEF_USB_8      true


/*
* Miscellaneous settings
*/

#define CONFIG_COMMAND_TIMEOUT 70 // 7 second command time out
#define CONFIG_TX_FAN_THRESH_SEC 30 // TX fan will turn on after this key down exceeds this time in seconds.

/*
* Build configurations
*/


//
// Define if dual 5351 board with 4 outputs is used
//

#define QUAD_OUTPUT_VFO_BOARD 

//
// VFO Module test mode skips checking for other boards in the system when testing the VFO standalone
//

//#define VFO_MODULE_TEST_MODE

//
// Show VFO calibration info on display (Currently limited to SMETER ADC values, and TX power)
//

//#define VFO_SHOW_CAL_INFO

//
// Initialize VFO EEPROM (supported on Quad VFO only)
//

//#define INITIALIZE_VFO_EEPROM

//
// Initialize TRX eeprom constants to be stored in the TRX board EEPROM
//

//#define INITIALIZE_TRX_EEPROM // Define to force initialization of the TRX EEPROM using the constants below


/*
* Start of EEPROM constants
*/


#define SYSTEM_NAME  "LEFTY TRX" // Name for all of the boards comprising a transceiver
#define TRX_BOARD_NAME "TRXC2" // Transceiver board name
#define TRX_SER_NUM "0003"
#define VFO_SER_NUM "0001"

//
// First IF intiialization values
//

//
// Center frequency of first IF monolithic crystal filter (does not change from board to board)
//
#define FIRST_IF_FCENTER 45100000

//
// Bandwidth of the first if monolithic crystal filter (does not change from board to board)
//

#define FIRST_IF_BW6DB 15000

//
// Contants specific to the different versions of VFO boards used
//

#define VFO_BOARD_NAME "VFOC2" // Board name for concept 2 VFO with on-board eeprom
#define REF_TCXO_FREQ_HZ 26000000 // 26 MHz for the quad output SI5351 VFO board
#define CLK_SOURCE_CAL_VALUE -395  // VFO clock calibration value for Quad output VFO board
#define SECOND_IF_UPPER_M6DB 12287564 // Second IF upper -6dB point crystal filter passband
#define SECOND_IF_LOWER_M6DB 12285077 // Second IF lower -6dB point crystal filter passband



// Below are the gain constants needed to get 10W out of the final using the TRX tune oscillator leval adjustment pot
// R1713 set to 3.5V p-p at TP1701. These constants are used to flatten out the TX power across all bands
// from the TRX motherboard through the low pass filter bank, the power amplifier, and the low pass filter bank.
// If any of these boards are swapped or re-ajusted, these gain settings will need to be updated.


// These constants equate to are the DAC codes needed to produce an output voltage on the TRX DAC, U401
// which sets the gain of the 12.288 MHz IF amplifiers.

// TRX DAC U401 is referenced to 5.0V and has 4096 steps. The voltage per step is therefore 1.22mV.
// These are stored in the TRX eeprom.



#define TRX_TXGAIN_160M 1450
#define TRX_TXGAIN_80M 1435
#define TRX_TXGAIN_40M 1485
#define TRX_TXGAIN_20M 1525
#define TRX_TXGAIN_17M 1540
#define TRX_TXGAIN_15M 1510
#define TRX_TXGAIN_12M 1560
#define TRX_TXGAIN_10M 1640


// S Meter Calibration
// These constants map ADC counts to S-Units
// Constants are with respect to first enclosed radio.
// These are stored in the TRX eeprom


#define S_UNIT_2    1355 // -115 dBm
#define S_UNIT_3    1330 // -109 dBm
#define S_UNIT_4    1315 // -103 dBm
#define S_UNIT_5    1280 // -97 dBm
#define S_UNIT_6    1206 // -91 dBm
#define S_UNIT_7    1075 // -85 dBm
#define S_UNIT_8    885  // -79 dBm
#define S_UNIT_9    698  // -73 dBm
#define S_UNIT_10   530  // -63 dBm
#define S_UNIT_20   470  // -53 dBm


// Relative TX power for adc forward power measurement
// Actual power varies depending on the operating band.
// This should not be relied upon to give an accurate power measurement

#define TX_POWER_LEVEL_1 330
#define TX_POWER_LEVEL_2 440
#define TX_POWER_LEVEL_3 550
#define TX_POWER_LEVEL_4 660
#define TX_POWER_LEVEL_5 770
#define TX_POWER_LEVEL_6 880
#define TX_POWER_LEVEL_7 1100
#define TX_POWER_LEVEL_8 1282

//
// If the VMON_ADC pin is defined, define the calibration value
//

#ifdef VMON_ADC
#define VMON_ADC_CAL_FACTOR 1.0
#endif 

/*
* END TRX eeprom constants
*/



// Emission modes

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_DEFAULT 100

// Radio modes
#define RADIO_RX 0
#define RADIO_TX 1
#define RADIO_TUNE 2

// Max number of menu items
#define MAX_MENU_ITEMS 4
// Max number of menu levels
#define MAX_MENU_LEVELS 3

#define __CONFIG_HPP__

#endif



