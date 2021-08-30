#ifndef __CONFIG_HPP__

#include <bandsel.hpp>

// Clock Source Calibration value
#define CLK_SOURCE_CAL_VALUE -4440

// Carrier oscillator frequency (Depends on crystal filter)

#define CARRIER_OSC_FREQ  9000683UL // Value is in Hz.(9.000683 MHz)

// Crystal filter bandwidth

#define XTAL_FILTER_BW 2234UL       // Crystal Filter bandwidth in Hz

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
     

// I2C Slaves
#define TRX_I2C_ADDR 0x38           // Transceiver control
#define BPF_I2C_ADDR 0x39           // Band pass filter control
#define LPF_I2C_ADDR 0x3A           // Low pass filter control


#define _CONFIG_HPP__

#endif



