/** @file ads1015.c */

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

#include <stdint.h>
#include <stdbool.h>

#include "Fw_global_config.h"
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_timer.h"

#define ADS1015_I2C_ADDR   (0x48)

#define ADS1015_RESULT_REG (0x00)
#define ADS1015_CONFIG_REG (0x01)

#define ADS1015_ADDRESS_GND 0x48 //7-bit unshifted default I2C Address
#define ADS1015_ADDRESS_VDD 0x49
#define ADS1015_ADDRESS_SDA 0x4A
#define ADS1015_ADDRESS_SCL 0x4B

//Register addresses
#define ADS1015_DELAY                (1)

//Pointer Register
#define ADS1015_POINTER_CONVERT      (0x00)
#define ADS1015_POINTER_CONFIG       (0x01)
#define ADS1015_POINTER_LOWTHRESH    (0x02)
#define ADS1015_POINTER_HITHRESH     (0x03)

#define ADS1015_CONFIG_OS_NO         (0x8000)
#define ADS1015_CONFIG_OS_SINGLE     (0x8000)
#define ADS1015_CONFIG_OS_READY      (0x0000)
#define ADS1015_CONFIG_OS_NOTREADY   (0x8000)

#define ADS1015_CONFIG_MODE_CONT     (0x0000)
#define ADS1015_CONFIG_MODE_SINGLE   (0x0100)

#define ADS1015_CONFIG_MUX_SINGLE_0    (0x4000)
#define ADS1015_CONFIG_MUX_SINGLE_1    (0x5000)
#define ADS1015_CONFIG_MUX_SINGLE_2    (0x6000)
#define ADS1015_CONFIG_MUX_SINGLE_3    (0x7000)
#define ADS1015_CONFIG_MUX_DIFF_P0_N1  (0x0000)
#define ADS1015_CONFIG_MUX_DIFF_P0_N3  (0x1000)
#define ADS1015_CONFIG_MUX_DIFF_P1_N3  (0x2000)
#define ADS1015_CONFIG_MUX_DIFF_P2_N3  (0x3000)


#define ADS1015_CONFIG_RATE_128HZ    (0x0000)
#define ADS1015_CONFIG_RATE_250HZ    (0x0020)
#define ADS1015_CONFIG_RATE_490HZ    (0x0040)
#define ADS1015_CONFIG_RATE_920HZ    (0x0060)
#define ADS1015_CONFIG_RATE_1600HZ   (0x0080)
#define ADS1015_CONFIG_RATE_2400HZ   (0x00A0)
#define ADS1015_CONFIG_RATE_3300HZ   (0x00C0)

#define ADS1015_CONFIG_PGA_MASK      (0X0E00)
#define ADS1015_CONFIG_PGA_TWOTHIRDS       (0X0000)  // +/- 6.144v
#define ADS1015_CONFIG_PGA_1         (0X0200)  // +/- 4.096v
#define ADS1015_CONFIG_PGA_2         (0X0400)  // +/- 2.048v
#define ADS1015_CONFIG_PGA_4         (0X0600)  // +/- 1.024v
#define ADS1015_CONFIG_PGA_8         (0X0800)  // +/- 0.512v
#define ADS1015_CONFIG_PGA_16        (0X0A00)  // +/- 0.256v

#define ADS1015_CONFIG_CMODE_TRAD   (0x0000)  // Traditional comparator with hysteresis (default)
#define ADS1015_CONFIG_CMODE_WINDOW (0x0010)  // Window comparator
#define ADS1015_CONFIG_CPOL_ACTVLOW (0x0000)  // ALERT/RDY pin is low when active (default)
#define ADS1015_CONFIG_CPOL_ACTVHI  (0x0008)  // ALERT/RDY pin is high when active
#define ADS1015_CONFIG_CLAT_NONLAT  (0x0000)  // Non-latching comparator (default)
#define ADS1015_CONFIG_CLAT_LATCH   (0x0004)  // Latching comparator    
#define ADS1015_CONFIG_CQUE_1CONV   (0x0000)  // Assert ALERT/RDY after one conversions
#define ADS1015_CONFIG_CQUE_2CONV   (0x0001)  // Assert ALERT/RDY after two conversions
#define ADS1015_CONFIG_CQUE_4CONV   (0x0002)  // Assert ALERT/RDY after four conversions
#define ADS1015_CONFIG_CQUE_NONE    (0x0003)  // Disable the comparator and put ALERT/RDY in high state (default)

typedef bool boolean;
typedef uint8_t byte;

boolean ADS1015_begin(uint8_t i2caddr);

boolean ADS1015_isConnected(); //Checks if sensor ack's the I2C request

uint16_t ADS1015_getSingleEnded(uint8_t channel);
int16_t ADS1015_getDifferential(uint16_t CONFIG_MUX_DIFF);
uint16_t ADS1015_getAnalogData(uint8_t channel); // antiquated function; here for backward compatibility
float ADS1015_getScaledAnalogData(uint8_t channel);
void ADS1015_calibrate();
uint16_t ADS1015_getCalibration(uint8_t channel, bool hiLo);
void ADS1015_setCalibration(uint8_t channel, bool hiLo, uint16_t value);
void ADS1015_resetCalibration();

float ADS1015_mapf(float val, float in_min, float in_max, float out_min, float out_max);

boolean ADS1015_available(); //True if OS bit is set

void ADS1015_setMode(uint16_t mode); //Set mode of the sensor. Mode 0 is continuous read mode
uint16_t ADS1015_getMode();

void ADS1015_setGain(uint16_t gain);
uint16_t ADS1015_getGain();

void ADS1015_setSampleRate(uint16_t sampleRate);
uint16_t ADS1015_getSampleRate();

float ADS1015_getMultiplier();

uint16_t ADS1015_readRegister(uint8_t location); //Basic read of a register
void ADS1015_writeRegister(uint8_t location, uint16_t val); //Writes to a location
uint16_t ADS1015_readRegister16(byte location); //Reads a 16bit value

void ADS1015_setComparatorSingleEnded(uint8_t channel, int16_t threshold);
int16_t ADS1015_getLastConversionResults();
uint16_t ADS1015_mode = ADS1015_CONFIG_MODE_CONT;
uint16_t ADS1015_gain = ADS1015_CONFIG_PGA_2;
uint16_t ADS1015_sampleRate = ADS1015_CONFIG_RATE_1600HZ;
float ADS1015_multiplierToVolts = 1.0F; // at a default gain of 2, the multiplier is 1, also updated in setGain()
void ADS1015_updateMultiplierToVolts();

uint8_t ADS1015_i2caddr;

//Array is structured as calibrationValues[finger][lo/hi]
uint16_t ADS1015_calibrationValues[2][2] = {{0, 0}, {0, 0}};

boolean ADS1015_printDebug = false; //Flag to print the serial commands we are sending to the Serial port for debug

static void delay(int milli_secs)
{
  HAL_DelayUSec(milli_secs * 1000);
  return;
}

void ADS1015_reset(void)
{
	// Configure ADS1015
	uint8_t ads1015_config[] = { 0xF4, 0x83 };
	uint8_t val;
	val = 0x01;       HAL_I2C_Write(ADS1015_I2C_ADDR, ADS1015_CONFIG_REG, ads1015_config, 2);
	return;
}

int16_t ADS1015_readValue(void)
{
	uint8_t sensorValueBytes[2];
	int16_t sensorValue;
	HAL_I2C_Read_UsingRestart(ADS1015_I2C_ADDR, ADS1015_RESULT_REG, sensorValueBytes, 2);
	
	sensorValue  = sensorValueBytes[0] << 8;
	sensorValue |= sensorValueBytes[1]     ;
	return sensorValue;
}

/*
  This is a library written for the ADS1015 ADC->I2C.
  Written by Andy England @ SparkFun Electronics, October 17th, 2017
  The sensor uses I2C to communicate, as well as a single (optional)
  interrupt line that is not currently supported in this driver.
  https://github.com/sparkfun/SparkFun_ADS1015_Arduino_Library
  Do you like this library? Help support SparkFun. Buy a board!
  Development environment specifics:
  Arduino IDE 1.8.1
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include "SparkFun_ADS1015_Arduino_Library.h"

//Sets up the sensor for constant read
//Returns false if sensor does not respond

boolean ADS1015_begin(uint8_t i2caddr)
{
  if (ADS1015_isConnected() == false) return (false); //Check for sensor presence

  return (true); //We're all setup!
}

//Returns true if I2C device ack's
boolean ADS1015_isConnected()
{
  return (true);
}

//Returns the decimal value of sensor channel single-ended input
uint16_t ADS1015_getSingleEnded(uint8_t channel)
{
	if (channel > 3) {
		return 0;
	}

	uint16_t config = ADS1015_CONFIG_OS_SINGLE   |
					  ADS1015_mode |
					  ADS1015_sampleRate;

	config |= ADS1015_gain;

	switch (channel)
    {
    case (0):
        config |= ADS1015_CONFIG_MUX_SINGLE_0;
        break;
    case (1):
        config |= ADS1015_CONFIG_MUX_SINGLE_1;
        break;
    case (2):
        config |= ADS1015_CONFIG_MUX_SINGLE_2;
        break;
    case (3):
        config |= ADS1015_CONFIG_MUX_SINGLE_3;
        break;
    }

	ADS1015_writeRegister(ADS1015_POINTER_CONFIG, config);
	delay(ADS1015_DELAY);

    return ADS1015_readRegister(ADS1015_POINTER_CONVERT) >> 4;
}

//Returns the *signed* decimal value of sensor differential input
//Note, there are 4 possible differential pin setups:
//ADS1015_CONFIG_MUX_DIFF_P0_N1
//ADS1015_CONFIG_MUX_DIFF_P0_N3
//ADS1015_CONFIG_MUX_DIFF_P1_N3
//ADS1015_CONFIG_MUX_DIFF_P2_N3
int16_t ADS1015_getDifferential(uint16_t CONFIG_MUX_DIFF)
{
	// check for valid argument input
	if (
	(CONFIG_MUX_DIFF == ADS1015_CONFIG_MUX_DIFF_P0_N1) ||
	(CONFIG_MUX_DIFF == ADS1015_CONFIG_MUX_DIFF_P0_N3) ||
	(CONFIG_MUX_DIFF == ADS1015_CONFIG_MUX_DIFF_P1_N3) ||
	(CONFIG_MUX_DIFF == ADS1015_CONFIG_MUX_DIFF_P2_N3)
	)
	{
		// valid argument; do nothing and then carry on below
	}
	else
	{
		return 0; // received invalid argument
	}

	uint16_t config = ADS1015_CONFIG_OS_SINGLE   |
					  ADS1015_mode |
					  ADS1015_sampleRate;

	config |= ADS1015_gain;

    config |= CONFIG_MUX_DIFF; // default is ADS1015_CONFIG_MUX_DIFF_P0_N1

	ADS1015_writeRegister(ADS1015_POINTER_CONFIG, config);
	delay(ADS1015_DELAY);

    uint16_t result = ADS1015_readRegister(ADS1015_POINTER_CONVERT) >> 4;

    // making sure we keep the sign bit intact
    if (result > 0x07FF)
    {
      // negative number - extend the sign to 16th bit
      result |= 0xF000;
    }
    return (int16_t)result; // cast as a *signed* 16 bit int.
}

// antiquated function from older library, here for backwards compatibility
uint16_t ADS1015_getAnalogData(uint8_t channel)
{
	return ADS1015_getSingleEnded(channel);
}

//Returns a value between 0 and 1 based on how bent the finger is. This function will not work with an uncalibrated sensor
float ADS1015_getScaledAnalogData (uint8_t channel)
{
	float data = ADS1015_mapf(ADS1015_getAnalogData(channel), ADS1015_calibrationValues[channel][0], ADS1015_calibrationValues[channel][1], 0, 1);
	if (data > 1)
	{
		return 1;
	}
	else if (data < 0)
	{
		return 0;
	}
	else
	{
		return data;
	}
}

void ADS1015_calibrate ()
{
	for (int finger = 0; finger < 2; finger++)
	{
		uint16_t value = ADS1015_getAnalogData(finger);
		if ((value > ADS1015_calibrationValues[finger][1] || ADS1015_calibrationValues[finger][1] == 0) && value < 1085)
		{
			ADS1015_calibrationValues[finger][1] = value;
		}
		else if (value < ADS1015_calibrationValues[finger][0] || ADS1015_calibrationValues[finger][0] == 0)
		{
			ADS1015_calibrationValues[finger][0] = value;
		}
	}
}

uint16_t ADS1015_getCalibration(uint8_t channel, bool hiLo)
{
	return ADS1015_calibrationValues[channel][hiLo];
}

void ADS1015_setCalibration(uint8_t channel, bool hiLo, uint16_t value)
{
	ADS1015_calibrationValues[channel][hiLo] = value;
}

void ADS1015_resetCalibration()
{
	for (int channel = 0; channel < 2; channel++)
	{
		for (int hiLo = 0; hiLo < 2; hiLo++)
		{
			ADS1015_calibrationValues[channel][hiLo] = 0;
		}
	}
}

float ADS1015_mapf(float val, float in_min, float in_max, float out_min, float out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//Set the mode. Continuous mode 0 is favored
void ADS1015_setMode(uint16_t mode)
{
  ADS1015_mode = mode;
}

//getMode will return 0 for continuous and 1 for single shot
uint16_t ADS1015_getMode ()
{
  return ADS1015_mode;
}

void ADS1015_setGain (uint16_t gain)
{
	ADS1015_gain = gain;
	ADS1015_updateMultiplierToVolts(); // each new gain setting changes how we convert to volts
}

//Will return a different hex value for each gain
//0x0E00: +/- 0.256V
//0X0000: +/- 6.144V
//0X0200: +/- 4.096V
//0X0400: +/- 2.048V
//0X0600: +/- 1.024V
//0X0800: +/- 0.512V
//0X0A00: +/- 0.256V
uint16_t ADS1015_getGain ()
{
	return ADS1015_gain;
}

void ADS1015_updateMultiplierToVolts()
{
	switch (ADS1015_gain)
    {
    case (ADS1015_CONFIG_PGA_TWOTHIRDS):
        ADS1015_multiplierToVolts = 3.0F;
        break;
    case (ADS1015_CONFIG_PGA_1):
        ADS1015_multiplierToVolts = 2.0F;
        break;
    case (ADS1015_CONFIG_PGA_2):
        ADS1015_multiplierToVolts = 1.0F;
        break;
    case (ADS1015_CONFIG_PGA_4):
        ADS1015_multiplierToVolts = 0.5F;
        break;
    case (ADS1015_CONFIG_PGA_8):
        ADS1015_multiplierToVolts = 0.25F;
        break;
    case (ADS1015_CONFIG_PGA_16):
        ADS1015_multiplierToVolts = 0.125F;
        break;
	default:
		ADS1015_multiplierToVolts = 1.0F;
    }
}

float ADS1015_getMultiplier()
{
  return ADS1015_multiplierToVolts;
}

void ADS1015_setSampleRate (uint16_t sampleRate)
{
	ADS1015_sampleRate = sampleRate;
}

//Will return a different hex value for each sample rate
//0x0000: 128 Hz
//0X0020: 250 Hz
//0X0040: 490 Hz
//0X0060: 920 Hz
//0X0080: 1600 Hz
//0X00A0: 2400 Hz
//0X00C0: 3300 Hz
uint16_t ADS1015_getSampleRate ()
{
	return ADS1015_sampleRate;
}

//Checks to see if DRDY flag is set in the status register
boolean ADS1015_available()
{
  uint16_t value = ADS1015_readRegister(ADS1015_POINTER_CONFIG);
  return (value & (1 << 0)); //Bit 0 is DRDY
}

//Reads from a give location
uint16_t ADS1015_readRegister(uint8_t location)
{
  uint8_t regValueBytes[2] = { 0, 0 };
  uint16_t regValue;
  HAL_I2C_Read_UsingRestart(ADS1015_I2C_ADDR, ADS1015_POINTER_CONVERT, regValueBytes, 2);
  regValue  = regValueBytes[0] << 8;
  regValue |= regValueBytes[1];
  return ( regValue );
#if 0
  _i2cPort->beginTransmission(_i2caddr);
  _i2cPort->write(ADS1015_POINTER_CONVERT);
  _i2cPort->endTransmission();
  _i2cPort->requestFrom((int)_i2caddr, 2); //Ask for one byte
  return (_i2cPort->read() << 8 | _i2cPort->read());
#endif
}

//Write a value to a spot
void ADS1015_writeRegister(uint8_t location, uint16_t val)
{
  uint8_t regValueBytes[2] = { 0, 0 };
  regValueBytes[0] = (val >> 8) & 0xff;
  regValueBytes[1] = (val     ) & 0xff;
  HAL_I2C_Write(ADS1015_I2C_ADDR, location, regValueBytes, 2);
#if 0
  _i2cPort->beginTransmission(_i2caddr);
  _i2cPort->write(location);
  _i2cPort->write((uint8_t)(val >> 8));
  _i2cPort->write((uint8_t)(val & 0xFF));
  _i2cPort->endTransmission();
#endif
}

//Reads a two byte value from a consecutive registers
uint16_t ADS1015_readRegister16(byte location)
{
  uint8_t  regValueBytes[2] = { 0, 0 };
  uint16_t regValue;
  HAL_I2C_Read_UsingRestart(ADS1015_I2C_ADDR, ADS1015_POINTER_CONVERT, regValueBytes, 2);
  regValue  = regValueBytes[0] << 0;
  regValue |= regValueBytes[1] << 8;
  return ( regValue );
#if 0
  uint8_t  result;
  _i2cPort->beginTransmission(_i2caddr);
  _i2cPort->write(ADS1015_POINTER_CONVERT);
  result = _i2cPort->endTransmission();
  _i2cPort->requestFrom((int)_i2caddr, 2);

  uint16_t data = _i2cPort->read();
  data |= (_i2cPort->read() << 8);
  return (data);
#endif
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.
            This will also set the ADC in continuous conversion mode.

			Note, this function was adapted from the Adafruit Industries
			located here:
			https://github.com/adafruit/Adafruit_ADS1X15
*/
/**************************************************************************/
void ADS1015_setComparatorSingleEnded(uint8_t channel, int16_t threshold)
{
	if (channel > 3) {
		return;
	}

	uint16_t config =
		    ADS1015_CONFIG_MODE_CONT |
		    ADS1015_sampleRate |
		    ADS1015_CONFIG_CQUE_1CONV   | 	// Comparator enabled and asserts on 1 match
            ADS1015_CONFIG_CLAT_LATCH   | 	// Latching mode
            ADS1015_CONFIG_CPOL_ACTVLOW | 	// Alert/Rdy active low   (default val)
            ADS1015_CONFIG_CMODE_TRAD; 		// Traditional comparator (default val)

	config |= ADS1015_gain;

	switch (channel)
    {
    case (0):
        config |= ADS1015_CONFIG_MUX_SINGLE_0;
        break;
    case (1):
        config |= ADS1015_CONFIG_MUX_SINGLE_1;
        break;
    case (2):
        config |= ADS1015_CONFIG_MUX_SINGLE_2;
        break;
    case (3):
        config |= ADS1015_CONFIG_MUX_SINGLE_3;
        break;
    }

	// Set the high threshold register
	// Shift 12-bit results left 4 bits for the ADS1015
	ADS1015_writeRegister(ADS1015_POINTER_HITHRESH, threshold << 4);

	// Write config register to the ADC
	ADS1015_writeRegister(ADS1015_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.

			Note, this function was adapted from the Adafruit Industries
			located here:
			https://github.com/adafruit/Adafruit_ADS1X15
*/
/**************************************************************************/
int16_t ADS1015_getLastConversionResults()
{
	// Wait for the conversion to complete
	delay(ADS1015_DELAY);

	// Read the conversion results
	uint16_t result = ADS1015_readRegister(ADS1015_POINTER_CONVERT) >> 4;

	// Shift 12-bit results right 4 bits for the ADS1015,
	// making sure we keep the sign bit intact
	if (result > 0x07FF)
	{
	  // negative number - extend the sign to 16th bit
	  result |= 0xF000;
	}
	return (int16_t)result;
}

