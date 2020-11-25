#include "Fw_global_config.h"
#include "mc3635.h"
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_timer.h"
#include "dbg_uart.h"

I2C_Config i2c0config =
{
  .eI2CFreq = I2C_400KHZ,    // 400kHz
  .eI2CInt = I2C_DISABLE,    // enabled interrupt
  .ucI2Cn = 0
};
extern HAL_StatusTypeDef HAL_I2C_Read_UsingRestart(UINT8_t ucDevAddress, UINT8_t ucAddress, UINT8_t *pucDataBuf, UINT32_t uiLength);

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
void mc3635_init(void)
{
     uint8_t val ;
     uint8_t rval[2]={0,0};

     HAL_I2C_Init(i2c0config);

     val = 0x01; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_MODE_C, &val, 1); // Write 0x01 --> Reg 0x10 
     val = 0x40; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_RESET,  &val, 1); // Write 0x40 --> Reg 0x24
     HAL_DelayUSec(1500);                          // Wait at least 1mSec
     val = 0x40; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FREG_1, &val, 1); // Write 0x40 --> Reg 0x0D 
     val = 0x42; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_INIT_1, &val, 1); // Write 0x42 --> Reg 0x0F 
     val = 0x01; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_DMX,    &val, 1); // Write 0x01 --> Reg 0x20 
     val = 0x80; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_DMY,    &val, 1); // Write 0x80 --> Reg 0x21 
     val = 0x00; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_INIT_2, &val, 1); // Write 0x00 --> Reg 0x28 
     val = 0x00; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_INIT_3, &val, 1); // Write 0x00 --> Reg 0x1A 
     
     val = 0x00; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FREG_2, &val, 1); // Write 0x00 --> Reg 0x0E 
     // set WRAPA to include registers in range 2-9
     //val = 0x01; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FREG_2, &val, 1); // Write 0x01 --> Reg 0x0D 

     // Read scratch register
     val = 0xA5;      HAL_I2C_Write(MC3635_I2C_ADDR, 0x1B, &val, 1);
     rval[0]=0x00; rval[1]=0x00;
     HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, 0x1B, rval, 1);
     if (rval[0] != val)
     {
        // Check if Scratch register read succeeded, assert if not.
       dbg_str_int("Error reading Scratch register. Value read", rval[0]);
       dbg_str_int("Expected value", val);
     }
     val = 0x5A;      HAL_I2C_Write(MC3635_I2C_ADDR, 0x1B, &val, 1);
     rval[0]=0x00; rval[1]=0x00;
     HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, 0x1B, rval, 1);
     if (rval[0] != val)
     {
        // Check if Scratch register read succeeded, assert if not.
       dbg_str_int("Error reading Scratch register. Value read", rval[0]);
       dbg_str_int("Expected value", val);
     }

     mc3635_set_sample_resolution(MC3635_DEFAULT_SAMPLE_BIT_DEPTH);
     mc3635_set_sample_range(MC3635_DEFAULT_RANGE);
     mc3635_set_sample_rate(MC3635_DEFAULT_SAMPLE_RATE);

#if (USE_IMU_FIFO_MODE)
     mc3635_fifo_enable();
     HAL_DelayUSec(2000);
#endif

     mc3635_set_mode(MC3635_MODE_CWAKE);
     HAL_DelayUSec(2000);
     
     dbg_str_int("Sample resolution ", mc3635_get_sample_resolution());
     dbg_str_int("Sample range      ", mc3635_get_sample_range());
     dbg_str_int("Sample rate       ", mc3635_get_sample_rate());

     return;
}

void mc3635_set_mode(mc3635_modes_t mode)
{
   uint8_t modeVal;
   uint8_t init1Val = 0x42;

   // Read mode control register to get current mode
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_MODE_C, &modeVal, 1);

   modeVal &= 0xf0;
   modeVal |= mode;

   // Write to the initialization register reg[0x0F] = 0x42
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_INIT_1, &init1Val, 1); // mode control register

   // Now write to the mode control register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_MODE_C, &modeVal, 1); // mode control register
   //dbg_str_int("MC3635 Mode set to", mode);
}

/* Low-power mode sample rates, when REG[0x1C] => 0x00 */
int mc3635_sample_rates[] =
{ 0, 0, 0, 0, 0, 14, 28, 54, 105, 210, 400, 600, 600, 600, 600, 750 };

void mc3635_set_sample_rate(int sample_rate_in_hz)
{
    uint8_t srate_index ;
    uint8_t curr_mode;
    
    HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_MODE_C, &curr_mode, 1);
    curr_mode &= 0x07;
    mc3635_set_mode(MC3635_MODE_STANDBY);

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
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_RATE_1, &srate_index, 1);
   
   // Restore mode
   mc3635_set_mode(curr_mode);
   dbg_str_int("Sample rate", mc3635_sample_rates[srate_index]);
}

int mc3635_get_sample_rate(void)
{
    uint8_t srate_index = 0;
    HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_RATE_1, &srate_index, 1);
    return mc3635_sample_rates[srate_index];
}

void mc3635_set_sample_range(mc3635_range_t range)
{
    uint8_t range_c_val;

    mc3635_set_mode(MC3635_MODE_STANDBY);

   // Read current range and resolution
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_RANGE_C, &range_c_val, 1);

   range_c_val &= 0x8F;
   range_c_val |= (range << 4);
   // Now write to the range and resolution register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_RANGE_C, &range_c_val, 1);
}

int mc3635_get_sample_range(void)
{
    uint8_t range_c_val;
    HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_RANGE_C, &range_c_val, 1);
    return (mc3635_range_t)(range_c_val >> 4);
}

int mc3635_bit_depths[] = { 6, 7, 8, 10, 12, 14, 14, 14 };
int mc3635_bit_depth_index[] = 
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

void mc3635_set_sample_resolution(int bit_depth)
{
    uint8_t range_c_val;

    mc3635_set_mode(MC3635_MODE_STANDBY);
    
    // Read current range and resolution
    HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_RANGE_C, &range_c_val, 1); // mode control register
    range_c_val &= 0xF8;
 
    bit_depth = (bit_depth > MC3635_RESOLUTION_14BITS) ? MC3635_RESOLUTION_14BITS : bit_depth;
    bit_depth &= 15;
    bit_depth = mc3635_bit_depth_index[bit_depth];
    range_c_val |= bit_depth;

   // Now write to the range and resolution register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_RANGE_C, &range_c_val, 1); // mode control register
}

int mc3635_get_sample_resolution(void)
{
   uint8_t range_c_val ;
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_RANGE_C, &range_c_val, 1); // mode control register
   return mc3635_bit_depths[range_c_val & 0x07];
}

void mc3635_interrupt_enable(void)
{
    uint8_t intr_c_val;

    mc3635_set_mode(MC3635_MODE_STANDBY);

   // Read current interrupt control register content
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_INTR_C, &intr_c_val, 1);

   intr_c_val = 0x08;
   // Now write to the interrupt control register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_INTR_C, &intr_c_val, 1);
   
    mc3635_set_mode(MC3635_MODE_CWAKE);

}

void mc3635_interrupt_disable(void)
{
    uint8_t intr_c_val;

    mc3635_set_mode(MC3635_MODE_STANDBY);

   // Read current interrupt control register content
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_INTR_C, &intr_c_val, 1);

   intr_c_val = 0x00;
   // Now write to the interrupt control register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_INTR_C, &intr_c_val, 1);
}

int mc3635_read_data(xyz_t *pdata)
{
  uint8_t val[2] = {0, 0};
  HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_STATUS_1, val, 2);
  
  // chech if new data is available
  if ((val[0]&0x08) == 0)
  {
    // No new data available, return status code
    return 0;
  }

  // New data is available, Read x,y,z values
  HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_XOUT_LSB, (uint8_t *)pdata, 6);
  return 1;
}

int mc3635_read_fifo_data(xyz_t *pdata)
{
  uint8_t val[2] = {0, 0};
  HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_STATUS_1, val, 2);
  
  // chech FIFO status
  if ( ((val[0]&0x10) == 0x10) && ((val[0]&0x08) == 0) )
  {
    // FIFO is empty, no data available to read
    return 0;
  }

  // data is available, Read x,y,z values
  HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_XOUT_LSB, (uint8_t *)pdata, 6);
  return 1;
}

// num_samples should be equal to FIFO threshold 
int mc3635_read_fifo_burst(xyz_t *pdata, int num_samples)
{
  uint8_t val[2] = {0, 0};
  HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_STATUS_1, val, 2);
  
  // chech FIFO status
  if ((val[0]&0x40) == 0x00)
  {
    // available sample count less than FIFO threshold 
    return 0;
  }

  // data is available, Read x,y,z values
  HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_XOUT_LSB, (uint8_t *)pdata, 6*num_samples);
  return 1;
}

void mc3635_fifo_enable(void)
{
    uint8_t fifo_c_val;

    mc3635_set_mode(MC3635_MODE_STANDBY);

   // Read current interrupt control register content
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_FIFO_C, &fifo_c_val, 1);

   // FIFO Control Register
   // ---------------------------------------------------
   // FIFO_RESET | FIFO_EN | FIFO_MODE | FIFO_TH[4] | FIFO_TH[3] | FIFO_TH[2] | FIFO_TH[1] | FIFO_TH[0]
   // ---------------------------------------------------
 
   fifo_c_val = 0x40 | MC3635_FIFO_THRESH_VAL; // Reset the FIFO, Enable FIFO
   // Now write to the interrupt control register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FIFO_C, &fifo_c_val, 1);
}

void mc3635_fifo_enable_threshold(int threshold_count)
{
    uint8_t fifo_c_val;

    if (threshold_count > 31)
      threshold_count = 31;
    mc3635_set_mode(MC3635_MODE_STANDBY);

   // FIFO Control Register
   // ---------------------------------------------------
   // FIFO_RESET | FIFO_EN | FIFO_MODE | FIFO_TH[4] | FIFO_TH[3] | FIFO_TH[2] | FIFO_TH[1] | FIFO_TH[0]
   // ---------------------------------------------------
 
   fifo_c_val = 0x40 | (threshold_count&0x1f); // Reset the FIFO, Enable FIFO, and Set threshold
   // Now write to the interrupt control register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FIFO_C, &fifo_c_val, 1);
   
   uint8_t val = 0x22; HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FREG_2, &val, 1); // Write 0x22 --> Reg 0x0E 

}

void mc3635_fifo_reset(void)
{
    uint8_t fifo_c_val;

    mc3635_set_mode(MC3635_MODE_STANDBY);

   // Read current interrupt control register content
   HAL_I2C_Read_UsingRestart(MC3635_I2C_ADDR, MC3635_FIFO_C, &fifo_c_val, 1);

   // FIFO Control Register
   // ---------------------------------------------------
   // FIFO_RESET | FIFO_EN | FIFO_MODE | FIFO_TH[4] | FIFO_TH[3] | FIFO_TH[2] | FIFO_TH[1] | FIFO_TH[0]
   // ---------------------------------------------------
 
   fifo_c_val = 0x80 | fifo_c_val; // Reset the FIFO, Enable FIFO
   // Now write to the interrupt control register
   HAL_I2C_Write(MC3635_I2C_ADDR, MC3635_FIFO_C, &fifo_c_val, 1);
}

#if 0
__PLATFORM_INIT__ QL_Status Accel_init(void)
{
    unsigned int index;

    accel_drv_priv.drvData.id = QL_SAL_SENSOR_ID_ACCEL;
    accel_drv_priv.drvData.type = QL_SDF_SENSOR_TYPE_BASE;		/* physical sensor */

    accel_drv_priv.drvData.ops.open = M4_mc3635_Accel_Open;
    accel_drv_priv.drvData.ops.close = M4_mc3635_Accel_Close;
    accel_drv_priv.drvData.ops.read = M4_mc3635_Accel_Read;
    accel_drv_priv.drvData.ops.probe = M4_mc3635_Accel_Probe;
    accel_drv_priv.drvData.ops.ioctl = M4_mc3635_Accel_Ioctl;
    accel_drv_priv.drvData.ops.suspend = M4_mc3635_Accel_Suspend;
    accel_drv_priv.drvData.ops.resume = M4_mc3635_Accel_Resume;

    /* init framework level probe status to default, failed state - updated when probe is called */
    accel_drv_priv.drvData.probe_status = SENSOR_PROBE_STATUS_FAILED;

    /* all instances will default to the INVALID state, until probed */
    for ( index = 0 ; index < CONFIG_MAX_FFE_ACCEL_INSTANCE; index++ )
    {
      accel_drv_priv.devInstances[index].state = SENSOR_STATE_INVALID;
    }

    /* Do any platform related pin mux / I2C setting */

    /* register with SDF */
    if (QL_STATUS_OK != QL_SDF_SensorDrvRegister((struct QL_SDF_SensorDrv *) &accel_drv_priv))
    {
        QL_TRACE_SENSOR_ERROR("Failed to register Driver to SDF\n");
        return QL_STATUS_ERROR;
    }

//	AccelBtimer = xTimerCreate("Accel_LSM6DSL_Btimer",pdMS_TO_TICKS(1000),pdTRUE, (void*)0xDEA,(TimerCallbackFunction_t)vAccelBtimerCB);
//
//	if(AccelBtimer == NULL){
//		QL_TRACE_SENSOR_ERROR(" %s: Failed to Create Batch Timer\n",__func__);
//		return QL_STATUS_ERROR;
//	}

    return QL_STATUS_OK;
}
#endif
