/** @file wire.h */

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

#ifndef WIRE_H
#define WIRE_H

#include <stdint.h>
#include <inttypes.h>

class TwoWire
{
private:
	static uint8_t _devAddress;
    static uint8_t recv_buf[];
    static int     recv_buf_index;
    static int     recv_buf_length;

    static uint8_t send_buf[];
    static int     send_buf_index;
    static int     send_buf_length;
public:
    TwoWire();
    void begin();
    void beginTransmission(int device_address);
    uint8_t endTransmission(void);
    uint8_t endTransmission(int stop);
    int requestFrom(int device_address, int size);
    int requestFrom(int device_address, int size, bool sendStop);

    int write(uint8_t val);
    int write(const uint8_t *p_source, int size);
    int available(void);
    uint8_t read(void);
    //void onReceive(void (*)(int));      // arduino api
    //void onRequest(void (*)(void));

};

extern TwoWire Wire;

#endif /* WIRE_H */

