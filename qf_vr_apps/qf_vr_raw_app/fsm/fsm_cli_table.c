/* FSM sub menu that defines all control events */
#include "Fw_global_config.h"
#include "ql_controlTask.h"
#include "cli.h"

void cli_fsm_event(const struct cli_cmd_entry *pEntry);

const struct cli_cmd_entry fsm_menu[] = {
	    CLI_CMD_WITH_ARG( "start", cli_fsm_event, CEVENT_START, "sends START event" ),
	    CLI_CMD_WITH_ARG( "hif_eot", cli_fsm_event, CEVENT_HIF_EOT, "sends HIF_EOT event" ),
	    CLI_CMD_WITH_ARG( "host_mute_on", cli_fsm_event, CEVENT_HOST_MUTE_ON, "sends HOST_MUTE_ON event" ),
	    CLI_CMD_WITH_ARG( "host_mute_off", cli_fsm_event, CEVENT_HOST_MUTE_OFF, "sends HOST_MUTE_OFF event" ),
	    CLI_CMD_WITH_ARG( "host_process_on", cli_fsm_event, CEVENT_HOST_PROCESS_ON, "sends HOST_PROCESS_ON event" ),
	    CLI_CMD_WITH_ARG( "host_process_off", cli_fsm_event, CEVENT_HOST_PROCESS_OFF, "sends HOST_PROCESS_OFF event" ),
	    CLI_CMD_WITH_ARG( "host_ready_to_receive", cli_fsm_event, CEVENT_HOST_READY_TO_RECEIVE, "sends HOST_READY_TO_RECEIVE event" ),
	    CLI_CMD_WITH_ARG( "vr_trigger", cli_fsm_event, CEVENT_VR_TRIGGER, "sends VR_TRIGGER event" ),
	    CLI_CMD_WITH_ARG( "lpsd_off", cli_fsm_event, CEVENT_LPSD_OFF, "sends LPSD_OFF event" ),
	    CLI_CMD_WITH_ARG( "lpsd_on", cli_fsm_event, CEVENT_LPSD_ON, "sends LPSD_ON event" ),
	    CLI_CMD_WITH_ARG( "wos_timer", cli_fsm_event, CEVENT_WOS_TIMER, "sends WOS_TIMER event" ),
	    CLI_CMD_WITH_ARG( "wos", cli_fsm_event, CEVENT_WOS, "sends WOS event" ),
	CLI_CMD_TERMINATE()
};
