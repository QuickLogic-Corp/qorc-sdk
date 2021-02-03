#include "Fw_global_config.h"

#include "lis2dh12_wire.h"
#include "eoss3_hal_i2c.h"
//#include "eoss3_hal_timer.h"
#include "dbg_uart.h"
#include "Wire.h"

extern "C" void HAL_DelayUSec(uint32_t usecs);

#define LIS2DH12_I2C_ADDR (0x18)

/** Device Identification (Who am I) **/
#define LIS2DH12_ID          0x33U

typedef enum {
  LIS2DH12_STATUS_REG_AUX = 0x07,
  LIS2DH12_OUT_TEMP_L     = 0x0C,
  LIS2DH12_OUT_TEMP_H     = 0x0D,
  LIS2DH12_WHO_AM_I       = 0x0F,
  LIS2DH12_CTRL_REG0      = 0x1E,
  LIS2DH12_TEMP_CFG_REG   = 0x1F,
  LIS2DH12_CTRL_REG1      = 0x20,
  LIS2DH12_CTRL_REG2      = 0x21,
  LIS2DH12_CTRL_REG3      = 0x22,
  LIS2DH12_CTRL_REG4      = 0x23,
  LIS2DH12_CTRL_REG5      = 0x24,
  LIS2DH12_CTRL_REG6      = 0x25,
  LIS2DH12_REFERENCE      = 0x26,
  LIS2DH12_STATUS_REG0    = 0x27,
  LIS2DH12_OUT_X_L        = 0x28,
  LIS2DH12_OUT_X_H        = 0x29,
  LIS2DH12_OUT_Y_L        = 0x2A,
  LIS2DH12_OUT_Y_H        = 0x2B,
  LIS2DH12_OUT_Z_L        = 0x2C,
  LIS2DH12_OUT_Z_H        = 0x2D,
  LIS2DH12_FIFO_CTRL_REG  = 0x2E,
  LIS2DH12_FIFO_SRC_REG   = 0x2F,
  LIS2DH12_INT1_CFG       = 0x30,
  LIS2DH12_INT1_SRC       = 0x31,
  LIS2DH12_INT1_THS       = 0x32,
  LIS2DH12_INT1_DURATION  = 0x33,
  LIS2DH12_INT2_CFG       = 0x34,
  LIS2DH12_INT2_SRC       = 0x35,
  LIS2DH12_INT2_THS       = 0x36,
  LIS2DH12_INT2_DURATION  = 0x37,
  LIS2DH12_CLICK_CFG      = 0x38,
  LIS2DH12_CLICK_SRC      = 0x39,
  LIS2DH12_CLICK_THS      = 0x3A,
  LIS2DH12_TIME_LIMIT     = 0x3B,
  LIS2DH12_TIME_LATENCY   = 0x3C,
  LIS2DH12_TIME_WINDOW    = 0x3D,
  LIS2DH12_ACT_THS        = 0x3E,
  LIS2DH12_ACT_DUR        = 0x3F
} lis2dh12_regnames_t ;


/* Initialization sequence
 */

void LIS2DH12::write_reg(uint8_t reg, uint8_t *p_data, int size)
{
	if (size > 1) reg |= 0x80; // Set MSB of register address to 1 to allow multiple byte writes
	Wire.beginTransmission(_deviceAddress);
	Wire.write(reg);
	Wire.write(p_data, size);
	Wire.endTransmission();
	return;
}

void LIS2DH12::read_reg(uint8_t reg, uint8_t *p_data, int size)
{
	if (size > 1) reg |= 0x80; // Set MSB of register address to 1 to allow multiple byte reads
	Wire.beginTransmission(_deviceAddress);
	Wire.write(reg);
	Wire.endTransmission(false);
	Wire.requestFrom(_deviceAddress, size);
	for (int k = 0; k < size; k++)
	{
		if (Wire.available())
		{
		   *p_data++ = Wire.read();
		}
	}
	return;
}

LIS2DH12::LIS2DH12(void)
{
	_deviceAddress = LIS2DH12_I2C_ADDR;
}

void LIS2DH12::begin(void)
{
     uint8_t val ;
     uint8_t rval[2]={0,0};

     rval[0]=0x00; rval[1]=0x00;
     read_reg(LIS2DH12_WHO_AM_I, rval, 1);
     if (rval[0] != LIS2DH12_ID)
     {
        // Check if Scratch register read succeeded, assert if not.
       dbg_str_int("Error reading Scratch register. Value read", rval[0]);
       dbg_str_int("Expected value", val);
     }

     val = 0x80; write_reg(LIS2DH12_CTRL_REG4, &val, 1); // Write 0x01 --> Reg 0x10
     set_sample_resolution(LIS2DH12_DEFAULT_SAMPLE_BIT_DEPTH);
     set_sample_range(LIS2DH12_DEFAULT_RANGE);
     set_sample_rate(LIS2DH12_DEFAULT_SAMPLE_RATE);

     set_mode(LIS2DH12_MODE_NORMAL);
     HAL_DelayUSec(2000);
     
     dbg_str_int("Sample resolution ", get_sample_resolution());
     dbg_str_int("Sample range      ", get_sample_range());
     dbg_str_int("Sample rate       ", get_sample_rate());

     return;
}

void LIS2DH12::set_mode(uint8_t mode)
{
	set_mode((lis2dh12_modes_t)mode);
}

void LIS2DH12::set_mode(lis2dh12_modes_t mode)
{
   uint8_t modeVal;
   // TODO
}

/* Low-power mode sample rates, when REG[0x1C] => 0x00 */
static int lis2dh12_sample_rates[] =
{ 0, 1, 10, 25, 50, 100, 200, 400, 400 };

void LIS2DH12::set_sample_rate(int sample_rate_in_hz)
{
    uint8_t srate_index ;
    uint8_t ctrl_reg1;
    
    // Find register setting for the given sample rate
    for (srate_index = 5; srate_index < 12; srate_index++) {
      if (lis2dh12_sample_rates[srate_index] < sample_rate_in_hz)
        continue;
    }
    if (sample_rate_in_hz < 1) {
      srate_index = 0;   // power down mode
    }
    else if (sample_rate_in_hz < 10) {
      srate_index = 1; // 1 Hz
    }
    else if (sample_rate_in_hz < 25) {
      srate_index = 2; // 10 Hz
    }
    else if (sample_rate_in_hz < 50) {
      srate_index = 3;  // 25Hz
    }
    else if (sample_rate_in_hz < 100) {
      srate_index = 4; // 50 Hz
    }
    else if (sample_rate_in_hz < 200) {
      srate_index = 5; // 100 Hz
    }
    else if (sample_rate_in_hz < 400) {
      srate_index = 6; // 200 Hz
    }
    else {
      srate_index = 7; // 400 Hz
    }
    
   // Now write to the rate register
   read_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);
   ctrl_reg1 &= 0x0f;
   ctrl_reg1 |= (srate_index << 4);
   write_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);

   dbg_str_int("Sample rate", lis2dh12_sample_rates[srate_index]);
}

int LIS2DH12::get_sample_rate(void)
{
    uint8_t srate_index = 0;
    read_reg(LIS2DH12_CTRL_REG1, &srate_index, 1);
    srate_index >>= 4;
    srate_index &= 0x0f;
    return lis2dh12_sample_rates[srate_index];
}

void LIS2DH12::set_sample_range(lis2dh12_range_t range)
{
    uint8_t range_c_val;

   // Read current range and resolution
   read_reg(LIS2DH12_CTRL_REG4, &range_c_val, 1);

   range_c_val &= 0xCF;
   range_c_val |= (range << 4);
   // Now write to the range and resolution register
   write_reg(LIS2DH12_CTRL_REG4, &range_c_val, 1);
}

int LIS2DH12::get_sample_range(void)
{
    uint8_t range_c_val;
    read_reg(LIS2DH12_CTRL_REG4, &range_c_val, 1);
    range_c_val &= 0x30;
    return (lis2dh12_range_t)(range_c_val >> 4);
}

static int lis2dh12_bit_depths[] = { 6, 7, 8, 10, 12, 14, 14, 14 };
static int lis2dh12_bit_depth_index[] =
{ 0, // [0] => 6-bits
  0, // [1] => 6-bits
  0, // [2] => 6-bits
  0, // [3] => 6-bits
  0, // [4] => 6-bits
  0, // [5] => 6-bits
  0, // [6] => 6-bits
  1, // [7] => 7-bits
  2, // [8] => 8-bits
  3, // [9] => 9-bits
  3, // [10] => 10-bits
  4, // [11] => 11-bits
  4, // [12] => 12-bits
  5, // [13] => 13-bits
  5, // [14] => 14-bits
  5, // [15] => 15-bits
};

void LIS2DH12::set_sample_resolution(int bit_depth)
{
	uint8_t ctrl_reg1, ctrl_reg4;
    uint8_t lpen_bit, hr_bit;
    read_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1); // Read low-power enable bit from CTRL_REG1
    read_reg(LIS2DH12_CTRL_REG4, &ctrl_reg4,   1); // Read high-resolution enable bit from CTRL_REG4

    lpen_bit &= 0x03;
    hr_bit   &= 0x03;
    switch (bit_depth)
    {
    case LIS2DH12_RESOLUTION_8BITS:
    	if (hr_bit)
    	{
    		ctrl_reg4 &= 0xF7; // set HR bit to 0
        	write_reg(LIS2DH12_CTRL_REG4, &ctrl_reg4, 1);
    	}
    	ctrl_reg1 |= 0x08; // set low-power mode to 0
    	write_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);
    	break;
    case LIS2DH12_RESOLUTION_12BITS:
    	ctrl_reg4 |= 0x08; // set high-resolution mode bit to 1
    	write_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);

    	ctrl_reg1 &= 0xF7; // clear low-power mode bit to 0
    	write_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);
    	break;
    default:
    	ctrl_reg4 &= 0xF7; // clear high-resolution mode bit to 0
    	write_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);

    	ctrl_reg1 &= 0xF7; // clear low-power mode bit to 0
    	write_reg(LIS2DH12_CTRL_REG1, &ctrl_reg1, 1);
    	break;
    }
}

int LIS2DH12::get_sample_resolution(void)
{
   uint8_t range_c_val ;
   uint8_t lpen_bit, hr_bit;
   read_reg(LIS2DH12_CTRL_REG1, &lpen_bit, 1); // Read low-power enable bit from CTRL_REG1
   read_reg(LIS2DH12_CTRL_REG4, &hr_bit,   1); // Read high-resolution enable bit from CTRL_REG4
   if (lpen_bit == 1)
   {
	   return 8;
   }
   else if (hr_bit == 1)
   {
	   return 12;
   }
   else
   {
	   return 10;
   }
}

xyz_t LIS2DH12::read(void)
{
  uint8_t val[2] = {0, 0};
  xyz_t   xyz_data;
  // New data is available, Read x,y,z values
  read_reg(LIS2DH12_OUT_X_L, (uint8_t *)&xyz_data, 6);
  return xyz_data;
}

