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
 *    File   : eoss3_hal_sdma.c
 *    Purpose:
 *
 *=========================================================*/
#include "Fw_global_config.h"

#include <stdio.h>
#include <stdint.h>
#include "eoss3_dev.h"
#include "eoss3_hal_sdma.h"


#include <FreeRTOS.h>
#include <semphr.h>
#define PHASE3_DESIGN

#define sdma_status(P)	((P)->DMA_STATUS & SDMA_DMA_STATUS_STATE_MASK)
#define sdma_isbusy(P)	(sdma_status(P) != SDMA_DMA_STATUS_IDLE_STATE)

static void resetSdma(void);
static int setSdmaCh(int ch);
static int startSdma(int chan);
static int setSdmaSram(int ch, uint32_t* srcEndAddr, uint32_t * destEndAddr, int xferSzMinus1, SDMA_XFER_FORMAT xferFormat);

#define FB_SDMA_BASE_CHAN SDMA_SRAM_CH12

/* Sdma channels from 12 to 15 cannot be triggered (enabling dreq to sdma) from sdma bridge &
 * these channels cannot inform M4 directly about dma transfer completion.
 * Triggering of above mentioned channels (12-15) has to be done by setting certain bit/bits
 * in corresponding module (channel 12 - lcdc & channel 13 - hrm) in fpga. Also SDMA transfer
 * completion is sent to fpga which is inturn sent to M4.
 * So to make sure that no h/w registers other than in SDMA module is touched here, the callbacks
 * triggerSdma & txrCompNotifier has been provided. These two callbacks has to be registered for
 * channel 12-15 for proper functioning of sdma channels. For other channels default functions
 * SdmaTxrCompNotifier & sdmaTrigger is used.
 */
typedef struct {
	void (*triggerSdma)(int chan);
	void (*txrCompNotifier)(int chan);
} SDMA_CH_Info_t;

static SDMA_CH_Info_t sdmaChInfo[SDMA_SRAM_CH15 + 1];

HAL_StatusTypeDef HAL_SDMA_Reg_CB(int channel_number, void (*triggerSdma)(int chan),  void (*comp_notifier)(int chan))
{
	if ((channel_number < SDMA_SRAM_CH0) || (channel_number > SDMA_SRAM_CH15))
	{
		printf("[SDMA] : Error : Invalid channel number passed to %s\n", __FUNCTION__);
		return HAL_ERROR;
	}
	sdmaChInfo[channel_number].txrCompNotifier = comp_notifier;
	sdmaChInfo[channel_number].triggerSdma = triggerSdma;
	return HAL_OK;
}

HAL_StatusTypeDef HAL_SDMA_Reg_TxrCompNotifier(int channel_number, void (*comp_notifier)(int chan))
{
	return HAL_SDMA_Reg_CB(channel_number, NULL,  comp_notifier);
}

HAL_StatusTypeDef HAL_SDMA_Reg_TriggerSdma(int channel_number, void (*triggerSdma)(int chan))
{
	return HAL_SDMA_Reg_CB(channel_number, triggerSdma, NULL);
}

static void sdmaTrigger(int chan)
{
	if ( chan < SDMA_SRAM_CH12)
		SDMA_BRIDGE->DMA_REQ = 1 << (chan - 1);
}

static void SdmaTxrCompNotifier(int chan)
{
	/* FIXME : Need IRQ based blocking call for non-fb based sdma channels */
	while (sdma_isbusy(SDMA)) ;
}

static void setupSdmaIrq(void)
{
	int chno;
	for (chno = 0 ; chno < SDMA_SRAM_CH12 ; chno++)
	{
		if (HAL_SDMA_Reg_CB(chno, sdmaTrigger,  SdmaTxrCompNotifier) != HAL_OK)
		{
			printf("[SDMA] : Error : HAL_SDMA_Reg_CB failed for channel %d\n", chno);
		}
	}
}

static void sdmaChanWaitForTxrComp(int chan)
{
	if (sdmaChInfo[chan].txrCompNotifier)
		sdmaChInfo[chan].txrCompNotifier(chan);
	return;
}

static HAL_StatusTypeDef __HAL_SDMA_Init(void)
{
	resetSdma();
	// Do the interrupt stuff here
	setupSdmaIrq();

        return HAL_OK;
}

HAL_StatusTypeDef HAL_SDMA_Init(void)
{
	static int initdone = 0;
	if (!initdone) {
		initdone = 1;
		return __HAL_SDMA_Init();
	}
	return HAL_OK;
}

HAL_StatusTypeDef HAL_SDMA_xferWord(int chan, uint32_t * srcptr, uint32_t *dstptr, int wordLen)
{
	uint32_t *sendptr, *dendptr;
	sendptr = &srcptr[wordLen - 1];
	dendptr = &dstptr[wordLen - 1];
	setSdmaSram(chan, sendptr, dendptr, wordLen - 1, SDMA_XFER_WORD);
	setSdmaCh(chan);
	startSdma(chan);
	sdmaChanWaitForTxrComp(chan);
    return HAL_OK;
}

static int setSdmaSram(int ch, uint32_t* srcEndAddr, uint32_t * destEndAddr, int xferSzMinus1, SDMA_XFER_FORMAT xferFormat)
{
    uint32_t formatCode = 0;
    uint32_t rpowerCode = SDMA_SRAM_R_POWER_NO;

    if ((ch < SDMA_SRAM_CH0) || (ch > SDMA_SRAM_CH15))
        return -1; // return error cause we support channels 0 to 15


    switch (xferFormat)
    {
        case SDMA_XFER_BYTE:
            formatCode = SDMA_SRAM_DST_INC_BYTE | SDMA_SRAM_DST_SZ_BYTE | SDMA_SRAM_SRC_INC_BYTE | SDMA_SRAM_SRC_SZ_BYTE;
            break;

        case SDMA_XFER_HWORD:
            if (((( ((uint32_t)srcEndAddr) + 2) & 0x1) != 0) || (((((uint32_t)destEndAddr)+ 2) & 0x1) != 0)) return -1; // return error if address is not multiple of 2 bytes
            formatCode = SDMA_SRAM_DST_INC_HWORD | SDMA_SRAM_DST_SZ_HWORD | SDMA_SRAM_SRC_INC_HWORD | SDMA_SRAM_SRC_SZ_HWORD;
            break;

        case SDMA_XFER_WORD:
            if ((((((uint32_t)srcEndAddr) + 4) & 0x3) != 0) || (((((uint32_t)destEndAddr) + 4) & 0x3) != 0)) return -1; // return error if address is not multiple of 4 bytes
            formatCode = SDMA_SRAM_DST_INC_WORD | SDMA_SRAM_DST_SZ_WORD | SDMA_SRAM_SRC_INC_WORD | SDMA_SRAM_SRC_SZ_WORD;
            break;

        default:
            return -1;
            break;
    }

    if ((xferSzMinus1 + 1) > 512) rpowerCode = SDMA_SRAM_R_POWER_NO;
    else if ((xferSzMinus1 + 1) > 256) rpowerCode = SDMA_SRAM_R_POWER_512;
    else if ((xferSzMinus1 + 1) > 128) rpowerCode = SDMA_SRAM_R_POWER_256;
    else if ((xferSzMinus1 + 1) > 64) rpowerCode = SDMA_SRAM_R_POWER_128;
    else if ((xferSzMinus1 + 1) > 32) rpowerCode = SDMA_SRAM_R_POWER_64;
    else if ((xferSzMinus1 + 1) > 16) rpowerCode = SDMA_SRAM_R_POWER_32;
    else if ((xferSzMinus1 + 1) > 8) rpowerCode = SDMA_SRAM_R_POWER_16;
    else if ((xferSzMinus1 + 1) > 4) rpowerCode = SDMA_SRAM_R_POWER_8;
    else if ((xferSzMinus1 + 1) > 2) rpowerCode = SDMA_SRAM_R_POWER_4;
    else rpowerCode = SDMA_SRAM_R_POWER_2;

    SDMA_SRAM_TAB[ch].SRC_DATA_END_PTR =  (uint32_t)srcEndAddr;
    SDMA_SRAM_TAB[ch].DST_DATA_END_PTR = (uint32_t)destEndAddr;
    SDMA_SRAM_TAB[ch].CH_CFG = ((xferSzMinus1 << SDMA_SRAM_N_MINUS_1_SHIFT) | formatCode | rpowerCode | SDMA_SRAM_CYCLE_CTRL_BASIC);

    return 0;
}

static int setSdmaCh(int ch)
{
    if ((ch < SDMA_SRAM_CH0) || (ch > SDMA_SRAM_CH15))
        return -1; // return error cause we support channels 0 to 15
    SDMA->CHNL_ENABLE_SET = 1 << ch;
    SDMA->DMA_CFG = SDMA_DMA_CFG_MASTER_ENABLE;
    SDMA->CTRL_BASE_PTR = SDMA_SRAM_BASE;
    return 0;
}

static void resetSdma()
{
 #ifndef DSPC_AW
 #if 1  // Later handle with PM calls
	PMU->MISC_SW_WU		= PMU_MISC_SW_WU_SDMA_WU;
	PMU->FFE_FB_PF_SW_WU = PMU_FFE_FB_PF_SW_WU_FFE_WU;
	CRU->C01_CLK_GATE = 0x2DB;
 CRU->C08_X4_CLK_GATE = 0x1;
 CRU->C08_X1_CLK_GATE = 0xF;
#endif
#endif
	SDMA_BRIDGE->DMA_REQ	= 0;
	SDMA->CHNL_ENABLE_CLR	= 0xFFFFFFFFL;
	SDMA->DMA_CFG			= 0;
}


static int startSdma(int chan)
{
	if ((chan < SDMA_SRAM_CH0) || (chan > SDMA_SRAM_CH15))
		return -1;

	if (sdmaChInfo[chan].triggerSdma)
		sdmaChInfo[chan].triggerSdma(chan);

	return(0);
}

HAL_StatusTypeDef HAL_SDMA_xfer(int chan, uint32_t * srcptr, uint32_t *dstptr, int wordLen, SDMA_ch_cfg_t *chCfg)
{
    uint32_t rpowerCode = 0;
    uint32_t *sendptr, *dendptr;

    if ((chCfg->rpowerCode) > 512) rpowerCode = SDMA_SRAM_R_POWER_NO;
    else if (chCfg->rpowerCode > 256) rpowerCode = SDMA_SRAM_R_POWER_512;
    else if (chCfg->rpowerCode > 128) rpowerCode = SDMA_SRAM_R_POWER_256;
    else if (chCfg->rpowerCode > 64) rpowerCode = SDMA_SRAM_R_POWER_128;
    else if (chCfg->rpowerCode > 32) rpowerCode = SDMA_SRAM_R_POWER_64;
    else if (chCfg->rpowerCode > 16) rpowerCode = SDMA_SRAM_R_POWER_32;
    else if (chCfg->rpowerCode > 8) rpowerCode = SDMA_SRAM_R_POWER_16;
    else if (chCfg->rpowerCode > 4) rpowerCode = SDMA_SRAM_R_POWER_8;
    else if (chCfg->rpowerCode > 2) rpowerCode = SDMA_SRAM_R_POWER_4;
    else if (chCfg->rpowerCode > 0) rpowerCode = SDMA_SRAM_R_POWER_2;
    else rpowerCode = SDMA_SRAM_R_POWER_0;

    //else rpowerCode = SDMA_SRAM_R_POWER_2;

    SDMA_SRAM_TAB[chan].CH_CFG = ((chCfg->dest_addr_inc << 30) |
    							(chCfg->data_size << 28) |
								(chCfg->src_addr_inc << 26) |
								(chCfg->data_size << 24) |
								(rpowerCode) |
								((wordLen - 1) << 4) |   //Legth of the transfer
								(1 << 0));     //Basic cycle control

    if(chCfg->src_addr_inc == SDMA_ADDR_WIDTH_WORD)
        sendptr = &srcptr[wordLen - 1];
    else if(chCfg->src_addr_inc == SDMA_ADDR_WIDTH_NOINC)
        sendptr = srcptr;

    if(chCfg->dest_addr_inc == SDMA_ADDR_WIDTH_WORD)
      dendptr = &dstptr[wordLen - 1];
    else if(chCfg->dest_addr_inc == SDMA_ADDR_WIDTH_NOINC)
      dendptr = dstptr;

    SDMA_SRAM_TAB[chan].SRC_DATA_END_PTR =  (uint32_t)sendptr;
    SDMA_SRAM_TAB[chan].DST_DATA_END_PTR = (uint32_t)dendptr;

	setSdmaCh(chan);
	startSdma(chan);
	sdmaChanWaitForTxrComp(chan);
    return HAL_OK;
}
