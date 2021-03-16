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

extern void mqttsn_comm_setup(void);
extern void mqttsn_comm_tx(const uint8_t *buf, int len);
extern int mqttsn_comm_rx_available(void);
extern int mqttsn_comm_rx(uint8_t *pBuf, int n);
extern int mqttsn_comm_tx_is_fifo_full(void);
extern int mqttsn_comm_tx_is_fifo_empty(void);
