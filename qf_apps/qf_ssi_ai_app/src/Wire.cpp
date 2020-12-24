/** @file wire.cpp */

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
#include <string.h>
#include "Fw_global_config.h"

#include "Wire.h"
#include "eoss3_hal_i2c.h"

#define SEND_BUFLEN (256)
#define RECV_BUFLEN (256)

uint8_t TwoWire::_devAddress;

uint8_t TwoWire::send_buf[SEND_BUFLEN];
int TwoWire::send_buf_index;
int TwoWire::send_buf_length;

uint8_t TwoWire::recv_buf[RECV_BUFLEN];
int TwoWire::recv_buf_index;
int TwoWire::recv_buf_length;

TwoWire::TwoWire()
{
}

void TwoWire::begin()
{
	send_buf_index = 0;
	recv_buf_index = 0;
	send_buf_length = 0;
	recv_buf_length = 0;
}

void TwoWire::beginTransmission(int device_address)
{
	send_buf_length = 0;
	send_buf_index = 0;
	_devAddress = device_address;
}

uint8_t TwoWire::endTransmission(void)
{
	return endTransmission(true);
}

uint8_t TwoWire::endTransmission(int stop)
{
	HAL_StatusTypeDef status;
	status = HAL_I2C_WriteRawData(_devAddress, send_buf, send_buf_length, stop);
	if (status == HAL_OK)
		return 0;  // success
	else
		return 4;  // other error
}

int TwoWire::requestFrom(int address, int size, bool sendStop)
{
	HAL_StatusTypeDef status;
	status = HAL_I2C_ReadRawData(address, recv_buf, size);
	if (status == HAL_OK)
	{
		recv_buf_length = size;
		recv_buf_index = 0;
	}
	return size;
}

int TwoWire::requestFrom(int address, int size)
{
    return requestFrom(address, size, true);
}

int TwoWire::write(const uint8_t *p_source, int size)
{
	int write_len = (send_buf_index + size) < (SEND_BUFLEN) ? size : ( (SEND_BUFLEN) - send_buf_index );
    memcpy(&send_buf[send_buf_index], (const void *)p_source, write_len);
    send_buf_index += write_len;
    send_buf_length += write_len;
    return write_len;
}

int TwoWire::write(uint8_t value)
{
	return write(&value, 1);
}

int TwoWire::available(void)
{
	return recv_buf_length;
}

uint8_t TwoWire::read(void)
{
	uint8_t value = 0xFF;
	if (recv_buf_index < RECV_BUFLEN)
	{
		value = recv_buf[recv_buf_index];
		recv_buf_index++;
		recv_buf_length--;
	}
	return value;
}
#if 0
void TwoWire::onReceive(void (*recv_handler)(int param))      // arduino api
{
	if (recv_handler)
       (*recv_handler)(param);
}

void TwoWire::onRequest(void (*request_handler)(void))
{
	if (request_handler)
       (*request_handler)();
}
#endif
TwoWire Wire;
