/* State Machine Description */
/* Created from state file: 'FSMstates.csv' */
/* In a separate file for ease of creation.  Should only be included in ql_controlTask.c */
#include "ql_controlDefines.h"
#if KPROCESS < 2
#error KPROCESS < 2
#endif
#if KSTATES < 6
#error KSTATES < 6
#endif
#define KPROCESSACTUAL 2
#define KSTATESACTUAL 6

//                                                      H2D                  CLI                 
struct GSMrow afsmrow[KSTATES] = {

  {true,  "RUNNING"           , 0                     ,{ PSTATE_STARTED      ,PSTATE_STARTED      ,},
                                                       { 0                   ,0                   ,}},
  {true,  "CONFIGURED"        , 0                     ,{ PSTATE_STOPPED      ,PSTATE_STOPPED      ,},
                                                       { 0                   ,0                   ,}},
  {false, ""                  , CEVENT_START          ,{ 0                   ,0                   ,},
                                                       { PACTION_START       ,PACTION_START       ,}},
  {true,  "INITIAL"           , 0                     ,{ PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,},
                                                       { 0                   ,0                   ,}},
  {false, ""                  , CEVENT_CONFIG         ,{ 0                   ,0                   ,},
                                                       { PACTION_CONFIG      ,PACTION_CONFIG      ,}},
  {true,  "//ENDOFTABLE"      , 0                     ,{ 0                   ,0                   ,},
                                                       { 0                   ,0                   ,}}
};

struct GSMrow   currentState = 

  {true,  "INITIAL"           , 0                     ,{ PSTATE_UNCONFIG     ,PSTATE_UNCONFIG     ,},
                                                       { 0                   ,0                   ,}}
;
extern enum process_state LPSD_State;

enum process_state H2D_FSMAction(enum process_action, void*);
extern int         H2D_FSMConfigData;
enum process_state CLI_FSMAction(enum process_action, void*);
extern int         CLI_FSMConfigData;

struct ProcessActions apaction[KPROCESSACTUAL] = {
    {            H2D_FSMAction,               (void*)&H2D_FSMConfigData},
    {            CLI_FSMAction,               (void*)&CLI_FSMConfigData},
};

char* apsFromProcess[] = {
    "H2D",
    "CLI",
};

char* apsFromFSMS[] = {
	"UNCONFIG",
	"CONFIG",
	"NULL",
	"INITIAL",
	"CONFIGURED",
	"RUNNING",
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

