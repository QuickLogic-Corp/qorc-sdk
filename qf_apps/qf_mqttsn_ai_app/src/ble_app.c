/*==========================================================
 *
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : ble_app.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

/*
 * All things BLE at the Application level for S3AI
 */

#include "Fw_global_config.h"
#include "iop_messages.h"
//#include "ble_iop_messages.h"
#include "DataCollection.h"
//#include "dcl_commands.h"
#include "dbg_uart.h"
#include "sensor_live.h"
#include "Mqttsn_MessageHandler.h"

extern void sensor_live_stop_only(void);

/* we collect IMU data here before sending data to the BLE interface
 * Only because historically we always sent IMU data as a pair of sensor valus.
 */

 
struct ble_data_config ble_data_config;
#if 1 //this is for MQTTSN app

//from Mqttsn_App.c
extern void my_ble_callback( uint8_t *pData );

uint8_t *my2_ble_callback( uint8_t *pData )
{
  //call the MQTTSN function to process the data
  my_ble_callback( pData );
  
  //the response is sent in my_ble_send() function
  return ((void *) NULL);
}
#else //this was for origninal SensorTile app
uint8_t *my2_ble_callback( uint8_t *pData )
{
  /* BLE has sent us a new command */
  /* wack the old command */
  memset( (void *)(&iop_globals.cmd_from_host), 0, sizeof(iop_globals.cmd_from_host) );
    
  /* extract len & cmd */
  iop_globals.cmd_from_host.len = pData[0];
  iop_globals.cmd_from_host.cmd = pData[1];
  
  /* payload starts at data+2 */
  memcpy( iop_globals.cmd_from_host.payload, pData+2, iop_globals.cmd_from_host.len-2 );
  /* process the command */
  iop_process_command();
  return ((void *)(&iop_globals.rsp_to_host));
}
#endif 
    
extern int ble_motion_drop;
// This is generic sensor processing, it should also handle ADC or other sensor.
void ble_send( const struct sensor_data *pInfo )
{
    Mqttsn_MsgData_t *pMsg;
    Mqttsn_TopicInfo_t *pTpcInfo;
    Sensor_live_t *sensor;
    Mqttsn_TxPublishErrorCodes_t publishErr;
    uint32_t result;

    // Check if TOPIC_LIVE_RAW_DATA is registered
    pTpcInfo = Mqttsn_GetTopicInfo(TOPIC_LIVE_RAW_DATA);
    if( !((pTpcInfo) && (pTpcInfo->status)) )
        return;

    // Check if the sensor's live streaming is enabled
    sensor = sensor_live_get_by_id(pInfo->sensor_id);
    if (!( sensor &&
           sensor->live &&
           (iop_globals.cur_status.bit_flags & LIVESTREAM_ACTIVE))
       )
    {
        if ( (sensor) && SENSOR_LIVE_GET_ALLOC(sensor) )
        {
            pMsg = &sensor->msg;

            // We have left over in buffer which has not been sent.
            Mqttsn_FreePublishPayload(&pMsg->pMsgPayldBuf, pMsg->allocLen);
            // Now we can reset live stream data struct for the sensor
            SENSOR_LIVE_SET_ALLOC(sensor,0);
            sensor_live_stop_only();
        }
        return;
    }
#define USE_VARIABLE_SAMPLES_PER_PACKET (1)
#if (USE_VARIABLE_SAMPLES_PER_PACKET == 1)
#define MQTTSN_LIVE_BUFFER_SIZE      (MQTTSN_BUFFER_LARGE)
    // set samples-per-packet based on the live data rate
    // if (rate > 50Hz)
    //     samples-per-packet-per-sensor-id = rate / 50
    // else
    //     samples-per-packet-per-sensor-id = 1
    if (pInfo->rate_hz < 450)
       SENSOR_LIVE_SET_RELOAD(sensor, 0); // Force sending every sample
    int live_rate_hz = (pInfo->rate_hz) / (SENSOR_LIVE_GET_RELOAD(sensor) + 1);
    int spp = (live_rate_hz > 50) ? (live_rate_hz / 50) : 1;
    // Limit samples-per-packet to maximum 8-bit integer
    // For Audio limit to 15-ms or 240 samples @16kHz
    if (spp > 240)
       spp = 240;

    SENSOR_LIVE_SET_MIN(sensor, spp);
#else
#define MQTTSN_LIVE_BUFFER_SIZE      (MQTTSN_BUFFER_SMALL)
#endif
    // Check if there is ongoing sending
    if( send_data_start() < 0 ) {
        ble_motion_drop += 1;
        return;
    }

    pMsg = &sensor->msg;
    pMsg->topicId = TOPIC_LIVE_RAW_DATA;

    do {
        uint32_t sp;
        uint8_t *next,*pSrc;

        pSrc = (uint8_t *)pInfo->vpData;

        // Loop through number of samples
        for (int i=0; i<(pInfo->n_bytes/SENSOR_LIVE_GET_SIZE(sensor)); i++)
        {
           // Check down count for imu sensor
           if( 0 == SENSOR_LIVE_GET_RELOAD(sensor) ) {
               /* always send */
           } else {
              if( SENSOR_LIVE_GET_COUNT(sensor) > 0 ) {
                 SENSOR_LIVE_DEC_COUNT(sensor);
                 // Advance pInfo->pvData to skip this sample.
                 pSrc += SENSOR_LIVE_GET_SIZE(sensor);
                 // iop_globals.data_busy is set by send_data_start() at beginning of this function
                 // reset it is needed as for-loop may exit without new samples to process!
                 send_data_end();
                 continue;
              } else {
                 SENSOR_LIVE_SET_COUNT(sensor, SENSOR_LIVE_GET_RELOAD(sensor));
              }
           }

           // pSrc now points to the sample we need to process

           // Check if new buffer needs to be allocated or working on unfinished one?
           if ( SENSOR_LIVE_GET_CUR(sensor) == 0 )
           {
              uint8_t *pPayld;

              result = Mqttsn_AllocPublishPayload( MQTTSN_BUFFER_LARGE, &(pMsg->pMsgPayldBuf),
                                    &(pMsg->allocLen) );
              if (result)
              {
                 pPayld = pMsg->pMsgPayldBuf;
                 // Only write sensor_id and sequence number for new allocated buffer
                 pMsg->payldLen  = Mqttsn_BuffWr_u32(&pPayld, pInfo->sensor_id);
                 pMsg->payldLen += Mqttsn_BuffWr_u8(&pPayld, SENSOR_LIVE_GET_SEQ(sensor));
                 SENSOR_LIVE_INC_SEQ(sensor);
                 SENSOR_LIVE_SET_ALLOC(sensor,1);
              }
              else
              {
                 configASSERT(0);
                 return;
              }
           }

           // Add this sample to mqttsn buffer
           // Advance the pointer in buffer to where we want to append the new samples
           sp = SENSOR_LIVE_GET_SIZE(sensor) * SENSOR_LIVE_GET_CUR(sensor) + 5; // 4(sensor_id) + 1(seq)
           next = pMsg->pMsgPayldBuf + sp;

           // Append new sample.
           memcpy(next, pSrc, SENSOR_LIVE_GET_SIZE(sensor));
           // Advance pInfo->pvData to next sample in source.
           pSrc += SENSOR_LIVE_GET_SIZE(sensor);
           pMsg->payldLen += SENSOR_LIVE_GET_SIZE(sensor);

           // update cur
           SENSOR_LIVE_INC_CUR(sensor);

           sp = SENSOR_LIVE_GET_SIZE(sensor) * SENSOR_LIVE_GET_CUR(sensor) + 5; // 4(sensor_id) + 1(seq)
           // available payload space in buffer
           sp = pMsg->allocLen - sp;

           // Check if buffer has space for next sample
           if ( (sp < SENSOR_LIVE_GET_SIZE(sensor)) || 
                (SENSOR_LIVE_GET_CUR(sensor) >= SENSOR_LIVE_GET_MIN(sensor)) )
           {
               //buffer is full, send it.
               publishErr = Mqttsn_SendPublish(pMsg, pTpcInfo);
               if (publishErr != MQTTSN_TX_PUBLISH_NO_ERROR)
               {
                   set_sys_error( SYS_ERR_EINVAL, publishErr);
               }
               SENSOR_LIVE_SET_CUR(sensor, 0);
               // The allocated buffer is copied to queue, or has been released (if publish failed)
               SENSOR_LIVE_SET_ALLOC(sensor,0);
           }
        }

        // reset flag
        send_data_end();
    } while(0)
        ;
}
