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

#include "Fw_global_config.h"

#include "dcl_commands.h"
#include "iop_messages.h"
#include "cli.h"
#include "string.h"
#include "sensor_cmds.h"
#include "DataCollection.h"
#include "Mqttsn_MessageHandler.h"
#include "Mqttsn_Topics.h"
#include "Mqttsn_App.h"
#include "Mqttsn_CliCmds.h"
#include "sensor_live.h"

void AddMqttsn(uint8_t flags, uint16_t topicId, uint16_t msgId)
{
    Mqttsn_TopicInfo_t *pTopic;
    
    cli_cmd_buff_wr_u8(MQTTSN_PUBLISH);
    cli_cmd_buff_wr_u8(flags);
    
    pTopic = Mqttsn_GetTopicInfo(topicId);
    cli_cmd_buff_wr_u16(pTopic->assgndID.data.id);
    
    cli_cmd_buff_wr_u16(msgId);
}

/* we have constructed the packet
 * our next step is to pretend that we have received the SPI packet.
 */
void do_dispatch(uint32_t numBytes)
{
  CLI_ProcessDataIn(raw_bytes, numBytes);
}

/* configure sensors */
static void do_configure( const struct cli_cmd_entry *pEntry )
{
    (void)pEntry;
    uint8_t flags;
    uint16_t msgId;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    AddMqttsn(flags, TOPIC_SENSOR_DONE, msgId);
    
    do_dispatch(7);
}

static void do_all_stop_cli( const struct cli_cmd_entry *pEntry )
{
    (void)(pEntry);
    uint8_t flags;
    uint16_t msgId;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags, TOPIC_SYS_ALL_STOP, msgId);
    
    do_dispatch(7);
}

static void do_get_uuids( const struct cli_cmd_entry *pEntry )
{
    (void)(pEntry);
    uint8_t flags;
    uint16_t msgId;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags, TOPIC_SYS_DEVICE_UUIDS_REQ, msgId);
    
    do_dispatch(7);
}

/* version command */
static void do_version( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SYS_VERSION_REQ, msgId);
  
  do_dispatch(7);
}

static void do_status_clear( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SYS_STATUS_CLR, msgId);
  
  do_dispatch(7);
}

/* request model GUID */
static void do_model_guid( const struct cli_cmd_entry *pEntry )
{
  (void)pEntry;
  uint8_t flags;
  uint16_t msgId;
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_RECOG_MODEL_UUID_REQ, msgId);
  
  do_dispatch(7);
}

/* request compile string */
static void do_compilestamp( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SYS_COMPDATETIME_REQ, msgId);

  do_dispatch(7);
}

/* sensor clear */
static void do_clear( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SENSOR_CLEAR, msgId);
  
  do_dispatch(7);
}

/* list sensors */
static void do_list( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SENSOR_LIST_REQ, msgId);

  do_dispatch(7);
}

/* request and print status */
static void do_status( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SYS_STATUS_REQ, msgId);
  
  CLI_printf("Status: bytes-saved:  %d\n", iop_globals.cur_status.bytes_saved);
  CLI_printf("Status:        bits:  %d\n", iop_globals.cur_status.bit_flags);
  CLI_printf("Status:  collect-oe:  %d\n", iop_globals.cur_status.collect_oe_count);
  CLI_printf("Status:     live-oe:  %d\n", iop_globals.cur_status.live_oe_count);
  CLI_printf("Status:    tx_count:  %d\n", iop_globals.cur_status.tx_count);
  CLI_printf("Status:    rx_count:  %d\n", iop_globals.cur_status.rx_count);
  CLI_printf("Status: errs: %d error: %d\n", iop_globals.cur_status.error_count, iop_globals.cur_status.sticky_error_code );
  do_dispatch(7);
}


/* common code for all sensor add commands */
static void do_sensor_add_common( uint32_t sensor_id )
{
  uint32_t rate;
  uint8_t flags;
  uint16_t msgId;
  
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SENSOR_ADD, msgId);
  
  cli_cmd_buff_wr_u32( sensor_id );
  CLI_uint32_required( "rate_hz", &rate );
  cli_cmd_buff_wr_u32( rate );
}

/* common for all imu sensors */
static void do_imu_common( uint32_t sensor_id )
{
  uint8_t range, len=16;
  /*  imu sensors A or G have this form:
   *  command RATE RANGE
   *  A+G has this form:
   *  command RATE RANGE-A RANGE-G
   */
  do_sensor_add_common( sensor_id );
  switch (sensor_id)
  {
     case SENSOR_ENG_VALUE_ACCEL:
         CLI_uint8_required( "rangeA", &range );
         cli_cmd_buff_wr_u8( range );
         break;
     case SENSOR_ENG_VALUE_GYRO:
         CLI_uint8_required( "rangeG", &range );
         cli_cmd_buff_wr_u8( range );
         break;
     case SENSOR_ENG_VALUE_ACCEL_GYRO:
         CLI_uint8_required( "rangeA", &range );
         cli_cmd_buff_wr_u8( range );
         CLI_uint8_required( "rangeG", &range );
         cli_cmd_buff_wr_u8( range );
         len++;
         break;
  }
  do_dispatch(len);
}

/* handle ACCEL_GYRO */
static void do_imu_accel_gyro( const struct cli_cmd_entry *pEntry )
{
  /* handle accel enable/disable */
  do_imu_common( SENSOR_ENG_VALUE_ACCEL_GYRO );
}

/* handle ACCEL */
static void do_imu_accel( const struct cli_cmd_entry *pEntry )
{
  /* handle accel enable/disable */
  do_imu_common( SENSOR_ENG_VALUE_ACCEL );
}


/* handle GYRO */
static void do_imu_gyro( const struct cli_cmd_entry *pEntry )
{
  /* handle gyro enable/disable */
  do_imu_common( SENSOR_ENG_VALUE_GYRO );
}

/* handle MAG sensor */
static void do_imu_mag( const struct cli_cmd_entry *pEntry )
{
  /* handle magometer enable/disable */
  do_imu_common( SENSOR_ENG_VALUE_MAGNETOMETER );
}

/* IMU sub commands */
static const struct cli_cmd_entry imu_cmds[] =
  {
   CLI_CMD_SIMPLE( "acgy", do_imu_accel_gyro, "usage: rate rangeA rangeG"),
   CLI_CMD_SIMPLE( "accel", do_imu_accel, "usage: rate rangeA"),
   CLI_CMD_SIMPLE( "gyro", do_imu_gyro, "usage: rate rangeG"),
   CLI_CMD_SIMPLE( "mag", do_imu_mag, "usage: rate rangeM"),
   CLI_CMD_TERMINATE()
  };

#if LTC1859_DRIVER
/* ADC command */
static void do_ltc1859a( const struct cli_cmd_entry *pEntry )
{
  /* handle ltc1859a adc chip */
  (void)(pEntry);
  uint8_t ch_cfg;
  /*
  * usage:  COMMAND  RATE   CH0_CFG CH1_CFG CH2_CFG CH3_CHG
  *
  * Use 0xFF, or -1 to disable channel.
  */
  
  do_sensor_add_common(SENSOR_ADC_LTC_1859_MAYHEW);
  CLI_uint8_required( "chnl0-cfg", &ch_cfg );
  cli_cmd_buff_wr_u8( ch_cfg );
  CLI_uint8_required( "chnl1-cfg", &ch_cfg );
  cli_cmd_buff_wr_u8( ch_cfg );
  CLI_uint8_required( "chnl2-cfg", &ch_cfg );
  cli_cmd_buff_wr_u8( ch_cfg );
  CLI_uint8_required( "chnl3-cfg", &ch_cfg );
  cli_cmd_buff_wr_u8( ch_cfg );
  do_dispatch(19);
}
#endif //LTC1859_DRIVER

/* audio command */
static void do_audio( const struct cli_cmd_entry *pEntry )
{
  uint8_t v;
  /* usage: COMMAND RATE  NBITS MIC0 MIC1 MIC2 MIC3 MIC4 MIC5 MIC6 MIC7 */
  do_sensor_add_common(SENSOR_AUDIO );

  CLI_uint8_required( "nbits", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic0-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic1-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic2-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic3-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic4-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic5-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic6-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  CLI_uint8_required( "mic7-cfg", &v );
  cli_cmd_buff_wr_u8( v );
  do_dispatch(24);
}

/* sensor types, add more sensors here */
static const struct cli_cmd_entry sensor_add_cmds[] =
  {
   CLI_CMD_SUBMENU( "imu", imu_cmds, "cofigure IMU sensors"),
#if LTC1859_DRIVER 
   CLI_CMD_SIMPLE( "ltc1859a", do_ltc1859a, "configure ltc1859(a)"),
#endif
   CLI_CMD_SIMPLE( "audio", do_audio, "configure audio capture"),
   CLI_CMD_TERMINATE()
  };

/* set clock */
static void do_datetime( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint32_t v;
  uint8_t flags;
  uint16_t msgId;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SYS_UNIXTIME_SET, msgId);
  
  CLI_uint32_required( "datetime-seconds", &v);
  cli_cmd_buff_wr_u32( v );
  do_dispatch(11);
}

/* start saving */
static void do_start( const struct cli_cmd_entry *pEntry )
{
  (void)pEntry;
  uint8_t flags;
  uint16_t msgId;
  char *cp;
  uint32_t numBytes = 0;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_COLLECT_START, msgId);
  
  /* technically, GUIDs are binary data but we are lazy for testing
   * and only allow the test process to enter ascii text 
   */
  CLI_string_ptr_required( "guid", &cp );
  while( *cp ){
      numBytes++;
    cli_cmd_buff_wr_u8( *cp );
    cp++;
  }
  
  do_dispatch(7+numBytes);
}

/* stop saving */
static void do_stop( const struct cli_cmd_entry *pEntry )
{
  (void)pEntry;
  uint8_t flags;
  uint16_t msgId;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_COLLECT_STOP, msgId);

  do_dispatch(7);
}

static void do_livestream_sensor_list_req( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  cli_cmd_buff_reset();

  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_LIVE_SENSOR_LIST_REQ, msgId);

  do_dispatch(7);
}

/**
 * @brief read payload from cli and fill it into raw_bytes[]
 * @note  cli_cmd_buf_reset() should be called before this function is called.
 *        It means that MQTTSN packet header has been filled and index of raw_bytes[],
 *        raw_bytes[0] is pointing to the 1st byte after head in raw_bytes[]. After calling
 *        this function, raw_bytes[0] (length of MQTTSN packet) is adjusted to include payload.
 * @return : payload length = n_sensor * (4(sensor-id) + 4(rate)) + 1(number of valid sensors) 
 */
static uint32_t do_livestream_set_rate_get_payload()
{
    uint8_t sensor_n,n,sensor_c=0;
    uint32_t sensor_id,rate;
    Sensor_live_t *sensor;

    // get num of sensors
    CLI_uint8_required( "MQTTSN n_sensor", &sensor_n );

    if (sensor_n == 0)
    {
        CLI_printf("Live-Set-Rate-Req : empty input!\n");
        return 0;
    }
    if (sensor_n>SENSOR_NUM_MAX)
    {
        // Strip off sensors which are not defined
        sensor_n = SENSOR_NUM_MAX;
        CLI_printf("Live-Set-Rate-Req : number of sensors exceeds allowed %d !\n",SENSOR_NUM_MAX);
    }

    n = sensor_n;
    cli_cmd_buff_wr_u8(sensor_n);

    do {
        n--;
        CLI_uint32_required( "MQTTSN sensor_id", &sensor_id );
        sensor = sensor_live_get_by_id(sensor_id);

        if (sensor)
        {
            // set sensor_id only it is found in predefined sensor list.
            cli_cmd_buff_wr_u32(sensor_id);
            // read & set streaming rate, add to cli buffer
            CLI_uint32_required("MQTTSN rate", &rate);
            cli_cmd_buff_wr_u32(rate);

            // postpong writing rate into predefined sensor list, until we prepare RSP.

            sensor_c++;
        }
        else {
            CLI_printf("sensore Id: 0x%X is invalid !\n",sensor_id);
            // read & flush rate for this sensor
            CLI_uint32_required("MQTTSN rate", &rate);
       }
    } while (n !=0);

    if (sensor_n != sensor_c)
    {
        // The actual number of valid sensors(sensor_c) is less than claimed(sensor_n)
        // we need to adjust number of sensors in payload, which is the 1st byte in payload.
        set_value_in_raw_bytes(7, sensor_c);
    }

    return (sensor_c * 8 + 1);
}

/* turn on BLE imu data */
static void do_livestream_set_rate_req( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  uint32_t pld_len;

  cli_cmd_buff_reset();

  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_LIVE_SET_RATE_REQ, msgId);

  if (pld_len = do_livestream_set_rate_get_payload())
      do_dispatch(7+pld_len);
}

/**
 * @brief read payload from cli and fill it into raw_bytes[]
 * @note  cli_cmd_buf_reset() should be called before this function is called.
 *        It means that MQTTSN packet header has been filled and index of raw_bytes[],
 *        raw_bytes[0] is pointing to the 1st byte after head in raw_bytes[]. After calling
 *        this function, raw_bytes[0] (length of MQTTSN packet) is adjusted to include payload.
 * @return : payload length = n_sensor * (4(sensor-id) + 1(n_samples)) + 1(number of sensors)
 */
static uint32_t do_livestream_raw_get_payload()
{
    uint8_t sensor_n,n,sample,sensor_c=0;
    uint32_t sensor_id;
    Sensor_live_t *sensor;

    // get num of sensors
    CLI_uint8_required( "MQTTSN n_sensor", &sensor_n );
    if (sensor_n == 0)
    {
        CLI_printf("Live-Set-Rate-Req : empty input!\n");
        return 0;
    }
    if (sensor_n>SENSOR_NUM_MAX)
    {
        // Strip off sensors which are not defined
        sensor_n = SENSOR_NUM_MAX;
        CLI_printf("The number of sensors exceeds allowed %d !\n",SENSOR_NUM_MAX);
    }

    n = sensor_n;
    cli_cmd_buff_wr_u8(sensor_n);

    do {
        n--;
        CLI_uint32_required( "MQTTSN sensorID", &sensor_id );
        sensor = sensor_live_get_by_id(sensor_id);

        if (sensor)
        {
            cli_cmd_buff_wr_u32(sensor_id);

            // read min & add to cli buffer
            CLI_uint8_required("MQTTSN samples", &sample);
            cli_cmd_buff_wr_u8(sample);

            sensor_c++;
        }
        else {
            CLI_printf("sensore Id: 0x%X is invalid !\n");
            // read & flush sample for this sensor
            CLI_uint8_required("MQTTSN sample", (uint8_t*)&sensor_id);
       }
    } while (n !=0);

    if (sensor_n != sensor_c)
    {
        // The actual number of sensor(sensor_c) is less than claimed(sensor_n)
        // we need to adjust number of sensors in payload, which is the 1st byte in payload !
        set_value_in_raw_bytes(7, sensor_c);
    }

    return (sensor_c * 5 + 1);
}

/* As function name says */
static void do_livestream_start( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  uint32_t pld_len;

  cli_cmd_buff_reset();

  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_LIVE_START, msgId);

  if ( pld_len = do_livestream_raw_get_payload() )
      do_dispatch(7+pld_len);
}

/* turn off all live streaming data */
static void do_livestream_stop( const struct cli_cmd_entry *pEntry )
{
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_LIVE_STOP, msgId);

  do_dispatch(7);
}

/* select specific sensor by id */
static void do_ble_sensor_id( const struct cli_cmd_entry *pEntry )
{
  uint32_t id;
  uint8_t flags;
  uint16_t msgId;
  
  CLI_uint32_getshow( "sensor-id", &id );

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SENSOR_SELECT_BY_ID, msgId);
  
  cli_cmd_buff_wr_u32( id );
  cli_cmd_buff_wr_u32( 0xFFFFFFFF ); /* all sub-channels */
  do_dispatch(15);
}


static void do_ble_sensor_start( const struct cli_cmd_entry *pEntry )
{
  /* turn on selected sensor over ble */
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SENSOR_START, msgId);
  
  do_dispatch(7);
}

static void do_ble_sensor_stop( const struct cli_cmd_entry *pEntry )
{
  /* turn off selected sensor over ble */
  (void)(pEntry);
  uint8_t flags;
  uint16_t msgId;
  
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_SENSOR_STOP, msgId);  

  do_dispatch(7);
}

/* sub-command for selected sensor */
static struct cli_cmd_entry ble_sensor_menu[] = {
  CLI_CMD_SIMPLE( "sensor-id", do_ble_sensor_id, "select-ble-sensor" ),
  CLI_CMD_SIMPLE( "sensor-start", do_ble_sensor_start, "start-ble-sensor" ),
  CLI_CMD_SIMPLE( "sensor-stop", do_ble_sensor_stop, "stop-ble-sensor" ),
  CLI_CMD_TERMINATE()
};

/* command for data save filename */
static void do_filename_cmd( const struct cli_cmd_entry *pEntry )
{
  char *cp;
  uint8_t flags;
  uint16_t msgId;
  (void)(pEntry);
  uint32_t numBytes = 0;

  cli_cmd_buff_reset();
  
  CLI_uint8_required( "MQTTSN flags", &flags );
  CLI_uint16_required( "Msg Id", &msgId );

  AddMqttsn(flags, TOPIC_COLLECT_PREFIX_SET, msgId); 
  
  CLI_string_ptr_required(  "filename", &cp );
  while( *cp ){
      numBytes++;
    cli_cmd_buff_wr_u8( *cp );
    cp++;
  }
  cli_cmd_buff_wr_u8(0);
  do_dispatch(7+numBytes+1);
}

static void do_classification_start( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    uint8_t type;
    cli_cmd_buff_reset();

    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );

    AddMqttsn(flags,  TOPIC_RESULT_CLASS_START, msgId); 

    // get data type
    CLI_uint8_required( "MQTTSN type", &type );

    if ((type > 2) || (type ==0))
    {
        CLI_printf("Type : 1-result only, 2-result + FV\n");
        return;
    }
    cli_cmd_buff_wr_u8( type );

    do_dispatch(8);
}

static void do_classification_stop( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags,  TOPIC_RESULT_CLASS_STOP, msgId); 
    
    do_dispatch(7);
}

static void do_classification_set_rate( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    uint8_t count_down;
    cli_cmd_buff_reset();

    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    // get count down
    CLI_uint8_required( "MQTTSN countdown", &count_down );

    AddMqttsn(flags,  TOPIC_RESULT_CLASS_SET_RATE, msgId); 

    cli_cmd_buff_wr_u8( count_down );

    do_dispatch(8);
}

static void do_recog_start( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags,  TOPIC_RECOG_START, msgId); 
    
    do_dispatch(7);
}

static void do_recog_stop( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags,  TOPIC_RECOG_STOP, msgId); 
    
    do_dispatch(7);
}

static void do_storage_dir( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    uint32_t numBytes = 0;
    char *cp;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags,  TOPIC_STORAGE_DIR_REQ, msgId); 
    
    CLI_string_ptr_required( "path", &cp );
    while( *cp ){
        numBytes++;
        cli_cmd_buff_wr_u8( *cp );
        cp++;
    }
    cli_cmd_buff_wr_u8(0);
    
    do_dispatch(7+numBytes+1);
}

static void do_disk_space( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    uint32_t numBytes = 0;
    char *cp;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags,  TOPIC_STORAGE_SPACE_REQ, msgId); 
    
    CLI_string_ptr_required(  "storage area name", &cp );
    while( *cp ){
        numBytes++;
        cli_cmd_buff_wr_u8( *cp );
        cp++;
    }
    cli_cmd_buff_wr_u8(0);
    
    do_dispatch(7+numBytes+1);
}

static void do_delete_file( const struct cli_cmd_entry *pEntry)
{
    uint8_t flags;
    uint16_t msgId;
    uint32_t numBytes = 0;
    char *cp;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &flags );
    CLI_uint16_required( "Msg Id", &msgId );
    
    AddMqttsn(flags,  TOPIC_STORAGE_DEL, msgId); 
    
    CLI_string_ptr_required(  "filename", &cp );
    while( *cp ){
        numBytes++;
        cli_cmd_buff_wr_u8( *cp );
        cp++;
    }
    cli_cmd_buff_wr_u8(0);
    
    do_dispatch(7+numBytes+1);
}

static void do_get_start( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint32_t numBytes = 0;
    char *cp;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &u8 );
    CLI_uint16_required( "Msg Id", &u16 );
    
    AddMqttsn(u8, TOPIC_STORAGE_GET_START, u16); 
    
    CLI_uint32_required( "Transaction ID", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint32_required( "Block size", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_string_ptr_required(  "filename", &cp );
    while( *cp ){
        numBytes++;
        cli_cmd_buff_wr_u8( *cp );
        cp++;
    }
    cli_cmd_buff_wr_u8(0);
    
    do_dispatch(15+numBytes+1);
}

static void do_get_file( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &u8 );
    CLI_uint16_required( "Msg Id", &u16 );
    
    AddMqttsn(u8,  TOPIC_STORAGE_GET_DATA_REQ, u16); 
    
    CLI_uint32_required( "Transaction ID", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint32_required( "Block #", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    do_dispatch(15);
}

static void do_put_start( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint32_t numBytes = 0;
    char *cp;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &u8 );
    CLI_uint16_required( "Msg Id", &u16 );
    
    AddMqttsn(u8,  TOPIC_STORAGE_PUT_START, u16); 
    
    CLI_uint32_required( "Transaction ID", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint32_required( "File size", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint32_required( "Block size", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint16_required( "File CRC", &u16 );
    cli_cmd_buff_wr_u16(u16);
    
   
    CLI_string_ptr_required(  "filename", &cp );
    while( *cp ){
        numBytes++;
        cli_cmd_buff_wr_u8( *cp );
        cp++;
    }
    cli_cmd_buff_wr_u8(0);
    
    do_dispatch(21+numBytes+1);
}

static void do_put_file( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &u8 );
    CLI_uint16_required( "Msg Id", &u16 );
    
    AddMqttsn(u8,  TOPIC_STORAGE_PUT_DATA_REQ, u16); 
    
    CLI_uint32_required( "Transaction ID", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint32_required( "Fragment number", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    CLI_uint16_required( "Fragment size", &u16 );
    cli_cmd_buff_wr_u16(u16);
    
    CLI_uint16_required( "File CRC", &u16 );
    cli_cmd_buff_wr_u16(u16);
    
    cli_cmd_buff_wr_u32(0x01020304);
    
    do_dispatch(23);
}

static void do_get_file_done( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &u8 );
    CLI_uint16_required( "Msg Id", &u16 );
    
    AddMqttsn(u8,  TOPIC_STORAGE_GET_STOP, u16); 
    
    CLI_uint32_required( "Transaction ID", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    do_dispatch(7+4);
}

static void do_put_file_done( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    
    cli_cmd_buff_reset();
    
    CLI_uint8_required( "MQTTSN flags", &u8 );
    CLI_uint16_required( "Msg Id", &u16 );
    
    AddMqttsn(u8,  TOPIC_STORAGE_PUT_STOP, u16); 
    
    CLI_uint32_required( "Transaction ID", &u32 );
    cli_cmd_buff_wr_u32(u32);
    
    do_dispatch(7+4);
}

static void do_puback( const struct cli_cmd_entry *pEntry)
{
    uint8_t u8;
    uint16_t u16;
    
    cli_cmd_buff_reset();
    
    cli_cmd_buff_wr_u8(MQTTSN_PUBACK);
    
    CLI_uint16_required( "Topic ID", &u16 );
    cli_cmd_buff_wr_u16(u16);
    
    CLI_uint16_required( "Msg ID", &u16 );
    cli_cmd_buff_wr_u16(u16);
    
    CLI_uint8_required( "Return code", &u8 );
    cli_cmd_buff_wr_u8(u8);
    
    do_dispatch(7);
}


/* sensor sub menu */
const struct cli_cmd_entry sensor_cmds[] =
{
   CLI_CMD_SIMPLE( "all-stop", do_all_stop_cli, "stop all activity" ),
   CLI_CMD_SIMPLE( "uuids", do_get_uuids, "get-UUIDS" ),
   CLI_CMD_SIMPLE( "version", do_version, "get-version" ),
   CLI_CMD_SIMPLE( "status-clear", do_status_clear, "clear status" ),
   CLI_CMD_SIMPLE( "compiler-stamp", do_compilestamp, "get-compile-timestamp"),
   CLI_CMD_SIMPLE( "model-guid", do_model_guid, "get guid of model" ),
   CLI_CMD_SIMPLE( "live-start", do_livestream_start, "start livestream data" ),
   CLI_CMD_SIMPLE( "live-stop", do_livestream_stop, "stop livestream data" ),
   CLI_CMD_SIMPLE( "live-lst", do_livestream_sensor_list_req, "get active sensors list" ),
   CLI_CMD_SIMPLE( "live-rate", do_livestream_set_rate_req, "set streaming rate for active sensors" ),
   CLI_CMD_SUBMENU(   "ble-sensor", ble_sensor_menu, "select sensor for ble data"),
   CLI_CMD_SIMPLE( "clear", do_clear, "clear sensors" ),
   CLI_CMD_SIMPLE( "list", do_list, "list sensors" ),
   CLI_CMD_SIMPLE( "status", do_status, "get status" ),
   CLI_CMD_SIMPLE( "done",   do_configure, "done-configuring" ),
   CLI_CMD_SUBMENU( "add", sensor_add_cmds, "add sensors" ),
   CLI_CMD_SIMPLE( "filename", do_filename_cmd, "set-filename-prefix" ),
   CLI_CMD_SIMPLE( "datetime", do_datetime, "set datetime"),
   CLI_CMD_SIMPLE( "start-collect", do_start, "start data collection"),
   CLI_CMD_SIMPLE( "stop-collect", do_stop, "stop data collection" ),
   CLI_CMD_SIMPLE( "clssf-start", do_classification_start, "Start Classification"),
   CLI_CMD_SIMPLE( "clssf-stop", do_classification_stop, "Stop Classification"),
   CLI_CMD_SIMPLE( "clssf-rate", do_classification_set_rate, "Count down Classification"),
   CLI_CMD_SIMPLE( "recog-start", do_recog_start, "Start Recognition"),
   CLI_CMD_SIMPLE( "recog-stop", do_recog_stop, "Stop Recognition"),
   CLI_CMD_SIMPLE( "dir", do_storage_dir, "List contents of the default directory"),
   CLI_CMD_SIMPLE( "disk-space", do_disk_space, "Return information about disk space"),
   CLI_CMD_SIMPLE( "delete", do_delete_file, "Delete a file from storage"),
   CLI_CMD_SIMPLE( "get-start", do_get_start, "Start a get file transfer"),
   CLI_CMD_SIMPLE( "get-file", do_get_file, "Get a file fragment"),
   CLI_CMD_SIMPLE( "get-file-done", do_get_file_done, "End get file transfer"),
   CLI_CMD_SIMPLE( "put-start", do_put_start, "Start a put file transfer"),
   CLI_CMD_SIMPLE( "put-file", do_put_file, "Put a file fragment"),
   CLI_CMD_SIMPLE( "put-file-done", do_put_file_done, "End put file transfer"),   
   CLI_CMD_SIMPLE( "puback", do_puback, "Send a Puback for Publish"),
   CLI_CMD_TERMINATE()
};
