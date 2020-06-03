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

#ifndef QLSPI_H_INCLUDED
#define QLSPI_H_INCLUDED

#define QL_STATUS_BASE 0	// QL Status starting number

#define QL_SPI_READ_MAX_LEN (64*1024)
#define QL_SPI_WRITE_MAX_LEN 128
#if 1
#define QL_STATUS_BASE 0	// QL Status starting number
typedef enum {
				QL_STATUS_OK = QL_STATUS_BASE,		/*0*/
				QL_STATUS_ERROR,					/*1 Error*/
				QL_STATUS_ERROR_BAD_PARAMETER,		/*2 Bad parameter passed*/
				QL_STATUS_ERROR_TIMEOUT,			/*3 Timeout occured*/
				QL_STATUS_ERROR_DEVICE_BUSY			/*4 Device is currently busy*/
			 } QL_Status;
#endif
typedef QL_Status (*QLSPI_Isr_Handler)(void *data);		/*User Defined ISR to be registered with QLSPI. It is invoked by QLSPI upon interrupt from EOSS3 M4*/

struct QLSPI_Platform {
	uint32_t irq_gpio;		/*GPIO number to be configured as IRQ to receive interrupt from EOSS3 M4 */
	uint32_t cs_gpio;		/*GPIO number to be configured as Chip Select or Slave Select Pin */
};



/* QLSPI API Prototypes*/
QL_Status QLSPI_Init (struct QLSPI_Platform *spi_plat) ;		/*This API does SPI as well as platform related configurations*/
QL_Status QLSPI_Read_S3_Mem (uint32_t addr, uint8_t *data, uint32_t len);		/*This API reads from S3 Memory using SPI Bus*/
QL_Status QLSPI_Write_S3_Mem (uint32_t addr, uint8_t *data, uint32_t len);		/*This API writes to S3 Memory using SPI Bus*/
QL_Status QLSPI_Register_Isr (QLSPI_Isr_Handler handler);		/*This API registers user defined ISR handler*/
QL_Status QLSPI_Trigger_Intr (void);		/*This API triggers interrupt to S3 MCU (SPI slave device)*/

void processS3GPIOInterrupt(void);
#endif // QLSPI_H_INCLUDED

 /* qlspi_common.h */

#ifndef QLSPI_COMMON_H_INCLUDED
#define QLSPI_COMMON_H_INCLUDED

#define SPI_STATUS_OK    	0
#define SPI_STATUS_ERR     -1

#endif // QLSPI_COMMON_H_INCLUDED

/* qlspi_interrupt.h */

#ifndef QLSPI_INTERRUPTS_H_INCLUDED
#define QLSPI_INTERRUPTS_H_INCLUDED

void clear_intr_sts_s3(void);
void dis_intr_from_s3(void);
void en_intr_to_s3(void);
void en_intr_from_s3(void);

#endif // QLSPI_INTERRUPTS_H_INCLUDED

/* shub_spi.h */

#ifndef SHUB_SPI_H_INCLUDED
#define SHUB_SPI_H_INCLUDED

int32_t shub_spi_write( uint8_t cmd, uint8_t *data, uint32_t len);
int32_t shub_spi_read( uint8_t reg_addr, uint8_t *data, uint32_t len);

#endif // SHUB_SPI_H_INCLUDED

/* shub_ahb.h */

#ifndef SHUB_AHB_H_INCLUDED
#define SHUB_AHB_H_INCLUDED

#define MAX_BUF_SIZE 	(64*1024)	// AHB read max size

int32_t shub_readDMAStatus(uint8_t* buf);
int32_t read_AHB_status(uint8_t *buf);

int32_t shub_ahb_write( uint32_t addr, uint8_t *buf, uint32_t sz);
int32_t shub_ahb_read(  unsigned int addr, uint8_t *readBuf, uint32_t size);

#endif // SHUB_AHB_H_INCLUDED

/* ql_spi_internal. h */

#ifndef QL_SPI_INTERNAL_H_INCLUDED
#define QL_SPI_INTERNAL_H_INCLUDED

int32_t ql_spi_read_s3_mem(uint32_t addr, uint8_t *data, uint32_t len);
int32_t ql_spi_write_s3_mem(uint32_t addr, uint8_t *data, uint32_t len);

#endif // QL_SPI_INTERNAL_H_INCLUDED
