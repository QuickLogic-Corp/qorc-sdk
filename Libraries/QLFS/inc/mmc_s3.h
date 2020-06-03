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

/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2+ (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#if !defined( _EnD_Of_Fw_global_config_h )
#error "Include Fw_global_config.h first"
#endif

//#include "integer.h"
#include "eoss3_hal_spi.h"
#include "s3x_clock_hal.h"

#include "media_drv_spi_sd.h"

#define _USE_WRITE	1	/* 1: Enable disk_write function */

/* Socket controls  (Platform dependent) */
#define CS_LOW()	_LATB15 = 0	/* MMC CS = L */
#define CS_HIGH()	_LATB15 = 1	/* MMC CS = H */
#define MMC_CD		(!_RB11)	/* Card detected   (yes:true, no:false, default:true) */
#define MMC_WP		(_RB10)		/* Write protected (yes:true, no:false, default:false) */

/* SPI bit rate controls */
#define	FCLK_SLOW()			/* Set slow clock for card initialization (100k-400k) */
#define	FCLK_FAST()			/* Set fast clock for generic read/write */



/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Handle for SPI transactions */
extern SPI_HandleTypeDef spiFlashHandleSD;

/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define CMD3   (3)			/* SD_CMD_SET_REL_ADDR */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define CMD13  (13)         /* Status */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */

/**
  * @brief Supported SD Memory Cards
  */
#define STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

/* @TBD starts - The definitions below are taken from diskio.h, need to replced by definitions above*/

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at _FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at _USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at _USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

/* MMC/SDC specific command (Not used by FatFs) */
#define CTRL_POWER_OFF		7	/* Put the device off state */
#define MMC_GET_TYPE		50	/* Get card type */
#define MMC_GET_CSD			51	/* Read CSD */
#define MMC_GET_CID			52	/* Read CID */
#define MMC_GET_OCR			53	/* Read OCR */
#define MMC_GET_SDSTAT		54	/* Read SD status */

#define  _USE_IOCTL         1
/* TBD ends */

/**
  * @}
  */

/* Status of Disk Functions */
typedef UINT8_t	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*-----------------------------------------------------------------------*/
/* Interface Controls (Platform dependent)                               */
/*-----------------------------------------------------------------------*/
/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/
void power_on(void);

void power_off(void);

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	UINT8_t pdrv		/* Physical drive nmuber (0) */
);


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	UINT8_t pdrv		/* Physical drive nmuber (0) */
);



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	UINT8_t pdrv,		/* Physical drive nmuber (0) */
	UINT8_t *buff,		/* Pointer to the data buffer to store read data */
	UINT32_t sector,	/* Start sector number (LBA) */
	UINT32_t count		/* Sector count (1..128) */
);



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	UINT8_t pdrv,				/* Physical drive nmuber (0) */
	const UINT8_t *buff,		/* Pointer to the data to be written */
	UINT32_t sector,			/* Start sector number (LBA) */
	UINT32_t count				/* Sector count (1..128) */
);
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	UINT8_t pdrv,		/* Physical drive nmuber (0) */
	UINT8_t cmd,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
);
#endif

DSTATUS SDdisk_initialize (
	UINT8_t pdrv,		/* Physical drive nmuber (0) */
        SD_HandleTypeDef *hsd   /* SD handle */
);




