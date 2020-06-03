/*==========================================================
 * Copyright 2020 QuickLogic Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *==========================================================*/

/*==========================================================
 *                                                          
 *    File   : LSM6DSL.h
 *    Purpose: Register definitions for LSM6DSL sensor
 *                                                          
 *=========================================================*/

#ifndef __LSM6DSL_H__
#define __LSM6DSL_H__


/*==============================================================================
    Constant Definition
==============================================================================*/

/**
 * @defgroup LSM6DSL_Slave_Adderess
 * @{ */
#define LSM6DSL_SLAVE_ADDR  ((short)(0x6a))

/**
 * @defgroup Macro_Group_LSM6DSL_RegAcc RegDefine(Common)
 * @{ */
#define LSM6DSL_FUNC_CFG_ACCESS         ((short)(0x01))     /*!< Enable embedded functions register */
#define LSM6DSL_SENSOR_SYNC_TIME_FRAME  ((short)(0x04))     /*!< Sensor synchronization time frame register */
#define LSM6DSL_SENSOR_SYNC_RATIO       ((short)(0x05))     /*!< Sensor synchronization resolution ratio */
#define LSM6DSL_FIFO_CTRL1              ((short)(0x06))     /*!< FIFO control register1 */
#define LSM6DSL_FIFO_CTRL2              ((short)(0x07))     /*!< FIFO control register2 */
#define LSM6DSL_FIFO_CTRL3              ((short)(0x08))     /*!< FIFO control register3 */
#define LSM6DSL_FIFO_CTRL4              ((short)(0x09))     /*!< FIFO control register4 */
#define LSM6DSL_FIFO_CTRL5              ((short)(0x0a))     /*!< FIFO control register5 */
#define LSM6DSL_DRDY_PULSE_CFG_G        ((short)(0x0b))     /*!< DataReady configuration register */
#define LSM6DSL_INT1_CTRL               ((short)(0x0d))     /*!< INT1 pad control register */
#define LSM6DSL_INT2_CTRL               ((short)(0x0e))     /*!< INT2 pad control register */
#define LSM6DSL_WHO_AM_I                ((short)(0x0f))     /*!< Who_AM_I register */
#define LSM6DSL_CTRL3_C                 ((short)(0x12))     /*!< Control register3 */
#define LSM6DSL_CTRL4_C                 ((short)(0x13))     /*!< Control register4 */
#define LSM6DSL_CTRL5_C                 ((short)(0x14))     /*!< Control register5 */
#define LSM6DSL_CTRL6_C                 ((short)(0x15))     /*!< Control register6 */
#define LSM6DSL_CTRL10_C                ((short)(0x19))     /*!< Control register10 */
#define LSM6DSL_MASTER_CONFIG           ((short)(0x1a))     /*!< Master configuration register */
#define LSM6DSL_WAKE_UP_SRC             ((short)(0x1b))     /*!< Wake up interrupt source register */
#define LSM6DSL_TAP_SRC                 ((short)(0x1c))     /*!< Tap source register */
#define LSM6DSL_D6D_SRC                 ((short)(0x1d))     /*!< Portrait, landscape, face-up and face-down source register */
#define LSM6DSL_STATUS_REG              ((short)(0x1e))     /*!< The STATUS_REG register is read by the SPI/I2C interface */
#define LSM6DSL_OUT_TEMP_L              ((short)(0x20))     /*!< Temperature data output register L */
#define LSM6DSL_OUT_TEMP_H              ((short)(0x21))     /*!< Temperature data output register H */
#define LSM6DSL_SENSORHUB1_REG          ((short)(0x2e))     /*!< First byte associated to external sensors */
#define LSM6DSL_SENSORHUB2_REG          ((short)(0x2f))     /*!< Second byte associated to external sensors */
#define LSM6DSL_SENSORHUB3_REG          ((short)(0x30))     /*!< Third byte associated to external sensors */
#define LSM6DSL_SENSORHUB4_REG          ((short)(0x31))     /*!< Fourth byte associated to external sensors */
#define LSM6DSL_SENSORHUB5_REG          ((short)(0x32))     /*!< Fifth byte associated to external sensors */
#define LSM6DSL_SENSORHUB6_REG          ((short)(0x33))     /*!< Sixth byte associated to external sensors */
#define LSM6DSL_SENSORHUB7_REG          ((short)(0x34))     /*!< Seventh byte associated to external sensors */
#define LSM6DSL_SENSORHUB8_REG          ((short)(0x35))     /*!< Eighth byte associated to external sensors */
#define LSM6DSL_SENSORHUB9_REG          ((short)(0x36))     /*!< Ninth byte associated to external sensors */
#define LSM6DSL_SENSORHUB10_REG         ((short)(0x37))     /*!< Tenth byte associated to external sensors */
#define LSM6DSL_SENSORHUB11_REG         ((short)(0x38))     /*!< Eleventh byte associated to external sensors */
#define LSM6DSL_SENSORHUB12_REG         ((short)(0x39))     /*!< Twelfth byte associated to external sensors */
#define LSM6DSL_FIFO_STATUS1            ((short)(0x3a))     /*!< FIFO status control register */
#define LSM6DSL_FIFO_STATUS2            ((short)(0x3b))     /*!< FIFO status control register */
#define LSM6DSL_FIFO_STATUS3            ((short)(0x3c))     /*!< FIFO status control register */
#define LSM6DSL_FIFO_STATUS4            ((short)(0x3d))     /*!< FIFO status control register */
#define LSM6DSL_FIFO_DATA_OUT_L         ((short)(0x3e))     /*!< FIFO data output register L */
#define LSM6DSL_FIFO_DATA_OUT_H         ((short)(0x3f))     /*!< FIFO data output register H */
#define LSM6DSL_TIMESTAMP0_REG          ((short)(0x40))     /*!< Timestamp first byte data output register */
#define LSM6DSL_TIMESTAMP1_REG          ((short)(0x41))     /*!< Timestamp second byte data output register */
#define LSM6DSL_TIMESTAMP2_REG          ((short)(0x42))     /*!< Timestamp third byte data output register */
#define LSM6DSL_STEP_TIMESTAMP_L        ((short)(0x49))     /*!< Step counter timestamp information register L */
#define LSM6DSL_STEP_TIMESTAMP_H        ((short)(0x4a))     /*!< Step counter timestamp information register H */
#define LSM6DSL_STEP_COUNTER_L          ((short)(0x4b))     /*!< Step counter output register L */
#define LSM6DSL_STEP_COUNTER_H          ((short)(0x4c))     /*!< Step counter output register H */
#define LSM6DSL_SENSORHUB13_REG         ((short)(0x4d))     /*!< Thirteenth byte associated to external sensors */
#define LSM6DSL_SENSORHUB14_REG         ((short)(0x4e))     /*!< Fourteenth byte associated to external sensors */
#define LSM6DSL_SENSORHUB15_REG         ((short)(0x4f))     /*!< Fifteenth byte associated to external sensors */
#define LSM6DSL_SENSORHUB16_REG         ((short)(0x50))     /*!< Sixteenth byte associated to external sensors */
#define LSM6DSL_SENSORHUB17_REG         ((short)(0x51))     /*!< Seventeenth byte associated to external sensors */
#define LSM6DSL_SENSORHUB18_REG         ((short)(0x52))     /*!< Eighteenth byte associated to external sensors */
#define LSM6DSL_FUNC_SRC1               ((short)(0x53))     /*!< Significant motion, tilt, step detector, hard/soft-iron and sensor hub interrupt source register */
#define LSM6DSL_FUNC_SRC2               ((short)(0x54))     /*!< Wrist tilt interrupt register */
#define LSM6DSL_TAP_CFG                 ((short)(0x58))     /*!< Timestamp, pedometer, tilt, filtering, and tap recognition functions configuration register */
#define LSM6DSL_TAP_THS_6D              ((short)(0x59))     /*!< Portrait/landscape position and tap function threshold register */
#define LSM6DSL_INT_DUR2                ((short)(0x5a))     /*!< Tap recognition function setting register */
#define LSM6DSL_WAKE_UP_THS             ((short)(0x5b))     /*!< Single and double-tap function threshold register */
#define LSM6DSL_WAKE_UP_DUR             ((short)(0x5c))     /*!< Free-fall, wakeup, timestamp and sleep mode functions duration setting register */
#define LSM6DSL_FREE_FALL               ((short)(0x5d))     /*!< Free-fall function duration setting register */
#define LSM6DSL_MD1_CFG                 ((short)(0x5e))     /*!< Functions routing on INT1 register */
#define LSM6DSL_MD2_CFG                 ((short)(0x5f))     /*!< Functions routing on INT2 register */
#define LSM6DSL_MASTER_CMD_CODE         ((short)(0x60))     /*!< Master command code used for stamping for sensor sync */
#define LSM6DSL_SYNC_SPI_ERROR_CODE     ((short)(0x61))     /*!< Error code used for sensor synchronization */
#define LSM6DSL_OUT_MAG_RAW_X_L         ((short)(0x66))     /*!< External magnetometer raw data */
#define LSM6DSL_OUT_MAG_RAW_X_H         ((short)(0x67))     /*!< External magnetometer raw data */
#define LSM6DSL_OUT_MAG_RAW_Y_L         ((short)(0x68))     /*!< External magnetometer raw data */
#define LSM6DSL_OUT_MAG_RAW_Y_H         ((short)(0x69))     /*!< External magnetometer raw data */
#define LSM6DSL_OUT_MAG_RAW_Z_L         ((short)(0x6a))     /*!< External magnetometer raw data */
#define LSM6DSL_OUT_MAG_RAW_Z_H         ((short)(0x6b))     /*!< External magnetometer raw data */
#define LSM6DSL_X_OFS_USR               ((short)(0x73))     /*!< Accelerometer X-axis user offset correction */
#define LSM6DSL_Y_OFS_USR               ((short)(0x74))     /*!< Accelerometer Y-axis user offset correction */
#define LSM6DSL_Z_OFS_USR               ((short)(0x75))     /*!< Accelerometer Z-axis user offset correction */
/** @} */


/**
 * @defgroup Macro_Group_LSM6DSL_RegAcc RegDefine(Accelerometer)
 * @{ */
#define LSM6DSL_CTRL1_XL                ((short)(0x10))     /*!< Linear acceleration sensor control register1 (ODR, full-scale selection, LPF bandwidth selection) */
#define LSM6DSL_CTRL8_XL                ((short)(0x17))     /*!< Linear acceleration sensor control register8 */
#define LSM6DSL_CTRL9_XL                ((short)(0x18))     /*!< Linear acceleration sensor control register9 */
#define LSM6DSL_OUTX_L_XL               ((short)(0x28))     /*!< Linear acceleration sensor X-axis output register L */
#define LSM6DSL_OUTX_H_XL               ((short)(0x29))     /*!< Linear acceleration sensor X-axis output register H */
#define LSM6DSL_OUTY_L_XL               ((short)(0x2a))     /*!< Linear acceleration sensor Y-axis output register L */
#define LSM6DSL_OUTY_H_XL               ((short)(0x2b))     /*!< Linear acceleration sensor Y-axis output register H */
#define LSM6DSL_OUTZ_L_XL               ((short)(0x2c))     /*!< Linear acceleration sensor Z-axis output register L */
#define LSM6DSL_OUTZ_H_XL               ((short)(0x2d))     /*!< Linear acceleration sensor Z-axis output register H */
/** @} */


/**
 * @defgroup Macro_Group_LSM6DSL_RegGyro RegDefine(Gyro)
 * @{ */

#define LSM6DSL_CTRL2_G                 ((short)(0x11))     /*!< Angular rate sensor control register2 (ODR, full-scale selection, full-scale at 125 dps) */
#define LSM6DSL_CTRL7_G                 ((short)(0x16))     /*!< Angular rate sensor control register7 */
#define LSM6DSL_OUTX_L_G                ((short)(0x22))     /*!< Angular rate sensor pitch axis (X) angular rate output register L */
#define LSM6DSL_OUTX_H_G                ((short)(0x23))     /*!< Angular rate sensor pitch axis (X) angular rate output register H */
#define LSM6DSL_OUTY_L_G                ((short)(0x24))     /*!< Angular rate sensor roll axis (Y) angular rate output register L */
#define LSM6DSL_OUTY_H_G                ((short)(0x25))     /*!< Angular rate sensor roll axis (Y) angular rate output register H */
#define LSM6DSL_OUTZ_L_G                ((short)(0x26))     /*!< Angular rate sensor yaw axis (Z) angular rate output register L */
#define LSM6DSL_OUTZ_H_G                ((short)(0x27))     /*!< Angular rate sensor yaw axis (Z) angular rate output register H */

/** @} */


/* Who_AM_I register */
#define LSM6DSL_DEVICE_ID     0x6a                          /*!< Who_AM_I register Read Value */

/* CTRL3_C  register */
#define LSM6DSL_SW_RESET      0x01                          /*!< Software Reset Bit */


/* FLAGS */
// interrupt on int 1
#define INT1_INACT_STATE 0x80
#define INT1_SINGLE_TAP 0x40
#define INT1_WAKE_UP 0x20
#define INT1_FREE_FALL 0x10
#define INT1_DOUBLE_TAP 0x08
#define INT1_6D 0x04
#define INT1_TILT 0x02
#define INT1_TIMER 0x01

// Flags for INT1_CTRL
#define INT1_STEP_DETECTOR 0x80
#define INT1_SIGN_MOT 0x40
#define INT1_FULL_FLAG 0x20
#define INT1_FIFO_OVR 0x10
#define INT1_FTH 0x08
#define INT1_BOOT 0x04
#define INT1_DRDY_G 0x02
#define INT1_DRDY_XL 0x01

// TAP configs
#define INTERRUPTS_ENABLE 0x80
#define INACT_EN0 0x40
#define INACT_EN1 0x20
#define SLOPE_FDS 0x10
#define TAP_X_EN 0x08
#define TAP_Y_EN 0x04
#define TAP_Z_EN 0x02
#define LIR 0x01

// CTRL1_XL settings
#define ODR_XL3 0x80
#define ODR_XL2 0x40
#define ODR_XL1 0x20
#define ODR_XL0 0x10
#define FS_XL1 0x08
#define FS_XL0 0x04
#define FS_2g (0x00)
#define FS_4g (FS_XL1)
#define FS_8g (FS_XL1 | FS_XL0)
#define FS_16g (FS_XL0)
#define LPF1_BW_SEL 0x02
// CTRL2_G settings
#define ODR_G3 0x80
#define ODR_G2 0x40
#define ODR_G1 0x20
#define ODR_G0 0x10
#define FS_G1 0x08
#define FS_G0 0x04
#define FS_245dps (0x00)
#define FS_500dps (FS_G0)
#define FS_1000dps (FS_G1)
#define FS_2000dps (FS_G1 | FS_G0)
#define FS_125dps 0x02

// DRDY_PULSE_CFG_G settings
#define DRDY_PULSED 0x80
#define INT2_WRIST_TILT 0x01

//  TAP_SRC
#define TAP_IA 0x40
#define SINGLE_TAP 0x20
#define DOUBLE_TAP 0x10
#define TAP_SIGN 0x08
#define X_TAP 0x04
#define Y_TAP 0x02
#define Z_TAP 0x01

// TAP_THS_6D

#define D4D_EN 0x80
#define SIXD_THS_80DEG 0x00
#define SIXD_THS_70DEG 0x20
#define SIXD_THS_60DEG 0x40
#define SIXD_THS_50DEG 0x60
// corresponds to the fullscale g settings
// at max value, the IMU must saturate
#define TAP_THS4 0x10
#define TAP_THS3 0x08
#define TAP_THS2 0x04
// lsb is 8 ODR_XL
#define TAP_THS1 0x02
#define TAP_THS0 0x01

#define TAP_THS_PERCENT(c)  ((uint8_t)((c * 0x1F))& 0x1F )

// INT_DUR2
// lsb is 32 times ODR_XL, the update rate.
#define DUR3 0x80
#define DUR2 0x40
#define DUR1 0x20
#define DUR0 0x10
// lsb is 4 * ODR_XL
#define QUIET1 0x08
#define QUIET0 0x04
// lsb is 8 ODR_XL
#define SHOCK1 0x02
#define SHOCK0 0x01

#endif /* __LSM6DSL_H__ */
