#include "Fw_global_config.h"

#include "mc3635_wire.h"
#include "eoss3_hal_i2c.h"
//#include "eoss3_hal_timer.h"
#include "dbg_uart.h"
#include "Wire.h"

extern "C" void HAL_DelayUSec(uint32_t usecs);

#define MC3635_I2C_ADDR (0x4C)
typedef enum {
  MC3635_EXT_STAT_1 = 0x00,
  MC3635_EXT_STAT_2 = 0x01,
  MC3635_XOUT_LSB   = 0x02,
  MC3635_XOUT_MSB   = 0x03,
  MC3635_YOUT_LSB   = 0x04,
  MC3635_YOUT_MSB   = 0x05,
  MC3635_ZOUT_LSB   = 0x06,
  MC3635_ZOUT_MSB   = 0x07,
  MC3635_STATUS_1   = 0x08,
  MC3635_STATUS_2   = 0x09,
  /* 0x0A - 0x0C Reserved */  
  MC3635_FREG_1     = 0x0D,
  MC3635_FREG_2     = 0x0E,
  MC3635_INIT_1     = 0x0F,
  MC3635_MODE_C     = 0x10,
  MC3635_RATE_1     = 0x11,
  MC3635_SNIFF_C    = 0x12,
  MC3635_SNIFFTH_C  = 0x13,
  MC3635_SNIFFCF_C  = 0x14,
  MC3635_RANGE_C    = 0x15,
  MC3635_FIFO_C     = 0x16,
  MC3635_INTR_C     = 0x17,
  /* 0x18 - 0x19 Reserved */
  MC3635_INIT_3     = 0x1A,
  MC3635_SCRATCH    = 0x1B,
  MC3635_PMCR       = 0x1C,
  /* 0x1D - 0x1F Reserved */
  MC3635_DMX        = 0x20,
  MC3635_DMY        = 0x21,
  MC3635_DMZ        = 0x22,
  /* 0x1D - 0x1F Reserved */
  MC3635_RESET      = 0x24,
  /* 0x25 - 0x27 Reserved */
  MC3635_INIT_2     = 0x28,
  MC3635_TRIGC      = 0x29,
  MC3635_XOFFL      = 0x2A,
  MC3635_XOFFH      = 0x2B,
  MC3635_YOFFL      = 0x2C,
  MC3635_YOFFH      = 0x2D,
  MC3635_ZOFFL      = 0x2E,
  MC3635_ZOFFH      = 0x2F,
  MC3635_XGAIN      = 0x30,
  MC3635_YGAIN      = 0x31,
  MC3635_ZGAIN      = 0x32
  /* 0x33 - 0x3F Reserved */

} mc3635_regnames_t ;


/* Initialization sequence
   reg[0x10] =  1
   reg[0x24] = 0x40
   reg[0x0d] = 0x40
   reg[0x0F] = 0x42
   reg[0x20] = 0x01
   reg[0x21] = 0x80
   reg[0x28] = 0
   reg[0x1a] = 0
   reg[0x1b] = 0xa5
   read scratch register reg[0x1b]
   reg[0x1b] = 0x5a
   read scratch register reg[0x1b] 
 */

void MC3635::write_reg(uint8_t reg, uint8_t *p_data, int size)
{
	Wire.beginTransmission(_deviceAddress);
	Wire.write(reg);
	Wire.write(p_data, size);
	Wire.endTransmission();
	return;
}

void MC3635::read_reg(uint8_t reg, uint8_t *p_data, int size)
{
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

MC3635::MC3635(void)
{
	_deviceAddress = MC3635_I2C_ADDR;
}

void MC3635::begin(void)
{
     uint8_t val ;
     uint8_t rval[2]={0,0};

     val = 0x01; write_reg(MC3635_MODE_C, &val, 1); // Write 0x01 --> Reg 0x10
     val = 0x40; write_reg(MC3635_RESET,  &val, 1); // Write 0x40 --> Reg 0x24
     HAL_DelayUSec(1500);                          // Wait at least 1mSec
     val = 0x40; write_reg(MC3635_FREG_1, &val, 1); // Write 0x40 --> Reg 0x0D
     val = 0x42; write_reg(MC3635_INIT_1, &val, 1); // Write 0x42 --> Reg 0x0F
     val = 0x01; write_reg(MC3635_DMX,    &val, 1); // Write 0x01 --> Reg 0x20
     val = 0x80; write_reg(MC3635_DMY,    &val, 1); // Write 0x80 --> Reg 0x21
     val = 0x00; write_reg(MC3635_INIT_2, &val, 1); // Write 0x00 --> Reg 0x28
     val = 0x00; write_reg(MC3635_INIT_3, &val, 1); // Write 0x00 --> Reg 0x1A
     
     val = 0x00; write_reg(MC3635_FREG_2, &val, 1); // Write 0x00 --> Reg 0x0E
     // set WRAPA to include registers in range 2-9
     //val = 0x01; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FREG_2, &val, 1); // Write 0x01 --> Reg 0x0D

     // Read scratch register
     val = 0xA5;      write_reg(MC3635_SCRATCH, &val, 1);
     rval[0]=0x00; rval[1]=0x00;
     read_reg(MC3635_SCRATCH, rval, 1);
     if (rval[0] != val)
     {
        // Check if Scratch register read succeeded, assert if not.
       dbg_str_int("Error reading Scratch register. Value read", rval[0]);
       dbg_str_int("Expected value", val);
     }
     val = 0x5A;      write_reg(MC3635_SCRATCH, &val, 1);
     rval[0]=0x00; rval[1]=0x00;
     read_reg(MC3635_SCRATCH, rval, 1);
     if (rval[0] != val)
     {
        // Check if Scratch register read succeeded, assert if not.
       dbg_str_int("Error reading Scratch register. Value read", rval[0]);
       dbg_str_int("Expected value", val);
     }

     set_sample_resolution(MC3635_DEFAULT_SAMPLE_BIT_DEPTH);
     set_sample_range(MC3635_DEFAULT_RANGE);
     set_sample_rate(MC3635_DEFAULT_SAMPLE_RATE);

     set_mode(MC3635_MODE_CWAKE);
     HAL_DelayUSec(2000);
     
     dbg_str_int("Sample resolution ", get_sample_resolution());
     dbg_str_int("Sample range      ", get_sample_range());
     dbg_str_int("Sample rate       ", get_sample_rate());

     return;
}

void MC3635::set_mode(uint8_t mode)
{
	set_mode((mc3635_modes_t)mode);
}

void MC3635::set_mode(mc3635_modes_t mode)
{
   uint8_t modeVal;
   uint8_t init1Val = 0x42;

   // Read mode control register to get current mode
   read_reg(MC3635_MODE_C, &modeVal, 1);

   modeVal &= 0xf0;
   modeVal |= mode;

   // Write to the initialization register reg[0x0F] = 0x42
   write_reg(MC3635_INIT_1, &init1Val, 1); // mode control register

   // Now write to the mode control register
   write_reg(MC3635_MODE_C, &modeVal, 1); // mode control register
   //dbg_str_int("MC3635 Mode set to", mode);
}

/* Low-power mode sample rates, when REG[0x1C] => 0x00 */
static int mc3635_sample_rates[] =
{ 0, 0, 0, 0, 0, 14, 28, 54, 105, 210, 400, 600, 600, 600, 600, 750 };

void MC3635::set_sample_rate(int sample_rate_in_hz)
{
    uint8_t srate_index ;
    uint8_t curr_mode;
    
    read_reg(MC3635_MODE_C, &curr_mode, 1);
    curr_mode &= 0x07;
    set_mode(MC3635_MODE_STANDBY);

    // Find register setting for the given sample rate
    for (srate_index = 5; srate_index < 12; srate_index++) {
      if (mc3635_sample_rates[srate_index] < sample_rate_in_hz)
        continue;
    }
    if (sample_rate_in_hz < 28) {
      srate_index = 5;   // 14 Hz
    }
    else if (sample_rate_in_hz < 54) {
      srate_index = 6; // 28 Hz
    }
    else if (sample_rate_in_hz < 105) {
      srate_index = 7; // 54 Hz
    }
    else if (sample_rate_in_hz < 210) {
      srate_index = 8;  // 105Hz
    }
    else if (sample_rate_in_hz < 400) {
      srate_index = 9; // 210 Hz
    }
    else if (sample_rate_in_hz < 600) {
      srate_index = 10; // 400 Hz
    }
    else {
      srate_index = 11; // 600 Hz
    }
    
   // Now write to the rate register
   write_reg(MC3635_RATE_1, &srate_index, 1);
   
   // Restore mode
   set_mode(curr_mode);
   dbg_str_int("Sample rate", mc3635_sample_rates[srate_index]);
}

int MC3635::get_sample_rate(void)
{
    uint8_t srate_index = 0;
    read_reg(MC3635_RATE_1, &srate_index, 1);
    return mc3635_sample_rates[srate_index];
}

void MC3635::set_sample_range(mc3635_range_t range)
{
    uint8_t range_c_val;

   set_mode(MC3635_MODE_STANDBY);

   // Read current range and resolution
   read_reg(MC3635_RANGE_C, &range_c_val, 1);

   range_c_val &= 0x8F;
   range_c_val |= (range << 4);
   // Now write to the range and resolution register
   write_reg(MC3635_RANGE_C, &range_c_val, 1);
}

int MC3635::get_sample_range(void)
{
    uint8_t range_c_val;
    read_reg(MC3635_RANGE_C, &range_c_val, 1);
    return (mc3635_range_t)(range_c_val >> 4);
}

static int mc3635_bit_depths[] = { 6, 7, 8, 10, 12, 14, 14, 14 };
static int mc3635_bit_depth_index[] =
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

void MC3635::set_sample_resolution(int bit_depth)
{
    uint8_t range_c_val;

    set_mode(MC3635_MODE_STANDBY);
    
    // Read current range and resolution
    read_reg(MC3635_RANGE_C, &range_c_val, 1); // mode control register
    range_c_val &= 0xF8;
 
    bit_depth = (bit_depth > MC3635_RESOLUTION_14BITS) ? MC3635_RESOLUTION_14BITS : bit_depth;
    bit_depth &= 15;
    bit_depth = mc3635_bit_depth_index[bit_depth];
    range_c_val |= bit_depth;

   // Now write to the range and resolution register
   write_reg(MC3635_RANGE_C, &range_c_val, 1); // mode control register
}

int MC3635::get_sample_resolution(void)
{
   uint8_t range_c_val ;
   read_reg(MC3635_RANGE_C, &range_c_val, 1); // mode control register
   return mc3635_bit_depths[range_c_val & 0x07];
}

xyz_t MC3635::read(void)
{
  uint8_t val[2] = {0, 0};
  xyz_t   xyz_data;
  // New data is available, Read x,y,z values
  read_reg(MC3635_XOUT_LSB, (uint8_t *)&xyz_data, 6);
  return xyz_data;
}

