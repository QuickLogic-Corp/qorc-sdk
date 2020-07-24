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
#include "sensor_cmds.h"
#include "dcl_commands.h"
#include "iop_messages.h"
#include "cli.h"
#include "string.h"
#include "Mqttsn_MessageHandler.h"

#include "Mqttsn_CliCmds.h"

#include "DataCollection.h"

//uint8_t raw_bytes[128];

const uint8_t MsgLength[] = {
    5,      /*MQTTSN_ADVERTISE*/ 
    3,      /*MQTTSN_SEARCHGW*/
    3,      /*MQTTSN_GWINFO - variable length*/
    0,      /*MQTTSN_RESERVED1*/
    6,      /*MQTTSN_CONNECT - Client ID->variable length */
    3,      /*MQTTSN_CONNACK*/ 
    2,      /*MQTTSN_WILLTOPICREQ*/
    3,      /*MQTTSN_WILLTOPIC, will topic->variable length*/
    2,      /*MQTTSN_WILLMSGREQ*/
    2,      /*MQTTSN_WILLMSG will msg->variable length*/
    6,      /*MQTTSN_REGISTER topicName->variable length*/
    7,      /*MQTTSN_REGACK*/ 
    7,      /*MQTTSN_PUBLISH Data->variable length*/            
    7,      /*MQTTSN_PUBACK */
    4,      /*MQTTSN_PUBCOMP */ 
    4,      /*MQTTSN_PUBREC */  
    4,      /*MQTTSN_PUBREL */ 
    0,      /*MQTTSN_RESERVED2 */ 
    7,      /*MQTTSN_SUBSCRIBE */ 
    8,      /*MQTTSN_SUBACK */ 
    7,      /*MQTTSN_UNSUBSCRIBE */ 
    4,      /*MQTTSN_UNSUBACK */ 
    2,      /*MQTTSN_PINGREQ Client ID->variable length*/ 
    2,      /*MQTTSN_PINGRESP */ 
    2,      /*MQTTSN_DISCONNECT */ 
    0,      /*MQTTSN_RESERVED3 */ 
    0,      /*MQTTSN_WILLTOPICUPD */ 
    0,      /*MQTTSN_WILLTOPICRESP */ 
    0,      /*MQTTSN_WILLMSGUPD */ 
    0       /*MQTTSN_WILLMSGRESP */
};

void do_mqttsn_dispatch(uint32_t numBytes)
{
    /* call the command processor */
  CLI_ProcessDataIn(raw_bytes, numBytes);
}

static void receive_advertise( const struct cli_cmd_entry *pEntry )
{
    uint8_t gwId;
    uint16_t chIn;
  /* turn on selected sensor over ble */
  (void)(pEntry);
  cli_cmd_buff_reset();
  //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_ADVERTISE]);
  cli_cmd_buff_wr_u8(MQTTSN_ADVERTISE);  
  
  CLI_uint8_required( "GW ID", &gwId );
  cli_cmd_buff_wr_u8( gwId );
  
  CLI_uint16_required( "Duration", &chIn );
  cli_cmd_buff_wr_u16( chIn );

  do_mqttsn_dispatch(MsgLength[MQTTSN_ADVERTISE]);
}


static void receive_gwinfo( const struct cli_cmd_entry *pEntry )
{
    char *gwAddr;
    uint8_t gwId;
    uint32_t numBytes = 0;
    
  /* turn on selected sensor over ble */
  (void)(pEntry);
  cli_cmd_buff_reset();
  
  CLI_uint8_required( "GW ID", &gwId );
  CLI_string_ptr_required( "GW Addr", &gwAddr );
  
  //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_GWINFO]+sizeof(gwAddr));
  cli_cmd_buff_wr_u8(MQTTSN_GWINFO);  
  cli_cmd_buff_wr_u8( gwId );
  
  while( *gwAddr )
  {
      numBytes++;
      cli_cmd_buff_wr_u8( *gwAddr );
      gwAddr++;
  }
  //cli_cmd_buff_wr_u8(0);
  do_mqttsn_dispatch(MsgLength[MQTTSN_GWINFO]+numBytes);
}


static void receive_connack( const struct cli_cmd_entry *pEntry )
{
    uint8_t retCode;
    
  (void)(pEntry);
  cli_cmd_buff_reset();
  
  //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_CONNACK]);
  cli_cmd_buff_wr_u8(MQTTSN_CONNACK); 
  
  CLI_uint8_required( "GW ID", &retCode );
  cli_cmd_buff_wr_u8( retCode );
  
  do_mqttsn_dispatch(MsgLength[MQTTSN_CONNACK]);
}

static void receive_puback( const struct cli_cmd_entry *pEntry )
{
    uint16_t rd16;
    uint8_t rd8;
    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_PUBACK]);
    cli_cmd_buff_wr_u8(MQTTSN_PUBACK); 
  
    CLI_uint16_required( "Topic ID", &rd16 );
    cli_cmd_buff_wr_u16( rd16 );
    
    CLI_uint16_required( "Msg ID", &rd16 );
    cli_cmd_buff_wr_u16( rd16 );
    
    CLI_uint8_required( "Return code", &rd8 );
    cli_cmd_buff_wr_u8( rd8 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_PUBACK]);
}

static void receive_pubrec( const struct cli_cmd_entry *pEntry )
{
    uint16_t rd16;
    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_PUBREC]);
    cli_cmd_buff_wr_u8(MQTTSN_PUBREC); 
    
    CLI_uint16_required( "Msg ID", &rd16 );
    cli_cmd_buff_wr_u16( rd16 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_PUBREC]);
}

static void receive_pubrel( const struct cli_cmd_entry *pEntry )
{
    uint16_t rd16;
    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_PUBREL]);
    cli_cmd_buff_wr_u8(MQTTSN_PUBREL); 
    
    CLI_uint16_required( "Msg ID", &rd16 );
    cli_cmd_buff_wr_u16( rd16 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_PUBREL]);    
}

static void receive_pubcomp( const struct cli_cmd_entry *pEntry )
{
    uint16_t rd16;
    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_PUBCOMP]);
    cli_cmd_buff_wr_u8(MQTTSN_PUBCOMP); 
    
    CLI_uint16_required( "Msg ID", &rd16 );
    cli_cmd_buff_wr_u16( rd16 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_PUBCOMP]);    
}

static void receive_unsuback( const struct cli_cmd_entry *pEntry )
{
    uint16_t rd16;
    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_UNSUBACK]);
    cli_cmd_buff_wr_u8(MQTTSN_UNSUBACK); 
                           
    CLI_uint16_required( "Msg ID", &rd16 );
    cli_cmd_buff_wr_u16( rd16 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_UNSUBACK]);

}

static void receive_disconnect( const struct cli_cmd_entry *pEntry )
{    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_DISCONNECT]);
    cli_cmd_buff_wr_u8(MQTTSN_DISCONNECT); 
    
    //CLI_uint16_required( "Duration", &rd16 );
    //cli_cmd_buff_wr_u16( rd16 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_DISCONNECT]);
     
}

static void receive_pingreq( const struct cli_cmd_entry *pEntry )
{
    uint64_t rd64;
    
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_PINGREQ]+sizeof(rd64));
    cli_cmd_buff_wr_u8(MQTTSN_PINGREQ); 
    
    CLI_uint64_required( "Client Id", &rd64 );
    cli_cmd_buff_wr_u64( rd64 );
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_PINGREQ]+sizeof(rd64));
}

static void receive_pingresp( const struct cli_cmd_entry *pEntry )
{
    (void)(pEntry);
    cli_cmd_buff_reset();
    
    //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_PINGRESP]);
    cli_cmd_buff_wr_u8(MQTTSN_PINGRESP);
    
    do_mqttsn_dispatch(MsgLength[MQTTSN_PINGRESP]);
}

static void receive_suback( const struct cli_cmd_entry *pEntry )
{
    uint8_t ch8;
    uint16_t ch16;
    
  /* turn on selected sensor over ble */
  (void)(pEntry);
  cli_cmd_buff_reset();
  
  //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_SUBACK]);
  cli_cmd_buff_wr_u8(MQTTSN_SUBACK); 
  
  CLI_uint8_required( "Flags", &ch8 );
  cli_cmd_buff_wr_u8( ch8 );
  
  CLI_uint16_required( "Topic ID", &ch16 );
  cli_cmd_buff_wr_u16( ch16 );
  
  CLI_uint16_required( "Msg ID", &ch16 );
  cli_cmd_buff_wr_u16( ch16 );
  
  CLI_uint8_required( "Return code", &ch8 );
  cli_cmd_buff_wr_u8( ch8 );
  

  do_mqttsn_dispatch(MsgLength[MQTTSN_SUBACK]);
}

static void receive_regack( const struct cli_cmd_entry *pEntry )
{
    uint8_t ch8;
    uint16_t ch16;
    
  /* turn on selected sensor over ble */
  (void)(pEntry);
  cli_cmd_buff_reset();
  
  //cli_cmd_buff_wr_u8(MsgLength[MQTTSN_SUBACK]);
  cli_cmd_buff_wr_u8(MQTTSN_REGACK); 
  
  CLI_uint16_required( "Topic ID", &ch16 );
  cli_cmd_buff_wr_u16( ch16 );
  
  CLI_uint16_required( "Msg ID", &ch16 );
  cli_cmd_buff_wr_u16( ch16 );
  
  CLI_uint8_required( "Return code", &ch8 );
  cli_cmd_buff_wr_u8( ch8 );
  

  do_mqttsn_dispatch(MsgLength[MQTTSN_REGACK]);
}

/* sensor sub menu */
const struct cli_cmd_entry mqttsn_cmds[] =
  {
    
   CLI_CMD_SIMPLE( "advertise", receive_advertise, "advertise" ),
   CLI_CMD_SIMPLE( "gwinfo", receive_gwinfo, "GW Info" ),
   CLI_CMD_SIMPLE( "connack", receive_connack, "connack" ),
   CLI_CMD_SIMPLE( "suback", receive_suback, "suback" ),
   CLI_CMD_SIMPLE( "regack", receive_regack, "regack" ),
   CLI_CMD_SUBMENU("publish", sensor_cmds, "Publish message"),
   CLI_CMD_SIMPLE( "puback", receive_puback, "Pub acknowledgement"),
   CLI_CMD_SIMPLE( "pubrec", receive_pubrec, "Pub Rec" ),
   CLI_CMD_SIMPLE( "pubrel", receive_pubrel, "Pub rel" ),
   CLI_CMD_SIMPLE( "pubcomp", receive_pubcomp, "Pub Comp" ),
   CLI_CMD_SIMPLE( "unsuback", receive_unsuback, "Unsubscribe ack" ),
   CLI_CMD_SIMPLE( "disconnect", receive_disconnect, "Disconnect" ),
   CLI_CMD_SIMPLE( "pingreq", receive_pingreq, "Ping request" ),
   CLI_CMD_SIMPLE( "pingresp", receive_pingresp, "Ping response" ),
   
   CLI_CMD_TERMINATE()
  };
