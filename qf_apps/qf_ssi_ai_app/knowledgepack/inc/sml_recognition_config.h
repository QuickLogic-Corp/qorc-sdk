#ifndef __SENSIML_SENSOR_CONFIG_H__
#define __SENSIML_SENSOR_CONFIG_H__

#include <stdint.h>
#include "dcl_commands.h"
#include "sensor_config.h"


#define SENSIML_INTERFACE_CONFIG_NUM_MSGS 5

const struct sensor_config_msg recognition_config[] = {
	SENSOR_CONFIG_CLEAR_MSG(),
	SENSOR_CONFIG_ADD_MSG(1229804865, 105, SENSOR_CONFIG_ARRAY(0x14)),
	SENSOR_CONFIG_DONE_MSG()
};

#endif //__SENSIML_SENSOR_CONFIG_H__