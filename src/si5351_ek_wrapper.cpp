#include <Arduino.h>
#include <Wire.h>
#include <si5351_ek_wrapper.hpp>

/*
* Initialize the SI5351 without calling Wire.begin
*/

bool SI5351_EK_WRAPPER::init_sans_wire_begin(uint8_t xtal_load_c, uint32_t xo_freq, int32_t corr)
{
	
	// Check for a device on the bus, bail out if it is not there
	Wire.beginTransmission(i2c_bus_addr);
	uint8_t reg_val;
    reg_val = Wire.endTransmission();

	if(reg_val == 0)
	{
		// Wait for SYS_INIT flag to be clear, indicating that device is ready
		uint8_t status_reg = 0;
		do
		{
			status_reg = si5351_read(SI5351_DEVICE_STATUS);
		} while (status_reg >> 7 == 1);

		// Set crystal load capacitance
		si5351_write(SI5351_CRYSTAL_LOAD, (xtal_load_c & SI5351_CRYSTAL_LOAD_MASK) | 0b00010010);

		// Set up the XO reference frequency
		if (xo_freq != 0)
		{
			set_ref_freq(xo_freq, SI5351_PLL_INPUT_XO);
		}
		else
		{
			set_ref_freq(SI5351_XTAL_FREQ, SI5351_PLL_INPUT_XO);
		}

		// Set the frequency calibration for the XO
		set_correction(corr, SI5351_PLL_INPUT_XO);

		reset();

		return true;
	}
	else
	{
		return false;
	}
}

/*
* Set frequency in Hz.
*/

uint8_t SI5351_EK_WRAPPER::set_freq_hz(uint32_t freq_hz, enum si5351_clock output)
    {
      uint64_t freq = 100ULL * freq_hz;
      return set_freq(freq, output);
    }
