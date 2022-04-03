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

enum BANDS {BAND_10M = 0x1, BAND_12M = 0x2, BAND_15M = 0x04, BAND_17M = 0x08, BAND_20M = 0x10, BAND_40M = 0x20, BAND_80M = 0x40, BAND_160M = 0x80};


// Band 1
#define BAND_FILTER_1       BAND_160M
#define BAND_EDGE_LOW_1     1800000UL // Values are in Hz.
#define BAND_EDGE_HIGH_1    2000000UL
#define BAND_DEF_USB_1      false

// Band 2
#define BAND_FILTER_2       BAND_80M
#define BAND_EDGE_LOW_2     3500000UL
#define BAND_EDGE_HIGH_2    4000000UL
#define BAND_DEF_USB_2      false

// Band 3
#define BAND_FILTER_3       BAND_40M
#define BAND_EDGE_LOW_3     7000000UL
#define BAND_EDGE_HIGH_3    7300000UL
#define BAND_DEF_USB_3      false

// Band 4
#define BAND_FILTER_4       BAND_20M
#define BAND_EDGE_LOW_4     14000000UL
#define BAND_EDGE_HIGH_4    14350000UL
#define BAND_DEF_USB_4      true

// Band 5
#define BAND_FILTER_5       BAND_17M
#define BAND_EDGE_LOW_5     18068000UL
#define BAND_EDGE_HIGH_5    18168000UL
#define BAND_DEF_USB_5      true

// Band 6
#define BAND_FILTER_6       BAND_15M
#define BAND_EDGE_LOW_6     21000000UL
#define BAND_EDGE_HIGH_6    21450000UL
#define BAND_DEF_USB_6      true

// Band 7
#define BAND_FILTER_7       BAND_12M
#define BAND_EDGE_LOW_7     24890000UL
#define BAND_EDGE_HIGH_7    24990000UL 
#define BAND_DEF_USB_7      true

// Band 8
#define BAND_FILTER_8       BAND_10M
#define BAND_EDGE_LOW_8     28000000UL
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

#ifndef QUAD_OUTPUT_VFO_BOARD
#define REF_TCXO_FREQ_HZ 25000000 // 25 MHz for VFO with Etherkit breakout board
#define CLK_SOURCE_CAL_VALUE -4560 // VFO calibration value for the Etherkit SI3531 in prototype S/N #1
#define SECOND_IF_UPPER_M6DB 12287533 // Second IF upper -6dB point crystal filter passband
#define SECOND_IF_LOWER_M6DB 12284967 // Second IF lower -6dB point crystal filter passband

#else
#define VFO_BOARD_NAME "VFOC2" // Board name for concept 2 VFO with on-board eeprom
#define REF_TCXO_FREQ_HZ 26000000 // 26 MHz for the quad output SI5351 VFO board
#define CLK_SOURCE_CAL_VALUE -395  // VFO clock calibration value for Quad output VFO board
#define SECOND_IF_UPPER_M6DB 12287500 // Second IF upper -6dB point crystal filter passband
#define SECOND_IF_LOWER_M6DB 12284950 // Second IF lower -6dB point crystal filter passband
#endif


// Below are the gain constants needed to get 10W out of the final using the TRX tune oscillator leval adjustment pot
// R1713 set to 475mV p-p at TP1701. These constants are used to flatten out the TX power across all bands
// from the TRX motherboard through the low pass filter bank, the power amplifier, and the low pass filter bank.
// If any of these boards are swapped or re-ajusted, these gain settings will need to be updated.


// These constants equate to are the DAC codes needed to produce an output voltage on the TRX DAC, U401
// which sets the gain of the 12.288 IF amplifiers.

// TRX DAC U401 is referenced to 5.0V and has 4096 steps. The voltage per step is therefore 1.22mV.


#ifndef QUAD_OUTPUT_VFO_BOARD

#define TRX_TXGAIN_160M 1500
#define TRX_TXGAIN_80M 1450
#define TRX_TXGAIN_40M 1600
#define TRX_TXGAIN_20M 1650
#define TRX_TXGAIN_17M 1700
#define TRX_TXGAIN_15M 1650
#define TRX_TXGAIN_12M 1725
#define TRX_TXGAIN_10M 1950

#else // Etherkit SI5351

#define TRX_TXGAIN_160M 1500
#define TRX_TXGAIN_80M 1450
#define TRX_TXGAIN_40M 1600
#define TRX_TXGAIN_20M 1650
#define TRX_TXGAIN_17M 1700
#define TRX_TXGAIN_15M 1650
#define TRX_TXGAIN_12M 1725
#define TRX_TXGAIN_10M 1950

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




// Calculated from the first IF and second IF constants above

#define SECOND_IF_CARRIER  (SECOND_IF_UPPER_M6DB + 300)
#define SECOND_IF_BW6DB  (SECOND_IF_UPPER_M6DB - SECOND_IF_LOWER_M6DB)

#define FIRST_TO_SECOND_IF_DELTA (FIRST_IF_FCENTER - SECOND_IF_CARRIER)


#define __CONFIG_HPP__

#endif



