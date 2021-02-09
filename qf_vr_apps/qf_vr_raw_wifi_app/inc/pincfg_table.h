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

#ifndef PINCFG_TABLE_H
#define PINCFG_TABLE_H

#include "Fw_global_config.h"
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_pad_config.h"

extern PadConfig pincfg_table[] ;
extern GPIOCfgTypeDef  gpiocfg_table[] ;
extern int sizeof_pincfg_table ;
extern int sizeof_gpiocfg_table;

#endif /* PINCFG_TABLE_H */
