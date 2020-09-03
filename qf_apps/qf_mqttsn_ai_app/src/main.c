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
 *    File   : main.c
 *    Purpose: main for QuickFeather helloworldsw and LED/UserButton test
 *
 *=========================================================*/

#include "Fw_global_config.h"   // This defines application specific charactersitics

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "RtosTask.h"

/*    Include the generic headers required for QORC */
#include "eoss3_hal_gpio.h"
#include "eoss3_hal_rtc.h"
#include "eoss3_hal_fpga_usbserial.h"
#include "ql_time.h"
#include "s3x_clock_hal.h"
#include "s3x_clock.h"
#include "s3x_pi.h"
#include "dbg_uart.h"
#include "cli.h"

#include "fpga_loader.h"    // API for loading FPGA
#include "gateware.h"           // FPGA bitstream to load into FPGA
#include "sensor_config.h"
#include "datablk_mgr.h"
#include "process_ids.h"
#include "datablk_processor.h"
#include "process_imu.h"
#include "mc3635.h"
extern const struct cli_cmd_entry my_main_menu[];

#if DBG_FLAGS_ENABLE
uint32_t DBG_flags = DBG_flags_default;
#endif

const char *SOFTWARE_VERSION_STR;

uint8_t DeviceClassUUID[UUID_TOTAL_BYTES] =
{ 0x4d, 0x36, 0xe9, 0x78,
  0xe3, 0x25, 0x11, 0xce,
  0xbf, 0xc1, 0x08, 0x00,
  0x2b, 0xe1, 0x03, 0x18
};

char *pSupportedPaths[NUM_SUPPORTED_PATHS] = {"/default/"};

/*
 * Global variable definition
 */


extern void qf_hardwareSetup();
static void nvic_init(void);
#define IMU_BLOCK_PROCESSOR (1)
#if (IMU_BLOCK_PROCESSOR == 1)
/*========== BEGIN: IMU SENSOR Datablock processor definitions =============*/
/** @addtogroup QAI_IMU_PIPELINE_EXAMPLE QuickAI SDK IMU pipeline example
 *
 *  @brief IMU (Inertial measurement unit) pipeline example code
 *
 * This example code demonstrates setting up IMU Queues,
 * setting up the datablock buffer manager (\ref DATABLK_MGR)
 * and setting up the datablock processor processing elements (\ref DATABLK_PE).
 * A specific IMU processing element for motion detection is provided in this
 * example.
 *
 * @{
 */

#define IMU_MAX_BATCH_SIZE (18)      ///< Approximately 10ms @1660Hz
#define IMU_FRAME_SIZE_PER_CHANNEL       (IMU_MAX_BATCH_SIZE)
#define IMU_NUM_CHANNELS      (6)    ///< (X,Y,Z) for Accel and (X,Y,Z) for Gryo
#define IMU_FRAME_SIZE      ( (IMU_FRAME_SIZE_PER_CHANNEL) * (IMU_NUM_CHANNELS) )
#define NUM_IMU_DATA_BLOCKS ( 20)    ///< 100 data blocks approximately 1 sec of data

/** Maximum number of imu data blocks that may be queued for chain processing */
#define MAX_IMU_DATA_BLOCKS                  (NUM_IMU_DATA_BLOCKS)

/** maximum number of vertical (parallel processing elements) that may generate datablock outputs
 *  that may add to the front of the queue.
 *
 *  Queue size of a given datablock processor must be atleast
 *  summation of maximum datablocks of all sensors registered for
 *  processing with some room to handle the vertical depth
 */
#define MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS  (5)

#define DBP_IMU_THREAD_Q_SIZE   (MAX_IMU_DATA_BLOCKS+MAX_THREAD_VERTICAL_DEPTH_DATA_BLOCKS)
#define DBP_IMU_THREAD_PRIORITY (10)

typedef struct {
  QAI_DataBlockHeader_t dbHeader;
  int16_t imu_data[IMU_FRAME_SIZE];
} QAI_IMU_DataBlock_t ;

QAI_IMU_DataBlock_t imu_data_blocks[NUM_IMU_DATA_BLOCKS] ;
QAI_DataBlockMgr_t imuBuffDataBlkMgr;
QueueHandle_t  dbp_imu_thread_q;

/* IMU AI processing element functions */
datablk_pe_funcs_t imu_sensiml_ai_funcs = {NULL, imu_ai_data_processor, NULL, NULL, (void *)NULL } ;

// outQ processor for IMU block processor
outQ_processor_t imu_sensiml_ai_outq_processor =
{
  .process_func = NULL,
  .p_dbm = &imuBuffDataBlkMgr,
  .in_pid = IMU_SENSIML_AI_PID,
  .outQ_num = 0,
  .outQ = NULL,
  .p_event_notifier = NULL
};

datablk_pe_descriptor_t  datablk_pe_descr_imu[] =
{ // { IN_ID, OUT_ID, ACTIVE, fSupplyOut, fReleaseIn, outQ, &pe_function_pointers, bypass_function, pe_semaphore }

    /* processing element descriptor for SensiML AI for IMU sensor */
    { IMU_ISR_PID, IMU_SENSIML_AI_PID, true, false, true, &imu_sensiml_ai_outq_processor, &imu_sensiml_ai_funcs, NULL, NULL},
};

datablk_processor_params_t datablk_processor_params_imu[] = {
    { DBP_IMU_THREAD_PRIORITY,
      &dbp_imu_thread_q,
      sizeof(datablk_pe_descr_imu)/sizeof(datablk_pe_descr_imu[0]),
      datablk_pe_descr_imu,
      256,
      "DBP_IMU_THREAD",
      NULL
    }
};

/* IMU sensors data reading function */
#define IMU_ISR_OUTQS_NUM (1)
QueueHandle_t *imu_isr_outQs[IMU_ISR_OUTQS_NUM] = { &dbp_imu_thread_q };
extern void imu_sensordata_read_callback(void);
extern void imu_event_notifier(int pid, int event_type, void *p_event_data, int num_data_bytes);
extern void set_first_imu_data_block(void);

outQ_processor_t imu_isr_outq_processor =
{
  .process_func = imu_sensordata_read_callback,
  .p_dbm = &imuBuffDataBlkMgr,
  .in_pid = IMU_ISR_PID,
  .outQ_num = 1,
  .outQ = imu_isr_outQs,
  .p_event_notifier = imu_event_notifier
};

void imu_block_processor(void)
{
  /** IMU datablock processor thread : Create IMU Queues */
  dbp_imu_thread_q = xQueueCreate(DBP_IMU_THREAD_Q_SIZE, sizeof(QAI_DataBlock_t *));
  vQueueAddToRegistry( dbp_imu_thread_q, "IMUPipelineExampleQ" );

  /** IMU datablock processor thread : Setup IMU Thread Handler Processing Elements */
  datablk_processor_task_setup(&datablk_processor_params_imu[0]);

  /** Set the first data block for the ISR or callback function */
  set_first_imu_data_block();
  /* [TBD]: sensor configuration : should this be here or after scheduler starts? */
  //sensor_imu_configure();
}

// temporary buffer to read sensor data from the FIFO
// Note: Needed because FIFO stores Gyro samples first and then Accel samples,
// where as the IMU processor needs Accel samples first and then Gyro samples.
//int16_t fifo_samples[IMU_FRAME_SIZE+6];
int imu_get_max_datablock_size(void)
{
  return IMU_MAX_BATCH_SIZE;
}

TimerHandle_t sensorTimId ;
void dataTimer_Callback(TimerHandle_t hdl)
{
  // Warning: must not call vTaskDelay(), vTaskDelayUntil(), or specify a non zero
  // block time when accessing a queue or a semaphore.
  imu_sensordata_read_callback(); //osSemaphoreRelease(readDataSem_id);
}

static int mc3635_threshold_count = 1;
void mc3635_set_threshold_count(int count)
{
  mc3635_threshold_count = count;
}

int mc3635_get_threshold_count(void)
{
  return mc3635_threshold_count;
}

extern void imu_batch_size_set(int batch_size);

void dataTimerStart(void)
{
  BaseType_t status;
#if (USE_IMU_FIFO_MODE)
    int milli_secs = 17; // reads fifo @17milli-secs intervals
#else
    int milli_secs = 2; // reads when a sample is available (upto 416Hz)
#endif
  // Create periodic timer
  //exec = 1;
  if (!sensorTimId) {
    sensorTimId = xTimerCreate("SensorTimer", pdMS_TO_TICKS(milli_secs), pdTRUE, (void *)0, dataTimer_Callback);
    configASSERT(sensorTimId != NULL);
  }

  if (sensorTimId)  {
    status = xTimerStart (sensorTimId, 0);  // start timer
    if (status != pdPASS)  {
      // Timer could not be started
    }
  }
  //set_first_imu_data_block();
#if (USE_IMU_FIFO_MODE)
  int num_samples;
  int srate = imu_get_accel_odr();
  if (srate > 29) {
    num_samples = srate / 50;
  } else {
    num_samples = 1;
  }
  mc3635_fifo_reset();
  mc3635_fifo_enable_threshold(num_samples);
  mc3635_set_threshold_count(num_samples);
  imu_batch_size_set(num_samples);
  vTaskDelay(2); // delay for 2ms to accumulate atleast 1 threshold
  //enable_lsm6dsm_stream();
#endif
  //mc3635_interrupt_enable();
  mc3635_set_mode(MC3635_MODE_CWAKE);
}

void dataTimerStop(void)
{
  if (sensorTimId) {
    xTimerStop(sensorTimId, 0);
  }
#if (USE_IMU_FIFO_MODE)
  //disable_lsm6dsm_stream();
#endif
  mc3635_interrupt_disable();
}
#endif /* IMU_BLOCK_PROCESSOR */

struct st_dbm_init {
  QAI_DataBlockMgr_t *pdatablk_mgr_handle;
  void  *pmem;
  int mem_size;
  int item_count;
  int item_size_bytes;
} ;

struct st_dbm_init dbm_init_table[] =
{
  {&imuBuffDataBlkMgr,   (void *)  imu_data_blocks, sizeof(  imu_data_blocks), IMU_FRAME_SIZE, sizeof(int16_t)},
//  {&audioBuffDataBlkMgr, (void *)audio_data_blocks, sizeof(audio_data_blocks), DBP_AUDIO_FRAME_SIZE, sizeof(int16_t)},
};

void init_all_datablock_managers(struct st_dbm_init *p_dbm_table, int len_dbm_table)
{
  /** Setup the data block managers */
  for (int k = 0; k < len_dbm_table; k++) {
      datablk_mgr_init( p_dbm_table[k].pdatablk_mgr_handle,
                        p_dbm_table[k].pmem,
                        p_dbm_table[k].mem_size,
                        p_dbm_table[k].item_count,
                        p_dbm_table[k].item_size_bytes
                      );
  }

}

void setup_sensors_data_block_processor(void)
{
  /** Initialize all datablock managers */
  init_all_datablock_managers(dbm_init_table, sizeof(dbm_init_table)/sizeof(struct st_dbm_init));
#if (AUDIO_DRIVER == 1)
  audio_block_processor();
#endif
  imu_block_processor();

#if (AD7476_FPGA_DRIVER == 1)  
  ad7476_block_processor();
#endif
}

int main(void)
{
    //SOFTWARE_VERSION_STR = "qorc-sdk/qf_apps/qf_mqttsn_ai_app";
#if S3AI_FIRMWARE_IS_COLLECTION
    SOFTWARE_VERSION_STR = "C Jun-2020";
#endif
#if S3AI_FIRMWARE_IS_RECOGNITION
    SOFTWARE_VERSION_STR = "R Jun-2020";
#endif

    qf_hardwareSetup();
    nvic_init();
    S3x_Clk_Disable(S3X_FB_21_CLK);
    S3x_Clk_Disable(S3X_FB_16_CLK);
    S3x_Clk_Enable(S3X_A1_CLK);
    S3x_Clk_Enable(S3X_CFG_DMA_A1_CLK);
    load_fpga(axFPGABitStream_length,axFPGABitStream);
#if (FEATURE_USBSERIAL == 1)
    // Use 0x6141 as the USB serial product ID (USB PID)
    HAL_usbserial_init2(false, true, 0x6141);   // Start USB serial not using interrupts, Use 72MHz clock
#endif
    for (int i = 0; i != 4000000; i++) ;   // Give it time to enumerate
    HAL_Delay_Init();

    dbg_str("\n\n");
    dbg_str( "##########################\n");
    dbg_str( "Quicklogic QuickFeather MQTT-SN/SensiML Interface Example\n");
    dbg_str( "SW Version: ");
    dbg_str( SOFTWARE_VERSION_STR );
    dbg_str( "\n" );
    dbg_str( __DATE__ " " __TIME__ "\n" );
    dbg_str( "##########################\n\n");

	dbg_str( "\n\nHello world!!\n\n");	// <<<<<<<<<<<<<<<<<<<<<  Change me!

    // Initialize mCube MC3635 Accelerometer sensor device
    mc3635_init();


    CLI_start_task( my_main_menu );

    setup_sensors_data_block_processor();
    sensor_set_virtual_sensor(IMU_V_SENSOR_NO);

    StartRtosTaskMqttsnApp();
    StartRtosTaskMqttsnMsgHandler();
#if S3AI_FIRMWARE_IS_RECOGNITION
    StartRtosTaskRecognition();
#endif
    //StartRtosTaskADC();
    xTaskSet_uSecCount(1546300800ULL * 1000ULL * 1000ULL); // start at 2019-01-01 00:00:00 UTC time

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    dbg_str("\n");

    while(1);
}

static void nvic_init(void)
 {
    // To initialize system, this interrupt should be triggered at main.
    // So, we will set its priority just before calling vTaskStartScheduler(), not the time of enabling each irq.
    NVIC_SetPriority(Ffe0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(SpiMs_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(CfgDma_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(Uart_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(FbMsg_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
 }

//needed for startup_EOSS3b.s asm file
void SystemInit(void)
{

}

//missing functions for S3 project
void wait_ffe_fpga_load(void){ return; };
