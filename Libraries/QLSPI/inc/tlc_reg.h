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

#ifndef TLC_REG_H_INCLUDED
#define TLC_REG_H_INCLUDED

#define MISC_CTRL_BASE	0x40005000
#define INTR_CTRL_BASE	0x40004800

#define MAX_WAIT	50

#define QLULPSH_CMD_READ	0x0

#define BOOTL1_START_ADDR                                         (0x20000000)
#define APP_START_ADDR                                            (0x20000000)

#define APP_START_ADDR_2                                            (0x20020000)
#define APP_START_ADDR_3                                            (0x20040000)

#define MISC_POR_0_ADDR                                           (0x40004400)
#define MISC_POR_2_ADDR                                           (0x40004408)
#define MISC_M4_STATUS                                            (0x40004480)
#define SW_MB_1_ADDR                                              (0x40005110)
#define SW_MB_2_ADDR                                              (0x40005114)


#define MISC_POR_0_RELEASE_CPU_RESET                              (0x01)

#define FIFO0	0
#define FIFO1	1

// QL Register Define
#define SPITLC_CM_FIFO_0_DATA                                     (0x09)
#define SPITLC_CM_FIFO_1_DATA                                     (0x0A)
#define SPITLC_CM_FIFO_2_DATA                                     (0x0B)
#define SPITLC_CM_FIFO_8K_DATA                                    (0x0C)
#define SPITLC_MEM_ADDR_BYTE_0                                    (0x20)
#define SPITLC_MEM_ADDR_BYTE_1                                    (0x21)
#define SPITLC_MEM_ADDR_BYTE_2                                    (0x22)
#define SPITLC_MEM_ADDR_BYTE_3                                    (0x23)
#define SPITLC_MEM_DATA_BYTE_0                                    (0x28)
#define SPITLC_MEM_DATA_BYTE_1                                    (0x29)
#define SPITLC_MEM_DATA_BYTE_2                                    (0x2A)
#define SPITLC_MEM_DATA_BYTE_3                                    (0x2B)
#define SPITLC_AHB_STATUS                                         (0x2F)
#define SPITLC_AHB_ACCESS_CTL                                     (0x30)
#define SPITLC_SCRATCH_BYTE                                       (0x31)
#define SPITLC_TAMAR_STATUS                                       (0x32)
#define SPITLC_DMA_DEBUG_CTL0                                     (0x36)
#define SPITLC_DMA_DEBUG_CTL1                                     (0x37)
#define SPITLC_DMA_ADDR0                                          (0x38)
#define SPITLC_DMA_ADDR1                                          (0x39)
#define SPITLC_DMA_ADDR2                                          (0x3A)
#define SPITLC_DMA_ADDR3                                          (0x3B)
#define SPITLC_DMA_BURST_SZ0                                      (0x3C)
#define SPITLC_DMA_BURST_SZ1                                      (0x3D)
#define SPITLC_DMA_STATUS                                         (0x3F)
#define SPITLC_DMA_RD_DATA                                        (0x40)
#define SPITLC_DEVICE_ID_BYTE                                     (0x7F)

#define SPITLC_DMA_DBG_CTRL0		0x36
#define SPITLC_DMA_DBG_CTRL1		0x37
#define SPITLC_DMA_ADD0		        0x38
#define SPITLC_DMA_BRUST_SIZE0		0x3C
#define SPITLC_DMA_BRUST_SIZE1		0x3D
//#define SPITLC_DMA_STATUS			0x3F
#define SPITLC_DMA_READ_DATA		0x40

#define SW_INTR_1_EN_AP_REG		(INTR_CTRL_BASE+0x44)
#define SW_INTR_1_EN_M4	        	(INTR_CTRL_BASE+0x48)



#define SW_INTR_2_EN_AP_REG		(INTR_CTRL_BASE+0x54)
#define SW_INTR_2_EN_M4_REG		(INTR_CTRL_BASE+0x58)
typedef enum {

	//SW_INTR_1               = ((uint32_t) (0x00000001)),
	SW_INTR_1_REG			= (INTR_CTRL_BASE+0x040),
	SW_INTR_2_REG			= (INTR_CTRL_BASE+0x050),
	QLULPSH_ENABLE			= 1,
	QLULPSH_DISABLE			= 0,
	QLULPSH_CMD_WRITE		= 0x1,
	OPER_BIT_LOC			= 7,
	ADDR_MASK				= 0x7F,
	ONE_BYTE				= 1,
	MISC_MAILBOX_REG_0	 	= MISC_CTRL_BASE + 0x110,
	MISC_MAILBOX_REG_1	 	= MISC_CTRL_BASE + 0x114,
} QL_DEF;

#define CREATE_CMD(operation, addr) (uint8_t) ((operation << OPER_BIT_LOC) | (addr & ADDR_MASK))

int32_t tlc_reg_read( uint32_t addr, uint8_t *readBuf, uint32_t length);
int32_t tlc_reg_write(uint32_t addr, uint8_t *writeBuf, uint32_t length);

#endif // TLC_REG_H_INCLUDED
