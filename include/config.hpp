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

//
// Unused pins
// 
// PA0
// PA2
// PA3
// PA4  
// PA11 - USB DM
// PA12 - USB DP
// PA15
// PC14
// PC15
// 


// Emission modes

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_DEFAULT 100

// Radio modes
#define RADIO_RX 0
#define RADIO_TX 1
#define RADIO_TUNE 2

// Clock Source Calibration value
// From Etherkit Clock source calibration Records
#define CLK_SOURCE_CAL_VALUE -4560 

// Carrier oscillator frequency (Depends on crystal filter)
// Empirically determined. Measured with test equipment.

//#define CARRIER_OSC_FREQ  9000683UL // REVX2 #1 Value is in Hz.(9.000683 MHz)
#define CARRIER_OSC_FREQ  9000736UL // REV X3 #1 Value is in Hz.(9.000736 MHz)

// Band bits

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


// Local oscillator outputs on clock generator
// ID numbers depend on the type of clock generator used.

#define FIRST_LO_ID 0               // Is the VFO in RX and the Carrier oscillator in TX
#define SECOND_LO_ID 2              // Is the BFO in RX, and the VFO in TX
     
//
// I2C Slaves
// 7 bit I2C addresses
//
#define TRX_I2C_ADDR 0x38           // Transceiver control
#define BPF_I2C_ADDR 0x39           // Band pass filter control
#define LPF_I2C_ADDR 0x3A           // Low pass filter control


#define __CONFIG_HPP__

#endif



