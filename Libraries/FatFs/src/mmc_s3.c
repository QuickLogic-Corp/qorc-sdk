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

#include "Fw_global_config.h"
#include "diskio.h"
#include "eoss3_hal_spi.h"
#include "s3x_clock_hal.h"
#include "eoss3_hal_rtc.h"
#include "ql_time.h"

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
SPI_HandleTypeDef spiFlashHandleSD;

/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
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


static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
UINT Timer1, Timer2;		/* 1000Hz decrement timer */

static
UINT CardType;



/*-----------------------------------------------------------------------*/
/* Interface Controls (Platform dependent)                               */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions.                                  */

static
void power_on(void)
{
	;					/* Turn socket power on, delay >1ms (Nothing to do) */

    /* Initialize SPI */
    spiFlashHandleSD.Init.ucFreq = SPI_BAUDRATE_20MHZ; /*SPI_BAUDRATE_2_5MHZ;*/
	spiFlashHandleSD.Init.ucSPIInf = SPI_4_WIRE_MODE;
	spiFlashHandleSD.Init.ucSSn = SPI_SLAVE_2_SELECT;
	spiFlashHandleSD.Init.ulCLKPhase = SPI_PHASE_1EDGE;
	spiFlashHandleSD.Init.ulCLKPolarity = SPI_POLARITY_LOW;
	spiFlashHandleSD.Init.ulDataSize = SPI_DATASIZE_8BIT;
	spiFlashHandleSD.Init.ulFirstBit = SPI_FIRSTBIT_MSB;
	spiFlashHandleSD.ucSPIx = SPI1_MASTER_SEL;
    //spiFlashHandleSD.fRawMode = 0;
	S3x_Clk_Enable(S3X_A1_CLK);
	S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
	if(HAL_SPI_Init(&spiFlashHandleSD) != HAL_OK)
	{
		configASSERT(0);
	}
    
    /* 80 dummy clocks */
    uint8_t acBuf[10] = {
        0xFF, 0xFF, 0xFF,0xFF, 0xFF,
        0xFF, 0xFF, 0xFF,0xFF, 0xFF};
    
    spiFlashHandleSD.Init.ucCmdType = CMD_NoResponse;
    spiFlashHandleSD.Init.ucSSn = SPI_SLAVE_1_SELECT; // Tim Switch to on-board (tacky)
    HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);
    HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);
}

static
void power_off (void)
{
	;					/* Turn socket power off (Nothing to do) */
}



/*-----------------------------------------------------------------------*/
/* SPI Transactions (Platform dependent)                                 */
/*-----------------------------------------------------------------------*/

/* Single byte SPI transaction */
static
BYTE xchg_spi (BYTE dat)
{
    uint8_t resp;
    int     ret;
    
    spiFlashHandleSD.Init.ucCmdType = READ_CMD;
    ret = HAL_SPI_TransmitReceive2(&spiFlashHandleSD, &dat, 1, &resp, 1, NULL);
    if(ret != HAL_OK)
    {
        QL_LOG_INFO_150K("Failed to get response %02x: %d\n", resp, ret);
        ret = FlashCmdFailed;
    }
    return(resp);
}

static 
BYTE send_multi_byte_spi(BYTE const *cmd, int count) {
    uint8_t ret;
    
    spiFlashHandleSD.Init.ucCmdType = CMD_NoResponse;
    ret = HAL_SPI_Transmit(&spiFlashHandleSD, (uint8_t *)cmd, count,NULL);
    if(ret != HAL_OK)
    {
        QL_LOG_INFO_150K("Failed to send command %02x: %d\n",*cmd,ret);
        ret = FlashCmdFailed;
    }
    return(ret);
}


/* Multi-byte SPI transaction (receive) */
#if 1
void rcvr_spi_multi (
	BYTE* buff,		/* Buffer to store received data */
	UINT cnt		/* Number of bytes to receive */
)
{
    int     ret;

    ret = HAL_SPI_Receive(&spiFlashHandleSD, buff, cnt, NULL);
    if(ret != HAL_OK)
    {
      //QL_LOG_INFO_150K("Failed to read data %d: %d\n", cnt, ret);
      //ret = FlashCmdFailed;
    }
}

#else //this uses byte by byte SPI transaction which is very slow
static
void rcvr_spi_multi (
	BYTE* buff,		/* Buffer to store received data */
	UINT cnt		/* Number of bytes to receive */
)
{
	do {
        *buff++ = xchg_spi(0xff);
        *buff++ = xchg_spi(0xff);
	} while (cnt -= 2);
}
#endif


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)
{
	BYTE d;

	Timer2 = 4*5000;	/* Wait for ready in timeout of 500ms */
	do {
		d = xchg_spi(0xFF);
	} while ((d != 0xFF) && --Timer2);

	return (d == 0xFF) ? 1 : 0;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect_card (void)
{
//	CS_HIGH();			/* Set CS# high */
	xchg_spi(0xFF);		/* Dummy clock (force DO hi-z for multiple slave SPI) */
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait ready                                        */
/*-----------------------------------------------------------------------*/

static
int select_card (void)	/* 1:Successful, 0:Timeout */
{
//	CS_LOW();			/* Set CS# low */
	xchg_spi(0xFF);		/* Dummy clock (force DO enabled) */

	if (wait_ready()) return 1;	/* Wait for card ready */

	deselect_card();
	return 0;	/* Timeout */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (	/* 1:OK, 0:Failed */
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be multiple of 4) */
)
{
	BYTE token;


	Timer1 = 100;
	do {							/* Wait for data packet in timeout of 100ms */
		token = xchg_spi(0xFF);
	} while ((token == 0xFF) && Timer1);

	if(token != 0xFE) return 0;		/* If not valid data token, retutn with error */

	rcvr_spi_multi(buff, btr);		/* Receive the data block into buffer */
	xchg_spi(0xFF);					/* Discard CRC */
	xchg_spi(0xFF);

	return 1;						/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
static BYTE acBuff[1+512+20];
static
int xmit_datablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data token */
)
{
	BYTE resp;
    int ret = 1;
    //BYTE acBuff[1+512+20];
    acBuff[0] = token;
    for (int i = 1; i != 1+512; i++) {
        acBuff[i] = buff[i-1];
    }
    for (int i = 1+512+1; i != 1+512+20; i++) {
        acBuff[i] = 0xFF;
    }

	if (!wait_ready()) return 0;

	if (token != 0xFD) {	/* Not StopTran token */
		send_multi_byte_spi(acBuff, 1+512+2);	/* Xmit the data block to the MMC */
		resp = xchg_spi(0xFF);		/* Receive a data response */
		if ((resp & 0x1F) != 0x05) {
            ret = 0;	/* If not accepted, return with error */
        }
    }
	return ret;
}
#endif



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

//static
//BYTE send_cmd (
//	BYTE cmd,		/* Command byte */
//	DWORD arg		/* Argument */
//)
//{
//	BYTE n, res;
//
//
//	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
//		cmd &= 0x7F;
//		res = send_cmd(CMD55, 0);
//		if (res > 1) return res;
//	}
//
//	/* Select the card and wait for ready except to stop multiple block read */
//	if (cmd != CMD12) {
//		deselect_card();
//		if (!select_card()) return 0xFF;
//	}
//
//	/* Send command packet */
//	xchg_spi(0x40 | cmd);			/* Start + Command index */
//	xchg_spi((BYTE)(arg >> 24));	/* Argument[31..24] */
//	xchg_spi((BYTE)(arg >> 16));	/* Argument[23..16] */
//	xchg_spi((BYTE)(arg >> 8));		/* Argument[15..8] */
//	xchg_spi((BYTE)arg);			/* Argument[7..0] */
//	n = 0x01;						/* Dummy CRC + Stop */
//	if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) + Stop */
//	if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) + Stop */
//	xchg_spi(n);
//
//	/* Receive command response */
//	if (cmd == CMD12) xchg_spi(0xFF);	/* Skip a stuff byte on stop to read */
//	n = 10;							/* Wait for a valid response in timeout of 10 attempts */
//	do {
//		res = xchg_spi(0xFF);
//	} while ((res & 0x80) && --n);
//
//	return res;			/* Return with the response value */
//}


static
BYTE send_cmd (
	BYTE cmd,		/* 1st byte (Start + Index) */
	DWORD arg		/* Argument (32 bits) */
)
{
	BYTE res;
    int n;
            
	if (cmd & 0x80) {	/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}
    
	/* Select the card and wait for ready except to stop multiple block read */
	if (cmd != CMD12 && cmd != CMD0) {
		deselect_card();
		if (!select_card())
        {
          printf("-ff-error5-");          
          return 0xFF;
        }
	}

    uint8_t acBuf[6];

    acBuf[0] = (0x40 | cmd);
    acBuf[1] = (arg >> 24);
    acBuf[2] = (arg >> 16);
    acBuf[3] = (arg >> 8);
    acBuf[4] = (arg >> 0);
    acBuf[5] = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) acBuf[5] = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) acBuf[5] = 0x87;			/* Valid CRC for CMD8(0x1AA) */

    spiFlashHandleSD.Init.ucCmdType = CMD_NoResponse;
    res = HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);
    if(res != HAL_OK)
    {
        QL_LOG_ERR_150K("Failed to send command %02x: %d\n", cmd, res);
        printf("-ff-error4-");
        res = FlashCmdFailed;
    }
    
    /* Receive command response */
	if (cmd == CMD12) xchg_spi(0xFF);	/* Skip a stuff byte on stop to read */
	n = 10;							/* Wait for a valid response in timeout of 10 attempts */
	do {
		res = xchg_spi(0xFF);
	} while ((res & 0x80) && --n);

    if(n <= 0)
      printf("-ff-error3-");
    
	return res;			/* Return with the response value */
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	if (pdrv != 0) return STA_NOINIT;	/* Supports only single drive */

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	BYTE n, cmd, ty, ocr[4];


	if (pdrv != 0) return STA_NOINIT;	/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Initialize memory card interface */

    /* 80 dummy clocks */
    uint8_t acBuf[10] = {
        0xFF, 0xFF, 0xFF,0xFF, 0xFF,
        0xFF, 0xFF, 0xFF,0xFF, 0xFF};
    
    spiFlashHandleSD.Init.ucCmdType = CMD_NoResponse;
    spiFlashHandleSD.Init.ucSSn = SPI_SLAVE_2_SELECT;
    HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);
    HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);
    send_cmd(CMD0, 0);
    spiFlashHandleSD.Init.ucCmdType = CMD_NoResponse;
    HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);
    HAL_SPI_Transmit(&spiFlashHandleSD, acBuf, sizeof(acBuf),NULL);

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 1000;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2? */
			for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);			/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 0x40000000));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {			/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;	/* SDv2+ */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			while (Timer1 && send_cmd(cmd, 0));		/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0) ty = 0;	/* Set read/write block length to 512 */
		}
	}
	CardType = ty;
	deselect_card();

	if (ty) {		/* Function succeded */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		FCLK_FAST();
	} else {		/* Function failed */
		power_off();	/* Deinitialize interface */
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Sector count (1..128) */
)
{
	if (pdrv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {		/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512)) {
			count = 0;
		}
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect_card();

	return (count != 0) ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,				/* Physical drive nmuber (0) */
	const BYTE *buff,		/* Pointer to the data to be written */
	DWORD sector,			/* Start sector number (LBA) */
	UINT count				/* Sector count (1..128) */
)
{
    int fOK = 0;
	if (pdrv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {		/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE)) {
			count = 0;
            fOK = 1;
		}
	}
	else {				/* Multiple block write */
        // Tim: Need to fix
#if 0 //does not work yet: f_close() has error using this method. 
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD)) count = 1;	/* STOP_TRAN token */
		}
#else
      fOK = 0;
        for (int i = 0; i != count; i++) {
            if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
                && xmit_datablock(buff, 0xFE)) {
                    buff += 512;
                    sector++;
                    fOK = 1;
                } else {
                    fOK = 0;
                    printf("-ff-error2-");
                    break;
                }
        }
#endif      
	}
	deselect_card();
    
//    if (count != 0) {
//        int rval = send_cmd(CMD13, 0);
//        printf("CMD13 = 0x%02x\n", rval);
//    }

	//return (count != 0) ? RES_ERROR : RES_OK;
    return (fOK ? RES_OK : RES_ERROR);
}
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	DWORD csz;


	if (pdrv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (cmd) {
	case CTRL_SYNC :	/* Flush write-back cache, Wait for end of internal process */
		if (select_card()) res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
		if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDv2? */
				csz = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(DWORD*)buff = csz << 10;
			} else {					/* SDv1 or MMCv3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csz = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csz << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
		if (CardType & CT_SD2) {	/* SDv2+? */
			if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
				xchg_spi(0xFF);
				if (rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) xchg_spi(0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDv1 or MMCv3 */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDv1 */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMCv3 */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
		*ptr = CardType;
		res = RES_OK;
		break;

	case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
		if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
			&& rcvr_datablock(buff, 16))
			res = RES_OK;
		break;

	case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
		if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
			&& rcvr_datablock(buff, 16))
			res = RES_OK;
		break;

	case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
		if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
			for (n = 0; n < 4; n++)
				*((BYTE*)buff+n) = xchg_spi(0xFF);
			res = RES_OK;
		}
		break;

	case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
		if ((CardType & CT_SD2) && send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
			xchg_spi(0xFF);
			if (rcvr_datablock(buff, 64)) res = RES_OK;
		}
		break;

	case CTRL_POWER_OFF :	/* Power off */
		power_off();
		Stat |= STA_NOINIT;
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	deselect_card();

	return res;
}
#endif



/*-----------------------------------------------------------------------*/
/* Device Timer Driven Procedure                                         */
/*-----------------------------------------------------------------------*/
/* This function must be called by timer interrupt in period of 1ms      */

void disk_timerproc (void)
{
	BYTE s;
	UINT n;


	n = Timer1;					/* 1000Hz decrement timer with zero stopped */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;


	/* Update socket status */

	s = Stat;
//	if (MMC_WP) {
//		s |= STA_PROTECT;
//	} else {
//		s &= ~STA_PROTECT;
//	}
//	if (MMC_CD) {
//		s &= ~STA_NODISK;
//	} else {
//		s |= (STA_NODISK | STA_NOINIT);
//	}
	Stat = s;
}


DWORD get_fattime(void)
{
    DWORD dw;
    DWORD tmp;
    time_t ltime;
    struct tm tm_time;
    
    ltime = ql_time(NULL);
	ql_localtime_r(&ltime, &tm_time);
    tm_time.tm_year += 1900;
    
    dw = 0;
    
    tmp = tm_time.tm_year - 1980;
    dw = dw | (tmp << (16+9));
    tmp = tm_time.tm_mon + 1;
    dw = dw | (tmp << (16+5));
    tmp = tm_time.tm_mday;
    dw = dw | (tmp << (16+0));
    tmp = tm_time.tm_hour;
    dw = dw | (tmp << 11);
    tmp = tm_time.tm_min;
    dw = dw | (tmp << 5);
    tmp = tm_time.tm_sec;
    tmp = tmp / 2;
    dw = dw | (tmp << 0);
    return dw;
}
     
    