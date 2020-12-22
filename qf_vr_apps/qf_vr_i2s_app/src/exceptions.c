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
 *    File   : exceptions.c
 *    Purpose: This file contains the default exception handlers
 *             and exception table.
 *
 *=========================================================*/

#include "Fw_global_config.h"
#include <stdio.h>
#include <assert.h>
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"

#include "eoss3_hal_def.h"
#include "eoss3_hal_pad_config.h"
#include "eoss3_dev.h"
#include "eoss3_hal_audio.h"
#include "eoss3_hal_i2s_drv.h"
#include "eoss3_hal_rtc.h"
#include "eoss3_hal_uart.h"
#include "dbg_uart.h" /* dbg_print() functions */
#include "eoss3_hal_gpio.h"

#include "sec_debug.h"
//#include "ffe_ipc_lib.h"

//#include "vm1010.h"

extern TaskHandle_t xPUITaskHandle;
extern TaskHandle_t xSensorTouchTaskHandle;
extern void HAL_Timer_ISR(void);
void PMU_Timer_Handler(void);
void HAL_SPI_IRQHandler(void);
void SPI_DMA_Complete(void);
HAL_StatusTypeDef HAL_PMU_Timer_Interrupt_Callback();
void S3x_Tick_Hook(void);

extern void service_intr_from_host(void);
extern void service_ack_from_host(void);

extern void SendBLERxDataReady(void);

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
//extern uint32_t pmu_trigger;

extern uint32_t pmu_trigger; // POWER_MANAGEMENT


volatile uint32_t gv_hardfault_args[10];
volatile int      hardfault_loop;

static void spurious_interrupt( int lineno )
{
	dbg_fatal_error_int( "sperious-irq", lineno );
}


void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress, uint32_t lr_exec_ret )
{

	// backup core registers
    gv_hardfault_args[0] = pulFaultStackAddress[0];    // R0
    gv_hardfault_args[1] = pulFaultStackAddress[1];    // R1
    gv_hardfault_args[2] = pulFaultStackAddress[2];    // R2
    gv_hardfault_args[3] = pulFaultStackAddress[3];    // R3
    gv_hardfault_args[4] = pulFaultStackAddress[4];    // R12
    gv_hardfault_args[5] = pulFaultStackAddress[5];    // LR
    gv_hardfault_args[6] = pulFaultStackAddress[6];    // PC
    gv_hardfault_args[7] = pulFaultStackAddress[7];    // PSR
    gv_hardfault_args[8] = (unsigned long)pulFaultStackAddress;    // Stack Pointer
    gv_hardfault_args[9] = lr_exec_ret;
    if (lr_exec_ret & 0x10)
	    gv_hardfault_args[8] += 0x20;
    else
	    gv_hardfault_args[8] += 0x68;

    dbg_str("\n\n**hardfault**\n");
    dbg_str_hex32( "r0", gv_hardfault_args[0]  );
    dbg_str_hex32( "r1", gv_hardfault_args[1]  );
    dbg_str_hex32( "r2", gv_hardfault_args[2]  );
    dbg_str_hex32( "r3", gv_hardfault_args[3]  );
    dbg_str_hex32( "r12", gv_hardfault_args[4]  );
    dbg_str_hex32( "lr", gv_hardfault_args[5]  );
    dbg_str_hex32( "pc", gv_hardfault_args[6]  );
    dbg_str_hex32( "SP", gv_hardfault_args[7] );
    dbg_str("stackdump x256\n");
    dbg_memdump32( (intptr_t)(gv_hardfault_args[8]), (void  *)(gv_hardfault_args[8]), 256);
	// TODO
	// If we modify SHCSR, the reboot cause will be one of hardfault, userfault, busfault and MemManage.
	// We have to define more reboot causes.
	REBOOT_STATUS_REG &= ~REBOOT_CAUSE;
	REBOOT_STATUS_REG |= REBOOT_CAUSE_HARDFAULT;	/* CHANGING THIS VALUE OR REGISTER REQUIRE CORRESPONDING CHANGE IN BOOTLOADER */
	strcpy((char*)0x20000000, "hardfault");

    hardfault_loop = 1;
    while(hardfault_loop)
      ;

}


/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void) {
#if 1
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " mov r1, lr 	                                            \n"
        " bl prvGetRegistersFromStack                               \n"
    );
#else
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1);
#endif
}

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
	spurious_interrupt(__LINE__);
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
	spurious_interrupt(__LINE__);
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
	spurious_interrupt(__LINE__);
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
	spurious_interrupt(__LINE__);
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void SramSleepHandler(void)
{
	spurious_interrupt(__LINE__);
}

int ucount=0;
int urxcount=0;
void Uart_Handler(void) {
	if ((UART->UART_MIS & UART_MIS_RX))
	{
		uart_isr_handler(UART_ID_HW);
		urxcount++;
		UART->UART_ICR = UART_IC_RX;
	}
	else if (UART->UART_MIS & UART_MIS_RX_TIMEOUT) {
		uart_isr_handler(UART_ID_HW);
		ucount++;
		UART->UART_ICR = UART_IC_RX_TIMEOUT;
	}
	INTR_CTRL->OTHER_INTR &= UART_INTR_DETECT;
}

void Timer_Handler(void) {

	HAL_Timer_ISR();
	INTR_CTRL->OTHER_INTR &= TIMER_INTR_DETECT;
	NVIC_ClearPendingIRQ(Timer_IRQn);
}

extern void WDT_ISR(void);
void CpuWdtInt_Handler(void) {
	WDT_ISR();
	INTR_CTRL->OTHER_INTR &= WDOG_INTR_DETECT;
	NVIC_ClearPendingIRQ(CpuWdtInt_IRQn);
}

void CpuWdtRst_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void BusTimeout_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Fpu_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Pkfb_Handler(void) {
	//fifoISR();
	INTR_CTRL->OTHER_INTR &= PKFB_INTR_DETECT;
}

void I2s_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Audio_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void SpiMs_Handler(void) {
	uint32_t irq_status = (SPI_MS->ISR & 0x3f);
	uint32_t new_mask;

	//Disable interrupt
	INTR_CTRL->OTHER_INTR_EN_M4 &= ~(SPI_MS_INTR_EN_M4);

	//clear interrupt at top
	INTR_CTRL->OTHER_INTR &= (SPI_MS_INTR_EN_M4);

	//Error Handling
	if (irq_status
			& (ISR_RXFOIM_ACTIVE | ISR_RXUIM_ACTIVE | ISR_TXOIM_ACTIVE)) {
		//clear interrupts
		SPI_MS->TXOICR |= 0x1;
		SPI_MS->RXOICR |= 0x1;
		SPI_MS->RXUICR |= 0x1;
		goto IRQ_HANDLED;
	}

	if (irq_status & ISR_TXEIM_ACTIVE) {
		//Disable interrupt
		new_mask = ((SPI_MS->IMR) & ~(ISR_TXEIM_ACTIVE));
		SPI_MS->IMR = new_mask;

		HAL_SPI_IRQHandler();
	}

	IRQ_HANDLED:
	//enable SPI MS interrupt at top
	INTR_CTRL->OTHER_INTR_EN_M4 |= SPI_MS_INTR_EN_M4;
}

void CfgDma_Handler(void) {
	//Disable interrupt
	INTR_CTRL->OTHER_INTR_EN_M4 &= ~(CFG_DMA_DONE_EN_M4);

	//clear interrupt at top
	INTR_CTRL->OTHER_INTR &= (CFG_DMA_DONE_EN_M4);

        SPI_DMA_Complete();

	INTR_CTRL->OTHER_INTR_EN_M4 |= (CFG_DMA_DONE_EN_M4);
}

void PmuTimer_Handler(void) {
//    PMU->PMU_TIMER_CFG_1 = 0x0;
//    PMU_Timer_Handler();
}

void AdcDone_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void RtcAlarm_Handler(void) {
    INTR_CTRL->OTHER_INTR_EN_M4 &= ~(RTC_INTR_DETECT);
    INTR_CTRL->OTHER_INTR &= RTC_INTR_DETECT;
    HAL_RTC_ISR();
}

void ResetInt_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Ffe0_Handler(void) {
#if 0 //for debug
        uint32_t ffe0overrun = 0;
        uint32_t sm0overrun = 0;
#endif
	//Disable interrupt
	INTR_CTRL->OTHER_INTR_EN_M4 &= ~(FFE0_INTR_OTHERS_EN_M4);

	//clear interrupt at top
	INTR_CTRL->OTHER_INTR &= (FFE0_INTR_OTHERS_EN_M4);
#if 0 //for debug
        if (EXT_REGS_FFE->INTERRUPT & FFE_INTR_FFE0_OVERRUN) {
          ffe0overrun = 1;
        }
        if (EXT_REGS_FFE->INTERRUPT & FFE_INTR_FFE0_SM0_OVERRUN) {
          sm0overrun = 1;
        }
#endif
//	HAL_SPI_IRQHandler(&SpiHandle);
	//while(1);
}
HAL_FBISRfunction FB_ISR [MAX_FB_INTERRUPTS]={NULL,NULL,NULL,NULL};

int fb_raw = 0;
int fb_intr = 0;
void FbMsg_Handler(void) {

#if ( configSAVE_IRQ_HISTORY==1 )
        sec_save_irq_history("FbMsg\0", xTaskGetTickCountFromISR());
#endif
fb_raw = INTR_CTRL->FB_INTR_RAW;
fb_intr = INTR_CTRL->FB_INTR;
// detect which one of the FB generators inetrrupted
//  if ( INTR_CTRL->FB_INTR_RAW & FB_0_INTR_RAW)
  if ( INTR_CTRL->FB_INTR & FB_0_INTR_RAW)
  {
    // call that particualr ISR
    if (FB_ISR[FB_INTERRUPT_0])
    FB_ISR[FB_INTERRUPT_0]();
    // clear that interrupt at FB level
    INTR_CTRL->FB_INTR = (FB_0_INTR_DETECT);
  }
//  if ( INTR_CTRL->FB_INTR_RAW & FB_1_INTR_RAW)
  if ( INTR_CTRL->FB_INTR & FB_1_INTR_RAW)
  {
    // call that particualr ISR
   if (FB_ISR[FB_INTERRUPT_1])
    FB_ISR[FB_INTERRUPT_1]();
    // clear that interrupt at FB level
    INTR_CTRL->FB_INTR = (FB_1_INTR_DETECT);
  }
//  if ( INTR_CTRL->FB_INTR_RAW & FB_2_INTR_RAW)
  if ( INTR_CTRL->FB_INTR & FB_2_INTR_RAW)
  {
    // call that particualr ISR
    if (FB_ISR[FB_INTERRUPT_2])
    FB_ISR[FB_INTERRUPT_2]();
    // clear that interrupt at FB level
    INTR_CTRL->FB_INTR = (FB_2_INTR_DETECT);
  }
// if ( INTR_CTRL->FB_INTR_RAW & FB_3_INTR_RAW)
  if ( INTR_CTRL->FB_INTR & FB_3_INTR_RAW)
  {
    // call that particualr ISR
    if (FB_ISR[FB_INTERRUPT_3])
    FB_ISR[FB_INTERRUPT_3]();
    // clear that interrupt at FB level
   INTR_CTRL->FB_INTR = (FB_3_INTR_DETECT);
  }
}



void FB_RegisterISR(UINT32_t fbIrq, HAL_FBISRfunction ISRfn)
{
  if (fbIrq< MAX_FB_INTERRUPTS){
    FB_ISR [fbIrq] = ISRfn;
  }
 }

void FB_ConfigureInterrupt ( UINT32_t fbIrq, UINT8_t type, UINT8_t polarity, UINT8_t destAP,UINT8_t destM4 )
{
  //Edege or level and polarity
  if ( type == FB_INTERRUPT_TYPE_LEVEL){
    INTR_CTRL->FB_INTR_TYPE &= ~( 1<< fbIrq);
    if ( polarity == FB_INTERRUPT_POL_LEVEL_LOW)
      INTR_CTRL->FB_INTR_POL &= ~(1<<fbIrq);
    else
      INTR_CTRL->FB_INTR_POL |=( 1<< fbIrq);
  }
  else{
    INTR_CTRL->FB_INTR_TYPE |=  ( 1<< fbIrq);
    if ( polarity == FB_INTERRUPT_POL_EDGE_FALL)
      INTR_CTRL->FB_INTR_POL &= ~(1<<fbIrq);
    else
      INTR_CTRL->FB_INTR_POL |=( 1<< fbIrq);
  }

  // Destination AP
  if ( destAP == FB_INTERRUPT_DEST_AP_DISBLE)
    INTR_CTRL->FB_INTR_EN_AP &= ~( 1<< fbIrq);
  else
    INTR_CTRL->FB_INTR_EN_AP |= ( 1<< fbIrq);

  // Destination M4
  if ( destM4 == FB_INTERRUPT_DEST_M4_DISBLE)
    INTR_CTRL->FB_INTR_EN_M4 &= ~( 1<< fbIrq);
  else
    INTR_CTRL->FB_INTR_EN_M4 |= ( 1<< fbIrq);
}





void SensorGpio_Handler(void)
{
	//spurious_interrupt(__LINE__);

	NVIC_DisableIRQ(Gpio_IRQn);
	if(INTR_CTRL->GPIO_INTR & (1<<GPIO_7))
	{
        /* This is ack from Host to Device */
		service_ack_from_host();
		INTR_CTRL->GPIO_INTR |= (1<<GPIO_7);
	}
    if(INTR_CTRL->GPIO_INTR & (1<<GPIO_6))
	{
        /* This is QL_INT from Host to Device*/
		service_intr_from_host();
		INTR_CTRL->GPIO_INTR |= (1<<GPIO_6);
	}

	NVIC_ClearPendingIRQ(Gpio_IRQn);
	NVIC_EnableIRQ(Gpio_IRQn);
}


uint32_t btn_press_msec = 0;
uint32_t btn_release_msec = 0;
int OD_On;
//extern volatile h_uint8 LightExpFlag;


void M4SramSleep_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void LDO30_PG_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void LDO50_PG_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void DMIC_Voice_Det_Handler(void)
{
	onDmicOn();
}

void ApPDMClkOn_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void ApPDMClkOff_Handler(void)
{
	spurious_interrupt(__LINE__);
}
#include "datablk_mgr.h"
extern outQ_processor_t audio_isr_outq_processor ;
void Dmac0BlkDone_Handler(void)
{
     onDmac0BlockDone();
}

void Dmac0BufDone_Handler(void)
{
    if (audio_isr_outq_processor.process_func)
      audio_isr_outq_processor.process_func();
	else
      onDmac0BufferDone();
}

void Dmac1BlkDone_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Dmac1BufDone_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma0Done_Handler(void)
{
    HAL_I2S_SLAVE_SDMA_Assp_Done();
}

void Sdma1Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma2Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma3Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma4Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma5Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma6Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma7Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma8Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma9Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma10Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void Sdma11Done_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void SdmaErr_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void I2S_SlvM4TxOr_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void SRAM_128KB_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void LPSD_Voice_Det_Handler(void)
{
	HAL_Audio_ISR_LpsdOn();
}

void FfeWdt_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void ApBoot_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void SwInt2_Handler(void)
{
	spurious_interrupt(__LINE__);
}

void SwInt1_Handler(void)
{
	/* this is the BLE interrupt */
	/* Disable the Interrupt */
	INTR_CTRL->SOFTWARE_INTR_1_EN_M4 &= ~(SW_INTR_1_EN_M4);

//    SendBLERxDataReady();
    /* Clear the pending interrupt */
    INTR_CTRL->SOFTWARE_INTR_1 = 0;
    NVIC_ClearPendingIRQ(SwInt1_IRQn);
    /* Re-Enable the Interrupt */
    INTR_CTRL->SOFTWARE_INTR_1_EN_M4 = SW_INTR_1_EN_M4;
}

void Ffe1Msg_Handler(void)
{
	spurious_interrupt(__LINE__);
}


void FFE_IPC_FIFO_ISR(void);                            // to remove warnings 	function declaration

void Ffe0Msg_Handler(void)
{
#if FFE_DRIVERS
    QL_FFE0MSG_ISR();
	NVIC_ClearPendingIRQ(Ffe0Msg_IRQn);
#else
    spurious_interrupt(__LINE__);
#endif
}

void sdmaDone0Handler()
{
	spurious_interrupt(__LINE__);
}

void sdmaErrorHandler()
{
	spurious_interrupt(__LINE__);
}

// Audio Related
void lpsdVoiceDetHandler()
{
	spurious_interrupt(__LINE__);
}

void dmicVoiceDetHandler()
{
	spurious_interrupt(__LINE__);
}

void reservedHandler()
{
	spurious_interrupt(__LINE__);
}

void apPdmClockOnHandler()
{
	spurious_interrupt(__LINE__);
}

void apPdmClockOffHandler()
{
	spurious_interrupt(__LINE__);
}

void dmac0BlockDoneHandler()
{
	spurious_interrupt(__LINE__);
}

void dmac0BufferDoneHandler()
{
	spurious_interrupt(__LINE__);
}

void dmac1BlockDoneHandler()
{
	spurious_interrupt(__LINE__);
}

void dmac1BufferDoneHandler()
{
	spurious_interrupt(__LINE__);
}

void lpsdVoiceOffHandler()
{
	HAL_Audio_ISR_LpsdOff();
}

void dmicVoiceOffHandler()
 {
	onDmicOff();
}

void i2sSlaveHandler()
{
	spurious_interrupt(__LINE__);
}


/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	extern void printAllocList();

	taskDISABLE_INTERRUPTS();
	//printAllocList(); (save code space)
    dbg_str("vApplicationMallocFailedHook..\n");
	invoke_soft_fault();

}
/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
    dbg_str("vApplicationStackOverflowHook..\n");
	invoke_soft_fault();

}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
    //S3x_Tick_Hook();
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
#ifdef PM_TEST
  	uint32_t wic_status = 0;

	if(pmu_trigger >= 10000)
	{
		QL_LOG_ERR_150K("down\r\n");
			pmu_trigger = 0;

			CM4Shutdown();

	}
#endif
}
