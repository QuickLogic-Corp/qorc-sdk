/*==========================================================
 *                                                          
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : tfl_task.c
 *    Purpose: tinyfpga loader. 
 * $Revision: 4224$
 * $Date: 2011-02-22$
 *=========================================================*/

/** @file tfl.c */


#include "Fw_global_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "eoss3_hal_spi.h"
#include "eoss3_hal_uart.h"
#include "spi_flash.h"
//#include "tfl_task.h"
#include "dbg_uart.h"
#include "RtosTask.h"

void        tfl_task(void*);
static void SPIM_Setup(void);

xTaskHandle xHandleTfl;

uint8_t atflPkt[5];
uint8_t atflCmd[1024];
uint8_t atflResponse[1024];

int         c;
int         ipkt = 0;
int         icmd = 0;
uint32_t    kbWrite;
uint32_t    kbRead;
uint32_t    xaddr;
uint32_t    xval;
uint32_t    xret;
uint32_t    xtickStart;
uint32_t    xtime;

//-------------------- External functions -----------//
void toggle_downloading_led(int high_time_msec, int low_time_msec);

//--------------------  Debug -----------------------//
int fzap = 0;


void program_flash( void *pParameter )
{
    (void)(pParameter);
    uint16_t kiter = 0;
    
    SPIM_Setup();

    while(1) {
        // Reset pointers
        ipkt = 0;
        icmd = 0;
        if (fzap == 1) {
            dbg_str("zap");
        }
        // Get pkt (first 5 bytes)
        while (ipkt != 5) {
            vTaskDelay(0);
            if( uart_rx_available( UART_ID_BOOTLOADER ) ){
                c = uart_rx( UART_ID_BOOTLOADER  );
                //First 5 bytes are pkt
                atflPkt[ipkt++] = c;
                if (atflPkt[0] == 0) {
                    dbg_str("boot()\n");
                    ipkt = 0;
					return;
                }
            } 
			toggle_downloading_led(50,750);
        }
        // Use pkt info to determine write and read data lengths
        kbWrite = (atflPkt[2] << 8) | atflPkt[1];
        kbRead = (atflPkt[4] << 8) | atflPkt[3];
        atflPkt[0] = 0xA5;

        // Grab the write data
        while (icmd < kbWrite) {
            if( uart_rx_available( UART_ID_BOOTLOADER ) ){
                c = uart_rx( UART_ID_BOOTLOADER  );
                atflCmd[icmd++] = c;
            }
        }
        // Process command
        switch(atflCmd[0]) {
        case 0xAB:
            dbg_str("Release Deep Power-Down() ... ");
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_NoResponse);
            dbg_str("Done\n");
            uart_tx_buf(UART_ID_BOOTLOADER, atflResponse, kbRead);
            break;
        case 0x9F:
            dbg_str("Read ID() ... ");
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_WithResponse);
            xval = (atflResponse[2] << 16) | (atflResponse[1] << 8) | (atflResponse[0] << 0);
            dbg_str_hex32("Done", xval);
            uart_tx_buf(UART_ID_BOOTLOADER, atflResponse, kbRead);
            break;
        case 0x05:
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_WithResponse);
            xval = (atflResponse[1] << 8) | (atflResponse[0] << 0);
            uart_tx_buf(UART_ID_BOOTLOADER, atflResponse, kbRead);
            break;
        case 0x20:
            xaddr = (atflCmd[1] << 16) | (atflCmd[2] << 8) | atflCmd[3];
            dbg_str_hex32("Addr", xaddr);
            dbg_str("Erase 4 KBytes() .. ");
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_NoResponse);
            dbg_str("done\n");
            break;
        case 0x52:
            xaddr = (atflCmd[1] << 16) | (atflCmd[2] << 8) | atflCmd[3];
            dbg_str_hex32("Addr", xaddr);
            dbg_str("Erase 32 KBytes() .. ");
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_NoResponse);
            dbg_str("done\n");
            break;
        case 0xD8:
            xaddr = (atflCmd[1] << 16) | (atflCmd[2] << 8) | atflCmd[3];
            dbg_str_hex32("Addr", xaddr);
            dbg_str("Erase 64 KBytes() .. ");
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_NoResponse);
            dbg_str("done\n");
            break;
        case 0x06:
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, CMD_NoResponse);
            break;
        case 0x02:
            xaddr = (atflCmd[1] << 16) | (atflCmd[2] << 8) | atflCmd[3];
            xval = (atflCmd[7] << 24) |(atflCmd[6] << 16) | (atflCmd[5] << 8) | atflCmd[4];
            xret = spi_flash_program(atflCmd, 4, &atflCmd[4], kbWrite-4, NULL);
            if (xret != FlashOperationSuccess) {
                dbg_str("Page Program failed\n");
                while (1);
            }
            break;
        case 0x0B:
            xaddr = (atflCmd[1] << 16) | (atflCmd[2] << 8) | atflCmd[3];
            spi_flash_cmd(atflCmd, kbWrite, atflResponse, kbRead, READ_CMD);
            uart_tx_raw_buf(UART_ID_BOOTLOADER, atflResponse, kbRead);
            break;
        default:
            dbg_str_hex8("Unknown OpCode", (int)atflCmd[0]);
            //while(1);
            break;
        }
    }
}

static void SPIM_Setup(void)
{
  
    //SPI master init for SPI flash
    spiFlashHandle.Init.ucFreq       = SPI_BAUDRATE_5MHZ; //above 5MHz does not work
    spiFlashHandle.Init.ucSPIInf     = SPI_4_WIRE_MODE;
    spiFlashHandle.Init.ucSSn        = SPI_SLAVE_1_SELECT;
    spiFlashHandle.Init.ulCLKPhase   = SPI_PHASE_1EDGE;
    spiFlashHandle.Init.ulCLKPolarity = SPI_POLARITY_LOW;
    spiFlashHandle.Init.ulDataSize   = SPI_DATASIZE_8BIT;
    spiFlashHandle.Init.ulFirstBit   = SPI_FIRSTBIT_MSB;
    spiFlashHandle.ucSPIx            = SPI1_MASTER_SEL;

    if(HAL_SPI_Init(&spiFlashHandle) != HAL_OK)
    {
        printf("SPI_Init failed\r\n");
        configASSERT(0); //will hang here with out flash access
        return;
    }
    
    //make sure there is a Flash chip
    if(read_flash_id() == 0)
    {
       configASSERT(0); //will hang here if cannot read flash
    }
    
	return;
}
