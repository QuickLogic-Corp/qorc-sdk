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
 *    File   : eoss3_hal_wb.c
 *    Purpose: This file contains HAL API for Wishbone Master inside
 *             FFE subsystem
 *                                                          
 *=========================================================*/
#include "Fw_global_config.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include "eoss3_dev.h"
//#include "eoss3_hal_ffe.h"
//#include "eoss3_hal_rcc.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_hal_wb.h"
#include <test_types.h>
#include "s3x_clock_hal.h"
#include "dbg_uart.h"

/*!
 * \fn		HAL_StatusTypeDef HAL_WB_Transmit(UINT8_t ucOffset, UINT8_t ucVal, UINT8_t ucSlaveSel)
 * \brief 	Function to send data over Wishbone interface
 * \param	ucOffset        --- Wishbone register offset
 * \param       ucVal           --- Data
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_Transmit(UINT8_t ucOffset, UINT8_t ucVal, UINT8_t ucSlaveSel)
{
        UINT32_t ulRegVal = 0;

        ulRegVal = EXT_REGS_FFE->CSR;

        if (!((ulRegVal & WB_CSR_BUSY) || (ulRegVal & WB_CSR_MASTER_START))) {
                  //bits[7:6] = 2 to select SPI as slave by WB master
                  EXT_REGS_FFE->ADDR = (ucSlaveSel | ucOffset);
                  EXT_REGS_FFE->WDATA = ucVal;

                  if(ucSlaveSel == WB_ADDR_SPI0_SLAVE_SEL)
                    EXT_REGS_FFE->CSR = (WB_CSR_SPI0MUX_SEL_WBMASTER | WB_CSR_MASTER_WR_EN | WB_CSR_MASTER_START);
                  else if(ucSlaveSel == WB_ADDR_I2C1_SLAVE_SEL)
                    EXT_REGS_FFE->CSR = (WB_CSR_I2C1MUX_SEL_WBMASTER | WB_CSR_MASTER_WR_EN | WB_CSR_MASTER_START);
                  else if(ucSlaveSel == WB_ADDR_I2C0_SLAVE_SEL)
                    EXT_REGS_FFE->CSR = (WB_CSR_I2C0MUX_SEL_WBMASTER | WB_CSR_MASTER_WR_EN | WB_CSR_MASTER_START);

		  //QL_LOG_DBG_150K("addr = %x, csr = %x\r\n",EXT_REGS_FFE->ADDR,EXT_REGS_FFE->CSR);
        	return HAL_OK;
        }

	return HAL_ERROR;
}

/*!
 * \fn		HAL_StatusTypeDef HAL_WB_Receive(UINT8_t ucOffset, UINT8_t *buf, UINT8_t ucSlaveSel)
 * \brief 	Function to read data over Wishbone interface
 * \param	ucOffset        --- Wishbone register offset
 * \param       ucVal           --- Data
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_Receive(UINT8_t ucOffset, UINT8_t *buf, UINT8_t ucSlaveSel)
{
        UINT32_t ulReg_val = 0;
	UINT8_t ucCnt = 10;

	EXT_REGS_FFE->ADDR = (ucSlaveSel | ucOffset);

        if(ucSlaveSel == WB_ADDR_SPI0_SLAVE_SEL)
          EXT_REGS_FFE->CSR = (WB_CSR_SPI0MUX_SEL_WBMASTER | WB_CSR_MASTER_START);
        else if(ucSlaveSel == WB_ADDR_I2C1_SLAVE_SEL)
          EXT_REGS_FFE->CSR = (WB_CSR_I2C1MUX_SEL_WBMASTER | WB_CSR_MASTER_START);
        else if(ucSlaveSel == WB_ADDR_I2C0_SLAVE_SEL)
          EXT_REGS_FFE->CSR = (WB_CSR_I2C0MUX_SEL_WBMASTER | WB_CSR_MASTER_START);


	//Check for status
        do {
                ulReg_val = EXT_REGS_FFE->CSR;
                if (!((ulReg_val & WB_CSR_MASTER_START) || (ulReg_val & WB_CSR_BUSY)))
                        break;
                ucCnt--;
        } while (ucCnt > 0);

        *buf = EXT_REGS_FFE->RDATA;

        return HAL_OK;
}

/*!
 * \fn		HAL_StatusTypeDef HAL_WB_Init(UINT8_t ucSlaveSel)
 * \brief 	Function to initialize Wishbone interface
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_Init(UINT8_t ucSlaveSel)
{
        PadConfig  padcfg;

        //enable FFE power & clock domain
        PMU->FFE_PWR_MODE_CFG = 0x0;
        PMU->FFE_PD_SRC_MASK_N = 0x0;
        PMU->FFE_WU_SRC_MASK_N = 0x0;

        //wake up FFE
        PMU->FFE_FB_PF_SW_WU = 0x1;
        //check if FFE is in Active mode
        while(!(PMU->FFE_STATUS & 0x1));

	//HAL_SetClkGate(FFE_X1_CLK_TOP, C08X1_CLK_GATE_FFE_X1CLK,1);
//	HAL_SetClkGate(EFUSE_SDMA_I2S_FFE_PF_CLK_TOP, C01_CLK_GATE_FFE,1);
        S3x_Clk_Enable(S3X_FFE_X1_CLK);
        S3x_Clk_Enable(S3X_FFE_CLK);
/*
        QL_LOG_DBG_150K("c8_x1 freq = %ld\r\n", S3x_Clk_Get_Rate(S3X_FFE_X1_CLK));
      	QL_LOG_DBG_150K("C01_clk_gate = %x\r\n",CRU->C01_CLK_GATE);
*/
        switch(ucSlaveSel)
        {
            case WB_ADDR_SPI0_SLAVE_SEL:
                //MOSI
                padcfg.ucPin = PAD_6;
                padcfg.ucFunc = PAD6_FUNC_SPI_SENSOR_MOSI;
                padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
                padcfg.ucMode = PAD_MODE_OUTPUT_EN;
                padcfg.ucPull = PAD_NOPULL;
                padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
                padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
                padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

                HAL_PAD_Config(&padcfg);

        //dbg_str_hex32("MOSI - pad6 =", IO_MUX->PAD_6_CTRL);

                //MISO
                padcfg.ucPin = PAD_8;
                padcfg.ucFunc = PAD8_FUNC_SEL_SPI_SENSOR_MISO;
                padcfg.ucCtrl = PAD_CTRL_SRC_A0;
                padcfg.ucMode = PAD_MODE_INPUT_EN;
                padcfg.ucPull = PAD_PULLDOWN;
                padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
                padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
                padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

                HAL_PAD_Config(&padcfg);
        //dbg_str_hex32("MISO - pad8", IO_MUX->PAD_8_CTRL);

                //clk
                padcfg.ucPin = PAD_10;
                padcfg.ucFunc = PAD10_FUNC_SEL_SPI_SENSOR_CLK;
                padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
                padcfg.ucMode = PAD_MODE_OUTPUT_EN;
                padcfg.ucPull = PAD_NOPULL;
                padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
                padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
                padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

                HAL_PAD_Config(&padcfg);
        //dbg_str_hex32("CLK - pad10", IO_MUX->PAD_10_CTRL);

                //Slave Select
                padcfg.ucPin = PAD_9;
                padcfg.ucFunc = PAD9_FUNC_SEL_SPI_SENSOR_SSn_1;
                padcfg.ucCtrl = PAD_CTRL_SRC_A0;
                padcfg.ucMode = PAD_MODE_OUTPUT_EN;
                padcfg.ucPull = PAD_NOPULL;
                padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
                padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
                padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

                HAL_PAD_Config(&padcfg);

        //dbg_str_hex32("CS - pad9", IO_MUX->PAD_9_CTRL);
            break;

            case WB_ADDR_I2C0_SLAVE_SEL:

                padcfg.ucPin = PAD_0;
		padcfg.ucFunc = PAD0_FUNC_SEL_SCL_0;
		padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
		padcfg.ucMode = PAD_MODE_INPUT_EN;
		padcfg.ucPull = PAD_PULLUP;
		padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
		padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
		padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

		HAL_PAD_Config(&padcfg);


		padcfg.ucPin = PAD_1;
		padcfg.ucFunc = PAD1_FUNC_SEL_SDA_0;
		padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
		padcfg.ucMode = PAD_MODE_INPUT_EN;
		padcfg.ucPull = PAD_PULLUP;
		padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
		padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
		padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

		HAL_PAD_Config(&padcfg);
              break;

            case WB_ADDR_I2C1_SLAVE_SEL:
                padcfg.ucPin = PAD_33;
		padcfg.ucFunc = PAD33_FUNC_SEL_SCL_1;
		padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
		padcfg.ucMode = PAD_MODE_INPUT_EN;
		padcfg.ucPull = PAD_PULLUP;
		padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
		padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
		padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

		HAL_PAD_Config(&padcfg);


		padcfg.ucPin = PAD_32;
		padcfg.ucFunc = PAD32_FUNC_SEL_SDA_1;
		padcfg.ucCtrl = PAD_CTRL_SRC_OTHER;
		padcfg.ucMode = PAD_MODE_INPUT_EN;
		padcfg.ucPull = PAD_PULLUP;
		padcfg.ucDrv = PAD_DRV_STRENGHT_4MA;
		padcfg.ucSpeed = PAD_SLEW_RATE_SLOW;
		padcfg.ucSmtTrg = PAD_SMT_TRIG_DIS;

		HAL_PAD_Config(&padcfg);

              break;
        }

        return HAL_OK;
}

/*!
 * \fn		HAL_StatusTypeDef HAL_WB_DeInit(UINT8_t ucSlaveSel)
 * \brief 	Function to De-initialize Wishbone interface
 * \param       ucSlaveSel      --- Slave Select (I2C1 or I2C0 or SPI)
 * \return      HAL status
 */
HAL_StatusTypeDef HAL_WB_DeInit(UINT8_t ucSlaveSel)
{
        return HAL_OK;
}



