#ifndef __LIS2DH12_WIRE_H__
#define __LIS2DH12_WIRE_H__

/* C++ Interface for mCube LIS2DH12 accelerometer
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
  LIS2DH12_MODE_LOW_POWER = 0,
  LIS2DH12_MODE_NORMAL    = 1,
  LIS2DH12_MODE_HIGH_RESOLUTION = 2,
} lis2dh12_modes_t ;

typedef enum {
  LIS2DH12_RANGE_2G  = 0,
  LIS2DH12_RANGE_4G  = 1,
  LIS2DH12_RANGE_8G  = 2,
  LIS2DH12_RANGE_16G = 3,
} lis2dh12_range_t ;

typedef enum {
  LIS2DH12_RESOLUTION_8BITS = 8,
  LIS2DH12_RESOLUTION_10BITS = 10,
  LIS2DH12_RESOLUTION_12BITS = 12
} lis2dh12_sample_bit_depths_t;

#define LIS2DH12_FIFO_SIZE        (32)
#define LIS2DH12_FIFO_RESET_BIT   (7)
#define LIS2DH12_FIFO_EN_BIT      (6)
#define LIS2DH12_FIFO_MODE_BIT    (5)
#define LIS2DH12_FIFO_THRESH_BIT  (0)

#define LIS2DH12_FIFO_THRESH_VAL  (8)   // Use 8-samples as FIFO water-mark

#if (USE_IMU_FIFO)
#define LIS2DH12_DEFAULT_SAMPLE_BIT_DEPTH   (LIS2DH12_RESOLUTION_10BITS)
#else
#define LIS2DH12_DEFAULT_SAMPLE_BIT_DEPTH   (LIS2DH12_RESOLUTION_10BITS)
#endif

#define LIS2DH12_DEFAULT_RANGE             (LIS2DH12_RANGE_2G)
#define LIS2DH12_DEFAULT_SAMPLE_RATE       (400)      // 400 Hz sampling rate
#define LIS2DH12_DEFAULT_SAMPLE_RESOLUTION (1 << (LIS2DH12_DEFAULT_SAMPLE_BIT_DEPTH-1))

class LIS2DH12
{
public:
	LIS2DH12();
	void begin(void); // initialize the sensor
	xyz_t read(void);              // get Accelerometer (X,Y,Z) values
	void set_mode(uint8_t mode);
	void set_mode(lis2dh12_modes_t mode);
    void set_sample_rate(int rate_hz);
    void set_sample_resolution(int bits);
    void set_sample_range(lis2dh12_range_t range);
    int  get_sample_rate(void);
    int  get_sample_resolution(void);
    int  get_sample_range();
    void write_reg(uint8_t reg, uint8_t *p_data, int size);
    void read_reg(uint8_t reg, uint8_t *p_data, int size);
private:
    uint8_t _deviceAddress;
};

#endif /* __LIS2DH12_WIRE_H__ */
