/*==========================================================
 * Copyright 2021 QuickLogic Corporation
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
 *    File   : ql_aecTask.h
 *    Purpose: 
 *                                                          
 *=========================================================*/

/* Standard includes. */
#include <stdio.h>
/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <RtosTask.h>
#include "Fw_global_config.h"

#if (AEC_ENABLED == 1)
void cb_notify_i2sRx_intr_from_FPGA(void);

#include "eoss3_hal_audio.h"
#include "ql_aecTask.h"
#include "eoss3_dev.h"
#include "eoss3_hal_i2s.h"
#include "eoss3_hal_i2s_drv.h"
#include "eoss3_hal_fpga_i2s_slave.h"
#include "eoss3_hal_fpga_decimation_fir.h"
#include "eoss3_hal_fpga_FLL.h"

#include "s3x_clock.h"
#include "qlsh_commands.h"
//#include "ql_i2sRxTask.h"

#include "eoss3_hal_audio_reg.h"
#include "s3x_dfs.h"
#include "fsm.h"
#include "ql_controlTask.h"

xTaskHandle xHandleTaskAEC;
QueueHandle_t xHandleQueueAEC;
struct xQ_Packet i2s_packet;

extern void HAL_I2S_Slave_FB_Cfg_Gpio_Intr(void);
extern void hif_release_mch_inputQ(void);

#ifdef ENABLE_I2S_SLAVE_FB_RX

void slave_fb_data_handler(uint8_t i2s_id_sel,
                          uint32_t const * p_data_received,
                          uint32_t       * p_data_to_send,
                          uint16_t         buffer_size)
{
   /* here we should process the data further since we are not getting the index of the data
   ignore */
   return;
}
#endif

static bool fb_i2s_enable=false;

int FB_I2S_Slave_Rx_Enable(void)
{
   I2S_Config_t p_i2s_fb_cfg={0};
   UINT32_t ret_val=true;

   if(fb_i2s_enable==true)
   {
      printf("\nFB I2S RX already enabled\n");
      return ret_val;
   }

   p_i2s_fb_cfg.ch_sel = I2S_CHANNELS_MONO;
   p_i2s_fb_cfg.i2s_wd_clk = 16;
   p_i2s_fb_cfg.mono_sel = I2S_CHANNEL_MONO_LEFT;
   p_i2s_fb_cfg.sdma_used = 12;
   ret_val = HAL_I2S_Init(I2S_SLAVE_FABRIC_RX, &p_i2s_fb_cfg, slave_fb_data_handler);
   if(ret_val != HAL_I2S_SUCCESS)
   {
      printf ("\nError in I2S Slave Init !\n");
      ret_val=false;
      return ret_val;
   }

   fb_i2s_enable=true;
   ret_val=true;
#if 0
   printf("\n *** I2S RX EN ***\n");
#endif
   return ret_val;
}

int FB_I2S_Slave_Rx_Disable(void)
{
   uint32_t ret_val=true;

   if(fb_i2s_enable==false)
   {
      printf("\nFB I2S RX already disabled\n");
      return ret_val;
   }

   ret_val=HAL_I2S_Stop(I2S_SLAVE_FABRIC_RX);
   if(ret_val != HAL_I2S_SUCCESS)
   {
      printf ("\nError in I2S Stop !\n");
      ret_val=false;
      return ret_val;
   }

   fb_i2s_enable=false;
#if 0
   printf("\n *** I2S RX DIS ***\n");
#endif
   
   return ret_val;

}

void Enable_I2S_data(void)
{
   eoss3_hal_fabric_i2s_slave_clks_enable();

   S3x_Set_Qos_Req(S3X_FB_16_CLK, MIN_CPU_FREQ, C10_N4_CLK);
   S3x_Set_Qos_Req(S3X_CLKGATE_FB, MIN_OP_FREQ, C09_N4_CLK);
   S3x_Set_Qos_Req(S3X_SDMA_CLK, MIN_OP_FREQ, C01_N4_CLK);

   HAL_FB_I2SRx_Ref_input_DmaStart();
}

void Disable_I2S_data(void)
{
   eoss3_hal_fabric_i2s_slave_clks_disable();

   S3x_Clear_Qos_Req(S3X_FB_16_CLK, MIN_CPU_FREQ);
   S3x_Clear_Qos_Req(S3X_CLKGATE_FB, MIN_OP_FREQ);
   S3x_Clear_Qos_Req(S3X_SDMA_CLK, MIN_OP_FREQ);
}
//This starts aec 
void enable_aec_task(void)
{
  //eoss3_hal_fabric_i2s_slave_clks_enable();
  
   HAL_I2S_Slave_FB_Register(); 
  
   S3x_Register_Qos_Node(S3X_FB_16_CLK);
   S3x_Register_Qos_Node(S3X_CLKGATE_FB);
   S3x_Register_Qos_Node(S3X_SDMA_CLK);

   FB_I2S_Slave_Rx_Enable();
   Enable_I2S_data();

   //just enable and dissable
   FB_I2S_Slave_Rx_Disable();
   Disable_I2S_data();

}
void send_host_vr_msg(void)
{

  struct xCQ_Packet CQpacket;

  CQpacket.ceEvent = CEVENT_VR_TRIGGER;
  addPktToControlQueue(&CQpacket);

  return;
}
extern void HAL_Audio_LPSD_Enable(bool enable);
extern void HAL_Audio_LPSD_Int(bool fEnable);
extern void HAL_Audio_StartDMA( void );
extern void HAL_Audio_StopDMA( bool fUsingLeftChannel, bool fUsingRightChannel );
extern struct HAL_Audio_Config  AudioHW_FSMConfigData;
extern void enable_stream_VR(void);
extern void hifcb_cmd_host_process_on(void *pdata, int len);
extern void hifcb_cmd_host_process_off(void *pdata, int len);
extern void ControlEventSend(enum control_event ce);
extern void ProcessControlEvent(enum control_event ceEvent);
extern int S3x_Set_Max_Policy(void);
extern void DFS_AlignPolicy(uint8_t policy);
extern void reset_aec_temp(void);
extern void QL_Audio_Voice_Start(void);
extern void setStreamingOn(void);
extern void set_spi_session_qos(uint8_t status);
void set_no_fifo_audio(void)
{
  //DFS_AlignPolicy(4);
  DFS_Start(6); //6);// 4);
//set_DFS_policy();

//  printf("\n\n--Pro--\n\n");
  //reset_aec_temp();
  ControlEventSend(CEVENT_LPSD_ON);
  vTaskDelay(1);
  
  QL_Audio_Voice_Start();
  HAL_Audio_LPSD_Enable(false); //no internal processing
  HAL_Audio_LPSD_Int(false);
  HAL_Audio_StartDMA();
  //enable_stream_VR();

  hif_release_mch_inputQ();
  
  vTaskDelay(4);
  send_host_vr_msg();
  
}
void reset_no_fifo_audio(void)
{
  ControlEventSend(CEVENT_LPSD_OFF);
  
  //reset_DFS_policy();
  HAL_Audio_LPSD_Enable(true);
  HAL_Audio_LPSD_Int(true);
  HAL_Audio_StopDMA(AudioHW_FSMConfigData.fUsingLeftChannel, AudioHW_FSMConfigData.fUsingRightChannel);
}

extern uint32_t i2s_rx_enabled;
extern void enable_i2s_disconnect(void);
extern int aec_enabled;

int i2s_rx_enabled_2;
int i2s_rx_enabled_3;

int host_i2s_enabled = 0;
void aecTaskHandler (void *pvParameters)
{
   struct xQ_Packet   rcvdAudio;
   BaseType_t     xResult;

   //enable FLL first
   HAL_FB_FLL_Init(0,0);
   NVIC_DisableIRQ(FbMsg_IRQn);
   
   //vTaskDelay(10);
   enable_aec_task();
#if 0   
   Enable_I2S_data();
   NVIC_ClearPendingIRQ(FbMsg_IRQn);
   NVIC_EnableIRQ(FbMsg_IRQn);
#endif
   
   while(1)
   {
      xResult = xQueueReceive(xHandleQueueAEC, &(rcvdAudio), portMAX_DELAY);

      if( xResult == pdPASS )
      {
            /* Parse input message and process the request */
            switch( rcvdAudio.ucCommand )
            {
            case eCMD_AUDIO_I2S_DATA_START:
               {
                  set_no_fifo_audio();
                  host_i2s_enabled = 1;
                  i2s_rx_enabled_2 = i2s_rx_enabled;
                  i2s_rx_enabled = 0;
                  aec_enabled = 0;
                  FB_I2S_Slave_Rx_Enable();
                  Enable_I2S_data();
                  HAL_FB_FLL_Enable();
                  NVIC_ClearPendingIRQ(FbMsg_IRQn);
                  NVIC_EnableIRQ(FbMsg_IRQn);
                  enable_i2s_disconnect(); 

                  break;
               }
            case eCMD_AUDIO_I2S_DATA_STOP:
               {
                  /* Stop Clock Sync */
                  hifcb_cmd_host_process_off((void *)0, 0);
                  i2s_rx_enabled_3 = i2s_rx_enabled;
                  host_i2s_enabled = 0;
                  aec_enabled =0;
                  HAL_FB_FLL_Disable();
                  FB_I2S_Slave_Rx_Disable();
                  Disable_I2S_data();

                  NVIC_DisableIRQ(FbMsg_IRQn);
                  reset_no_fifo_audio();

                  break;
               }
            case eCMD_AUDIO_CLK_SYNC_INVOKE:
               {
                 printf("Invalid case in AEC TASK\r\n");
                 break;
               }
            default:
               printf("Unknown SRC in AEC TASK\r\n");
               break;
            }
      }
   }
   //return the AEC task
   //while (1) 
   {
     printf("===Error - exited the AEC (aecTaskHandler) loop=== \n");
     vTaskDelay(100);
     ;
   }
}

signed portBASE_TYPE StartRtosTaskAEC(void)
{
   static UINT8_t ucParameterToPass;


   /* Create queue for AP Task */
   xHandleQueueAEC = xQueueCreate( AEC_QUEUE_LENGTH, sizeof(struct xQ_Packet) );
   if(xHandleQueueAEC == 0)
   {
      return pdFAIL;
   }
   vQueueAddToRegistry(xHandleQueueAEC, "Queue_AEC");
   /* Create AEC Task */
   xTaskCreate (aecTaskHandler, "AECTaskHandler", STACK_SIZE_ALLOC(STACK_SIZE_TASK_AEC), &ucParameterToPass, PRIORITY_TASK_AEC, &xHandleTaskAEC);
   configASSERT(xHandleTaskAEC );
   return pdPASS;
}

void cb_notify_i2sRx_disable_intr_from_FPGA_ISR(void)
{
   static struct xQ_Packet i2s_packet;
   i2s_packet.ucCommand = eCMD_AUDIO_I2S_DATA_STOP;
   i2s_packet.ucSrc = 6;
   
   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

   if(xQueueSendFromISR( xHandleQueueAEC, &i2s_packet, &xHigherPriorityTaskWoken )  != pdPASS ) 
   {
     printf("Error Sending AEC Disable message \n");
   }

   
   portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
   
}
void cb_notify_i2sRx_enable_intr_from_FPGA_ISR(void)
{
   static struct xQ_Packet i2s_packet2;
   i2s_packet2.ucCommand = eCMD_AUDIO_I2S_DATA_START;
   i2s_packet2.ucSrc = 6;
   
   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

   if(xQueueSendFromISR( xHandleQueueAEC, &i2s_packet2, &xHigherPriorityTaskWoken ) != pdPASS ) 
   {
     printf("Error Sending AEC Enable message \n");
   }

   portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
   
}

#if 0
void cb_notify_i2sRx_disable_intr_from_FPGA(void)
{
   struct xQ_Packet i2s_packet;
   i2s_packet.ucCommand = eCMD_AUDIO_I2S_DATA_STOP;
   i2s_packet.ucSrc = 6;
   if( xQueueSend(xHandleQueueAEC, &i2s_packet, 0) != pdPASS )
   {
      //printf("%s : xQueueSendFromISR Fail  \r\n", __func__);
   }
}
#endif
void cb_notify_i2sRx_intr_from_FPGA(void)
{
   struct xQ_Packet i2s_packet;
   /* Disable the IRQ to stop getting interrupts from the I2S CLK */
   i2s_packet.ucCommand = eCMD_AUDIO_I2S_DATA_START;
   i2s_packet.ucSrc = 6;
   if( xQueueSend(xHandleQueueAEC, &i2s_packet, 0 ) != pdPASS )
   {
      //printf("%s : xQueueSendFromISR Fail  \r\n", __func__);
   }
}

void service_i2s_intr_from_host(void)
{
  cb_notify_i2sRx_enable_intr_from_FPGA_ISR();
}

#endif //#ifdef AEC_ENABLED
