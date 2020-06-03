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
 *    File   : spi_flash.c
 *    Purpose: This file contains driver source code for Macronix MX25U3235FM2I SPI flash.
 *                                                          
 *=========================================================*/

#ifndef MEDIA_DRV_DEFINES_H
#define MEDIA_DRV_DEFINES_H

#define HUNDRED_64_BIT				      100ULL
#define BYTES_PER_MB				     (1024ull * 1024ull)
#define SECTORS_PER_MB				     (BYTES_PER_MB / SPIFLASH_DISK_SECTOR_SIZE)

typedef struct
{
	uint32_t total_size;
	uint32_t free_size;
}SPI_DISK_PRIVATE_DATA;

#endif  // MEDIA_DRV_SPI_SD_H
