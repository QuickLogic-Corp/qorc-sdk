#ifndef __MC3635_WIRE_H__
#define __MC3635_WIRE_H__

/* C++ Interface for mCube MC3635 accelerometer
 * Uses Wire interface for communicating with I2C device
 */
#include <stdint.h>
#include "Wire.h"

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} xyz_t ;

typedef enum {
  MC3635_MODE_SLEEP   = 0,
  MC3635_MODE_STANDBY = 1,
  MC3635_MODE_SNIFF   = 2,
  /* 3 and 4 are reserved */
  MC3635_MODE_CWAKE   = 5,
  MC3635_MODE_SWAKE   = 6,
  MC3635_MODE_TRIG    = 7
} mc3635_modes_t ;

typedef enum {
  MC3635_RANGE_2G  = 0,
  MC3635_RANGE_4G  = 1,
  MC3635_RANGE_8G  = 2,
  MC3635_RANGE_16G = 3,
  MC3635_RANGE_12G = 4 
} mc3635_range_t ;

typedef enum {
  MC3635_RESOLUTION_6BITS = 6,
  MC3635_RESOLUTION_7BITS = 7,
  MC3635_RESOLUTION_8BITS = 8,
  MC3635_RESOLUTION_10BITS = 10,
  MC3635_RESOLUTION_12BITS = 12,
  MC3635_RESOLUTION_14BITS = 14,
} mc3635_sample_bit_depths_t;

#define MC3635_FIFO_SIZE        (32)
#define MC3635_FIFO_RESET_BIT   (7)
#define MC3635_FIFO_EN_BIT      (6)
#define MC3635_FIFO_MODE_BIT    (5)
#define MC3635_FIFO_THRESH_BIT  (0)

#define MC3635_FIFO_THRESH_VAL  (8)   // Use 8-samples as FIFO water-mark

#if (USE_IMU_FIFO)
#define MC3635_DEFAULT_SAMPLE_BIT_DEPTH   (MC3635_RESOLUTION_12BITS)
#else
#define MC3635_DEFAULT_SAMPLE_BIT_DEPTH   (MC3635_RESOLUTION_14BITS)
#endif

#define MC3635_DEFAULT_RANGE             (MC3635_RANGE_2G)
#define MC3635_DEFAULT_SAMPLE_RATE       (400)      // 400 Hz sampling rate
#define MC3635_DEFAULT_SAMPLE_RESOLUTION (1 << (MC3635_DEFAULT_SAMPLE_BIT_DEPTH-1))

class MC3635
{
public:
	MC3635();
	void begin(void); // initialize the sensor
	xyz_t read(void);              // get Accelerometer (X,Y,Z) values
	void set_mode(uint8_t mode);
	void set_mode(mc3635_modes_t mode);
    void set_sample_rate(int rate_hz);
    void set_sample_resolution(int bits);
    void set_sample_range(mc3635_range_t range);
    int  get_sample_rate(void);
    int  get_sample_resolution(void);
    int  get_sample_range();
    void write_reg(uint8_t reg, uint8_t *p_data, int size);
    void read_reg(uint8_t reg, uint8_t *p_data, int size);
private:
    uint8_t _deviceAddress;
};

#endif /* __MC3635_WIRE_H__ */
