/* State Machine Description */
/* Created from state file: 'FSMstates.csv' */
/* In a separate file for ease of creation.  Should only be included in ql_controlTask.c */
#include "ql_controlDefines.h"
#if KPROCESS < 7
#error KPROCESS < 7
#endif
#if KSTATES < 13
#error KSTATES < 13
#endif
#define KPROCESSACTUAL 7
#define KSTATESACTUAL 13

//                                                      DFS                  VR                   AudioHW              PTT_Switch           Mute_Switch          LPSD                 CircularBuffer      
struct GSMrow afsmrow[KSTATES] = {

  {true,  "WAIT_ON_KWD"       , 0                     ,{ PSTATE_NODE4        ,PSTATE_STARTED      ,PSTATE_STREAMING    ,PSTATE_DONT_CARE    ,PSTATE_DONT_CARE    ,PSTATE_DONT_CARE    ,PSTATE_SAVING       ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}},
  {false, ""                  , CEVENT_VR_TRIGGER     ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,PSTATE_LPSD_ON      ,0                   ,},
                                                       { PACTION_MIN_NODE4   ,PACTION_START       ,PACTION_START_STREAMING,0                   ,0                   ,PACTION_START_ON    ,0                   ,}},
  {false, ""                  , CEVENT_VR_TRIGGER     ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,PSTATE_LPSD_OFF     ,0                   ,},
                                                       { PACTION_MIN_NODE3   ,PACTION_STOP        ,PACTION_START_SAVING,0                   ,0                   ,PACTION_START_OFF   ,0                   ,}},
  {false, ""                  , CEVENT_LPSD_OFF       ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { PACTION_MIN_NODE3   ,PACTION_STOP        ,PACTION_START_SAVING,0                   ,0                   ,PACTION_START_OFF   ,0                   ,}},
  {false, ""                  , CEVENT_LPSD_ON        ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}},
  {true,  "WAIT_ON_LPSD"      , 0                     ,{ PSTATE_NODE3        ,PSTATE_STOPPED      ,PSTATE_SAVING       ,PSTATE_DONT_CARE    ,PSTATE_DONT_CARE    ,PSTATE_DONT_CARE    ,PSTATE_SAVING       ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}},
  {false, ""                  , CEVENT_LPSD_ON        ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { PACTION_MIN_NODE4   ,PACTION_START       ,PACTION_START_STREAMING,0                   ,0                   ,PACTION_START_ON    ,0                   ,}},
  {false, ""                  , CEVENT_LPSD_OFF       ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { PACTION_MIN_NODE3   ,PACTION_STOP        ,PACTION_START_SAVING,0                   ,0                   ,PACTION_START_OFF   ,0                   ,}},
  {true,  "CONFIGURED"        , 0                     ,{ PSTATE_DONT_CARE    ,PSTATE_STOPPED      ,PSTATE_STOPPED      ,PSTATE_DONT_CARE    ,PSTATE_DONT_CARE    ,PSTATE_STOPPED      ,PSTATE_STOPPED      ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}},
  {false, ""                  , CEVENT_START          ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { PACTION_MIN_NODE3   ,0                   ,PACTION_START_SAVING,0                   ,0                   ,PACTION_START_OFF   ,PACTION_START_SAVING,}},
  {true,  "INITIAL"           , 0                     ,{ PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}},
  {false, ""                  , CEVENT_CONFIG         ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { PACTION_CONFIG      ,PACTION_CONFIG      ,PACTION_CONFIG      ,PACTION_CONFIG      ,PACTION_CONFIG      ,PACTION_CONFIG      ,PACTION_CONFIG      ,}},
  {true,  "//ENDOFTABLE"      , 0                     ,{ 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}}
};

struct GSMrow   currentState = 

  {true,  "INITIAL"           , 0                     ,{ PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,},
                                                       { 0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,0                   ,}}
;
extern enum process_state LPSD_State;

enum process_state DFS_FSMAction(enum process_action, void*);
extern int         DFS_FSMConfigData;
enum process_state VR_FSMAction(enum process_action, void*);
extern int         VR_FSMConfigData;
enum process_state AudioHW_FSMAction(enum process_action, void*);
extern int         AudioHW_FSMConfigData;
enum process_state PTT_Switch_FSMAction(enum process_action, void*);
extern int         PTT_Switch_FSMConfigData;
enum process_state Mute_Switch_FSMAction(enum process_action, void*);
extern int         Mute_Switch_FSMConfigData;
enum process_state LPSD_FSMAction(enum process_action, void*);
extern int         LPSD_FSMConfigData;
enum process_state CircularBuffer_FSMAction(enum process_action, void*);
extern int         CircularBuffer_FSMConfigData;

struct ProcessActions apaction[KPROCESSACTUAL] = {
    {            DFS_FSMAction,               (void*)&DFS_FSMConfigData},
    {             VR_FSMAction,                (void*)&VR_FSMConfigData},
    {        AudioHW_FSMAction,           (void*)&AudioHW_FSMConfigData},
    {     PTT_Switch_FSMAction,        (void*)&PTT_Switch_FSMConfigData},
    {    Mute_Switch_FSMAction,       (void*)&Mute_Switch_FSMConfigData},
    {           LPSD_FSMAction,              (void*)&LPSD_FSMConfigData},
    { CircularBuffer_FSMAction,    (void*)&CircularBuffer_FSMConfigData},
};

char* apsFromProcess[] = {
    "DFS",
    "VR",
    "AudioHW",
    "PTT_Switch",
    "Mute_Switch",
    "LPSD",
    "CircularBuffer",
};

char* apsFromFSMS[] = {
	"UNCONFIG",
	"CONFIG",
	"NULL",
	"INITIAL",
	"CONFIGURED",
	"WAIT_ON_LPSD",
	"WAIT_ON_KWD",
};

char* apsFromPS[] = {
	"UNCONFIG",
	"UNKNOWN",
	"DONT_CARE",
	"STOPPING",
	"OUTQING",
	"LPSD_ON",
	"LPSD_OFF",
	"STREAMING",
	"SAVING",
	"STARTED",
	"WOS",
	"NORMAL",
	"STOPPED",
	"NODE7",
	"NODE6",
	"NODE5",
	"NODE4",
	"NODE3",
	"NODE2",
	"NODE1",
	"NODE0",
};

char* apsFromCE[] = {
	"CONFIG",
	"START",
	"HIF_EOT",
	"HOST_MUTE_ON",
	"HOST_MUTE_OFF",
	"HOST_PROCESS_ON",
	"HOST_PROCESS_OFF",
	"HOST_READY_TO_RECEIVE",
	"VR_TRIGGER",
	"LPSD_OFF",
	"LPSD_ON",
	"WOS_TIMER",
	"WOS",
};

char* apsFromPA[] = {
	"NULL",
	"BEGIN_STOPPING",
	"FINISH_STOPPING",
	"START_OUTQ",
	"STATE_ON",
	"STATE_OFF",
	"START_ON",
	"START_OFF",
	"START_STREAMING",
	"START_SAVING",
	"START",
	"START_WOS",
	"START_NORMAL",
	"STOP",
	"MIN_NODE7",
	"MIN_NODE6",
	"MIN_NODE5",
	"MIN_NODE4",
	"MIN_NODE3",
	"MIN_NODE2",
	"MIN_NODE1",
	"MIN_NODE0",
	"CONFIG",
};

