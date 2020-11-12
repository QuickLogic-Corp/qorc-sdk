/** @file nau7802.c */

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

/*
  NAU7802 24-bit wheatstone bridge and load cell amplifier.

  The NAU7802 is an I2C device that converts analog signals to a 24-bit
  digital signal. This makes it possible to create your own digital scale
  either by hacking an off-the-shelf bathroom scale or by creating your
  own scale using a load cell.
  The NAU7802 is a better version of the popular HX711 load cell amplifier.
  It uses a true I2C interface so that it can share the bus with other
  I2C devices while still taking very accurate 24-bit load cell measurements
  up to 320Hz.

  For details Refer:
  https://github.com/sparkfun/SparkFun_Qwiic_Scale_NAU7802_Arduino_Library

*/

#include <stdbool.h>
#include "sensor_ssss.h"
#include "eoss3_hal_i2c.h"
#include "eoss3_hal_timer.h"
#include "micro_tick64.h"

/* NAU7802 I2C register definitions */
#define NAU7802_I2C_ADDR (0x2A)

typedef enum
{
  NAU7802_PU_CTRL = 0x00,
  NAU7802_CTRL1,
  NAU7802_CTRL2,
  NAU7802_OCAL1_B2,
  NAU7802_OCAL1_B1,
  NAU7802_OCAL1_B0,
  NAU7802_GCAL1_B3,
  NAU7802_GCAL1_B2,
  NAU7802_GCAL1_B1,
  NAU7802_GCAL1_B0,
  NAU7802_OCAL2_B2,
  NAU7802_OCAL2_B1,
  NAU7802_OCAL2_B0,
  NAU7802_GCAL2_B3,
  NAU7802_GCAL2_B2,
  NAU7802_GCAL2_B1,
  NAU7802_GCAL2_B0,
  NAU7802_I2C_CONTROL,
  NAU7802_ADCO_B2,
  NAU7802_ADCO_B1,
  NAU7802_ADCO_B0,
  NAU7802_ADC = 0x15, //Shared ADC and OTP 32:24
  NAU7802_OTP_B1,     //OTP 23:16 or 7:0?
  NAU7802_OTP_B0,     //OTP 15:8
  NAU7802_PGA = 0x1B,
  NAU7802_PGA_PWR = 0x1C,
  NAU7802_DEVICE_REV = 0x1F,
} Scale_Registers;

//Bits within the PU_CTRL register
typedef enum
{
  NAU7802_PU_CTRL_RR = 0,
  NAU7802_PU_CTRL_PUD,
  NAU7802_PU_CTRL_PUA,
  NAU7802_PU_CTRL_PUR,
  NAU7802_PU_CTRL_CS,
  NAU7802_PU_CTRL_CR,
  NAU7802_PU_CTRL_OSCS,
  NAU7802_PU_CTRL_AVDDS,
} PU_CTRL_Bits;

//Bits within the CTRL1 register
typedef enum
{
  NAU7802_CTRL1_GAIN = 2,
  NAU7802_CTRL1_VLDO = 5,
  NAU7802_CTRL1_DRDY_SEL = 6,
  NAU7802_CTRL1_CRP = 7,
} CTRL1_Bits;

//Bits within the CTRL2 register
typedef enum
{
  NAU7802_CTRL2_CALMOD = 0,
  NAU7802_CTRL2_CALS = 2,
  NAU7802_CTRL2_CAL_ERROR = 3,
  NAU7802_CTRL2_CRS = 4,
  NAU7802_CTRL2_CHS = 7,
} CTRL2_Bits;

//Bits within the PGA register
typedef enum
{
  NAU7802_PGA_CHP_DIS = 0,
  NAU7802_PGA_INV = 3,
  NAU7802_PGA_BYPASS_EN,
  NAU7802_PGA_OUT_EN,
  NAU7802_PGA_LDOMODE,
  NAU7802_PGA_RD_OTP_SEL,
} PGA_Bits;

//Bits within the PGA PWR register
typedef enum
{
  NAU7802_PGA_PWR_PGA_CURR = 0,
  NAU7802_PGA_PWR_ADC_CURR = 2,
  NAU7802_PGA_PWR_MSTR_BIAS_CURR = 4,
  NAU7802_PGA_PWR_PGA_CAP_EN = 7,
} PGA_PWR_Bits;

//Allowed Low drop out regulator voltages
typedef enum
{
  NAU7802_LDO_2V4 = 7, // 0b111,
  NAU7802_LDO_2V7 = 6, // 0b110,
  NAU7802_LDO_3V0 = 5, // 0b101,
  NAU7802_LDO_3V3 = 4, // 0b100,
  NAU7802_LDO_3V6 = 3, // 0b011,
  NAU7802_LDO_3V9 = 2, // 0b010,
  NAU7802_LDO_4V2 = 1, // 0b001,
  NAU7802_LDO_4V5 = 0  // 0b000,
} NAU7802_LDO_Values;

//Allowed gains
typedef enum
{
  NAU7802_GAIN_128 = 7, // 0b111,
  NAU7802_GAIN_64 = 6, // 0b110,
  NAU7802_GAIN_32 = 5, // 0b101,
  NAU7802_GAIN_16 = 4, // 0b100,
  NAU7802_GAIN_8 = 3, // 0b011,
  NAU7802_GAIN_4 = 2, // 0b010,
  NAU7802_GAIN_2 = 1, // 0b001,
  NAU7802_GAIN_1 = 0 // 0b000,
} NAU7802_Gain_Values;

//Allowed samples per second
typedef enum
{
  NAU7802_SPS_320 = 7, // 0b111,
  NAU7802_SPS_80 = 3, // 0b011,
  NAU7802_SPS_40 = 2, // 0b010,
  NAU7802_SPS_20 = 1, // 0b001,
  NAU7802_SPS_10 = 0, // 0b000,
} NAU7802_SPS_Values;

//Select between channel values
typedef enum
{
  NAU7802_CHANNEL_1 = 0,
  NAU7802_CHANNEL_2 = 1,
} NAU7802_Channels;

//Calibration state
typedef enum
{
  NAU7802_CAL_SUCCESS = 0,
  NAU7802_CAL_IN_PROGRESS = 1,
  NAU7802_CAL_FAILURE = 2,
} NAU7802_Cal_Status;

bool NAU7802_setRegister(uint8_t registerAddress, uint8_t value);
bool NAU7802_begin(bool reset, int sample_rate); //Check communication and initialize sensor
bool NAU7802_isConnected();                                      //Returns true if device acks at the I2C address

bool NAU7802_available();                          //Returns true if Cycle Ready bit is set (conversion is complete)
int32_t NAU7802_getReading();                      //Returns 24-bit reading. Assumes CR Cycle Ready bit (ADC conversion complete) has been checked by .available()
int32_t NAU7802_getAverage(uint8_t samplesToTake); //Return the average of a given number of readings

void NAU7802_calculateZeroOffset(uint8_t averageAmount); //Also called taring. Call this with nothing on the scale
void NAU7802_setZeroOffset(int32_t newZeroOffset);           //Sets the internal variable. Useful for users who are loading values from NVM.
int32_t NAU7802_getZeroOffset();                             //Ask library for this value. Useful for storing value into NVM.

void NAU7802_calculateCalibrationFactor(float weightOnScale, uint8_t averageAmount); //Call this with the value of the thing on the scale. Sets the calibration factor based on the weight on scale and zero offset.
void NAU7802_setCalibrationFactor(float calFactor);                                      //Pass a known calibration factor into library. Helpful if users is loading settings from NVM.
float NAU7802_getCalibrationFactor();                                                    //Ask library for this value. Useful for storing value into NVM.

float NAU7802_getWeight(bool allowNegativeWeights, uint8_t samplesToTake); //Once you've set zero offset and cal factor, you can ask the library to do the calculations for you.

bool NAU7802_setGain(uint8_t gainValue);        //Set the gain. x1, 2, 4, 8, 16, 32, 64, 128 are available
bool NAU7802_setLDO(uint8_t ldoValue);          //Set the onboard Low-Drop-Out voltage regulator to a given value. 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5V are avaialable
bool NAU7802_setSampleRate(uint8_t rate);       //Set the readings per second. 10, 20, 40, 80, and 320 samples per second is available
bool NAU7802_setChannel(uint8_t channelNumber); //Select between 1 and 2

bool NAU7802_calibrateAFE();                               //Synchronous calibration of the analog front end of the NAU7802. Returns true if CAL_ERR bit is 0 (no error)
void NAU7802_beginCalibrateAFE();                          //Begin asynchronous calibration of the analog front end of the NAU7802. Poll for completion with calAFEStatus() or wait with waitForCalibrateAFE().
bool NAU7802_waitForCalibrateAFE(uint32_t timeout_ms); //Wait for asynchronous AFE calibration to complete with optional timeout.
NAU7802_Cal_Status NAU7802_calAFEStatus();                 //Check calibration status.

bool NAU7802_reset(); //Resets all registers to Power Of Defaults

bool NAU7802_powerUp();   //Power up digital and analog sections of scale, ~2mA
bool NAU7802_powerDown(); //Puts scale into low-power 200nA mode

bool NAU7802_setIntPolarityHigh(); //Set Int pin to be high when data is ready (default)
bool NAU7802_setIntPolarityLow();  //Set Int pin to be low when data is ready

uint8_t NAU7802_getRevisionCode(); //Get the revision code of this IC. Always 0x0F.

bool NAU7802_setBit(uint8_t bitNumber, uint8_t registerAddress);   //Mask & set a given bit within a register
bool NAU7802_clearBit(uint8_t bitNumber, uint8_t registerAddress); //Mask & clear a given bit within a register
bool NAU7802_getBit(uint8_t bitNumber, uint8_t registerAddress);   //Return a given bit within a register

uint8_t NAU7802_getRegister(uint8_t registerAddress);             //Get contents of a register
bool NAU7802_setRegister(uint8_t registerAddress, uint8_t value); //Send a given value to be written to given address. Return true if successful

//y = mx+b
int32_t NAU7802_zeroOffset;      //This is b
float NAU7802_calibrationFactor; //This is m. User provides this number so that we can output y when requested

uint8_t NAU7802_SampleRate2RegValue(int sample_rate)
{
	// Set Sample rate
    uint8_t sampleRateRegValue;
	if (sample_rate >= NAU7802_SPS_320)
	{
		sampleRateRegValue = NAU7802_SPS_320;
	}
	else if (sample_rate >= NAU7802_SPS_80)
	{
		sampleRateRegValue = NAU7802_SPS_80;
	}
	else if (sample_rate >= NAU7802_SPS_40)
	{
		sampleRateRegValue = NAU7802_SPS_40;
	}
	else if (sample_rate >= NAU7802_SPS_20)
	{
		sampleRateRegValue = NAU7802_SPS_20;
	}
	else
	{
		sampleRateRegValue = NAU7802_SPS_10;
	}
    return sampleRateRegValue;
}

//Sets up the NAU7802 for basic function
//If initialize is true (or not specified), default init and calibration is performed
//If initialize is false, then it's up to the caller to initialize and calibrate
//Returns true upon completion
bool NAU7802_begin(bool initialize, int sample_rate)
{
  //Get user's options
  //_i2cPort = wirePort;

  uint8_t SampleRateRegValue = NAU7802_SampleRate2RegValue(sample_rate);

  //Check if the device ack's over I2C
  if (NAU7802_isConnected() == false)
  {
    //There are rare times when the sensor is occupied and doesn't ack. A 2nd try resolves this.
    if (NAU7802_isConnected() == false)
      return (false);
  }

  bool result = true; //Accumulate a result as we do the setup

  if (initialize)
  {
    result &= NAU7802_reset(); //Reset all registers

    result &= NAU7802_powerUp(); //Power on analog and digital sections of the scale

    result &= NAU7802_setLDO(NAU7802_LDO_3V3); //Set LDO to 3.3V

    result &= NAU7802_setGain(NAU7802_GAIN_128); //Set gain to 128

    result &= NAU7802_setSampleRate(SampleRateRegValue); //Set sampling rate close to desired value

    result &= NAU7802_setRegister(NAU7802_ADC, 0x30); //Turn off CLK_CHP. From 9.1 power on sequencing.

    result &= NAU7802_setBit(NAU7802_PGA_PWR_PGA_CAP_EN, NAU7802_PGA_PWR); //Enable 330pF decoupling cap on chan 2. From 9.14 application circuit note.

    result &= NAU7802_calibrateAFE(); //Re-cal analog front end when we change gain, sample rate, or channel
  }

  return (result);
}

//Returns true if device is present
//Tests for device ack to I2C address
bool NAU7802_isConnected()
{
#if 0
  _i2cPort->beginTransmission(_deviceAddress);
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK
#endif
  return (true);    //All good
}

//Returns true if Cycle Ready bit is set (conversion is complete)
bool NAU7802_available()
{
  return (NAU7802_getBit(NAU7802_PU_CTRL_CR, NAU7802_PU_CTRL));
}

//Calibrate analog front end of system. Returns true if CAL_ERR bit is 0 (no error)
//Takes approximately 344ms to calibrate; wait up to 1000ms.
//It is recommended that the AFE be re-calibrated any time the gain, SPS, or channel number is changed.
bool NAU7802_calibrateAFE()
{
  NAU7802_beginCalibrateAFE();
  return NAU7802_waitForCalibrateAFE(1000);
}

//Begin asynchronous calibration of the analog front end.
// Poll for completion with calAFEStatus() or wait with waitForCalibrateAFE()
void NAU7802_beginCalibrateAFE()
{
  NAU7802_setBit(NAU7802_CTRL2_CALS, NAU7802_CTRL2);
}

//Check calibration status.
NAU7802_Cal_Status NAU7802_calAFEStatus()
{
  if (NAU7802_getBit(NAU7802_CTRL2_CALS, NAU7802_CTRL2))
  {
    return NAU7802_CAL_IN_PROGRESS;
  }

  if (NAU7802_getBit(NAU7802_CTRL2_CAL_ERROR, NAU7802_CTRL2))
  {
    return NAU7802_CAL_FAILURE;
  }

  // Calibration passed
  return NAU7802_CAL_SUCCESS;
}

//Wait for asynchronous AFE calibration to complete with optional timeout.
//If timeout is not specified (or set to 0), then wait indefinitely.
//Returns true if calibration completes succsfully, otherwise returns false.
bool NAU7802_waitForCalibrateAFE(uint32_t timeout_ms)
{
  uint32_t begin = xTaskGet_uSecCount();
  NAU7802_Cal_Status cal_ready;

  while ((cal_ready = NAU7802_calAFEStatus()) == NAU7802_CAL_IN_PROGRESS)
  {
    if ((timeout_ms > 0) && ((xTaskGet_uSecCount() - begin) > 1000*timeout_ms))
    {
      break;
    }
    HAL_DelayUSec(1000); // delay(1);
  }

  if (cal_ready == NAU7802_CAL_SUCCESS)
  {
    return (true);
  }
  return (false);
}

//Set the readings per second
//10, 20, 40, 80, and 320 samples per second is available
bool NAU7802_setSampleRate(uint8_t rate)
{
  if (rate > 7 /* 0b111 */)
    rate = 7; /* 0b111; */ //Error check

  uint8_t value = NAU7802_getRegister(NAU7802_CTRL2);
  value &= 0x8F; /* 0b10001111; */ //Clear CRS bits
  value |= rate << 4;  //Mask in new CRS bits

  return (NAU7802_setRegister(NAU7802_CTRL2, value));
}

//Select between 1 and 2
bool NAU7802_setChannel(uint8_t channelNumber)
{
  if (channelNumber == NAU7802_CHANNEL_1)
    return (NAU7802_clearBit(NAU7802_CTRL2_CHS, NAU7802_CTRL2)); //Channel 1 (default)
  else
    return (NAU7802_setBit(NAU7802_CTRL2_CHS, NAU7802_CTRL2)); //Channel 2
}

//Power up digital and analog sections of scale
bool NAU7802_powerUp()
{
  NAU7802_setBit(NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
  NAU7802_setBit(NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL);

  //Wait for Power Up bit to be set - takes approximately 200us
  uint8_t counter = 0;
  while (1)
  {
    if (NAU7802_getBit(NAU7802_PU_CTRL_PUR, NAU7802_PU_CTRL) == true)
      break; //Good to go
    HAL_DelayUSec(1000); // delay(1);
    if (counter++ > 100)
      return (false); //Error
  }
  return (true);
}

//Puts scale into low-power mode
bool NAU7802_powerDown()
{
  NAU7802_clearBit(NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
  return (NAU7802_clearBit(NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL));
}

//Resets all registers to Power Of Defaults
bool NAU7802_reset()
{
  NAU7802_setBit(NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL); //Set RR
  HAL_DelayUSec(1000);
  return (NAU7802_clearBit(NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL)); //Clear RR to leave reset state
}

//Set the onboard Low-Drop-Out voltage regulator to a given value
//2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5V are available
bool NAU7802_setLDO(uint8_t ldoValue)
{
  if (ldoValue > 7 /* 0b111 */)
    ldoValue = 7; /* 0b111; */ //Error check

  //Set the value of the LDO
  uint8_t value = NAU7802_getRegister(NAU7802_CTRL1);
  value &= 0xC7; /* 0b11000111; */   //Clear LDO bits
  value |= ldoValue << 3; //Mask in new LDO bits
  NAU7802_setRegister(NAU7802_CTRL1, value);

  return (NAU7802_setBit(NAU7802_PU_CTRL_AVDDS, NAU7802_PU_CTRL)); //Enable the internal LDO
}

//Set the gain
//x1, 2, 4, 8, 16, 32, 64, 128 are avaialable
bool NAU7802_setGain(uint8_t gainValue)
{
  if (gainValue > 7 /* 0b111 */)
    gainValue = 7; /* 0b111; */ //Error check

  uint8_t value = NAU7802_getRegister(NAU7802_CTRL1);
  value &= 0xF8; /* 0b11111000; */ //Clear gain bits
  value |= gainValue;  //Mask in new bits

  return (NAU7802_setRegister(NAU7802_CTRL1, value));
}

//Get the revision code of this IC
uint8_t NAU7802_getRevisionCode()
{
  uint8_t revisionCode = NAU7802_getRegister(NAU7802_DEVICE_REV);
  return (revisionCode & 0x0F);
}

//Returns 24-bit reading
//Assumes CR Cycle Ready bit (ADC conversion complete) has been checked to be 1
int32_t NAU7802_getReading()
{
    /* Read 1 sample per channel, Fill the sample data to p_dest buffer */
    uint8_t sensorValue[4] = {0};
    HAL_I2C_Read_UsingRestart( NAU7802_I2C_ADDR, NAU7802_ADCO_B2, sensorValue, 3); // read 3-bytes

    uint32_t valueRaw = sensorValue[0];
    valueRaw <<= 8;
    valueRaw  |= sensorValue[1];
    valueRaw <<= 8;
    valueRaw  |= sensorValue[2];

    // the raw value coming from the ADC is a 24-bit number, so the sign bit now
    // resides on bit 23 (0 is LSB) of the uint32_t container. By shifting the
    // value to the left, I move the sign bit to the MSB of the uint32_t container.
    // By casting to a signed int32_t container I now have properly recovered
    // the sign of the original value
    int32_t valueShifted = (int32_t)(valueRaw << 8);

    // shift the number back right to recover its intended magnitude
    int32_t value = (valueShifted >> 8);

    return (value);
}

//Return the average of a given number of readings
//Gives up after 1000ms so don't call this function to average 8 samples setup at 1Hz output (requires 8s)
int32_t NAU7802_getAverage(uint8_t averageAmount)
{
  long total = 0;
  uint8_t samplesAquired = 0;

  unsigned long startTime = xTaskGet_uSecCount(); // millis();
  while (1)
  {
    if (NAU7802_available() == true)
    {
      total += NAU7802_getReading();
      if (++samplesAquired == averageAmount)
        break; //All done
    }
    if (xTaskGet_uSecCount() - startTime > 1000*1000)
      return (0); //Timeout - Bail with error
    HAL_DelayUSec(1000); // delay(1);
  }
  total /= averageAmount;

  return (total);
}

//Call when scale is setup, level, at running temperature, with nothing on it
void NAU7802_calculateZeroOffset(uint8_t averageAmount)
{
  NAU7802_setZeroOffset(NAU7802_getAverage(averageAmount));
}

//Sets the internal variable. Useful for users who are loading values from NVM.
void NAU7802_setZeroOffset(int32_t newZeroOffset)
{
  NAU7802_zeroOffset = newZeroOffset;
}

int32_t NAU7802_getZeroOffset()
{
  return (NAU7802_zeroOffset);
}

//Call after zeroing. Provide the float weight sitting on scale. Units do not matter.
void NAU7802_calculateCalibrationFactor(float weightOnScale, uint8_t averageAmount)
{
  int32_t onScale = NAU7802_getAverage(averageAmount);
  float newCalFactor = (onScale - NAU7802_zeroOffset) / (float)weightOnScale;
  NAU7802_setCalibrationFactor(newCalFactor);
}

//Pass a known calibration factor into library. Helpful if users is loading settings from NVM.
//If you don't know your cal factor, call setZeroOffset(), then calculateCalibrationFactor() with a known weight
void NAU7802_setCalibrationFactor(float newCalFactor)
{
  NAU7802_calibrationFactor = newCalFactor;
}

float NAU7802_getCalibrationFactor()
{
  return (NAU7802_calibrationFactor);
}

//Returns the y of y = mx + b using the current weight on scale, the cal factor, and the offset.
float NAU7802_getWeight(bool allowNegativeWeights, uint8_t samplesToTake)
{
  int32_t onScale = NAU7802_getAverage(samplesToTake);

  //Prevent the current reading from being less than zero offset
  //This happens when the scale is zero'd, unloaded, and the load cell reports a value slightly less than zero value
  //causing the weight to be negative or jump to millions of pounds
  if (allowNegativeWeights == false)
  {
    if (onScale < NAU7802_zeroOffset)
      onScale = NAU7802_zeroOffset; //Force reading to zero
  }

  float weight = (onScale - NAU7802_zeroOffset) / NAU7802_calibrationFactor;
  return (weight);
}

//Set Int pin to be high when data is ready (default)
bool NAU7802_setIntPolarityHigh()
{
  return (NAU7802_clearBit(NAU7802_CTRL1_CRP, NAU7802_CTRL1)); //0 = CRDY pin is high active (ready when 1)
}

//Set Int pin to be low when data is ready
bool NAU7802_setIntPolarityLow()
{
  return (NAU7802_setBit(NAU7802_CTRL1_CRP, NAU7802_CTRL1)); //1 = CRDY pin is low active (ready when 0)
}

//Mask & set a given bit within a register
bool NAU7802_setBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = NAU7802_getRegister(registerAddress);
  value |= (1 << bitNumber); //Set this bit
  return (NAU7802_setRegister(registerAddress, value));
}

//Mask & clear a given bit within a register
bool NAU7802_clearBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = NAU7802_getRegister(registerAddress);
  value &= ~(1 << bitNumber); //Set this bit
  return (NAU7802_setRegister(registerAddress, value));
}

//Return a given bit within a register
bool NAU7802_getBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value = NAU7802_getRegister(registerAddress);
  value &= (1 << bitNumber); //Clear all but this bit
  return (value);
}

//Get contents of a register
uint8_t NAU7802_getRegister(uint8_t registerAddress)
{
  uint8_t regVal=0;
  HAL_I2C_Read_UsingRestart( NAU7802_I2C_ADDR, registerAddress, &regVal, 1); // read 1-byte
  return regVal;
}

//Send a given value to be written to given address
//Return true if successful
bool NAU7802_setRegister(uint8_t registerAddress, uint8_t value)
{
  return HAL_I2C_Write( NAU7802_I2C_ADDR, registerAddress, &value, 1); // write 1-byte
}
