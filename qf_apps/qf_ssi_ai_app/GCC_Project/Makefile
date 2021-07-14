#
# Makefile for Quick-AI SDK
#

include config.mk

VPATH:=$(OUTPUT_PATH)

all: _makedirs_ external_srcs appdir gateware s3gw_drivers
	-${HIDE}$(RM) $(OUTPUT_PATH)$(DIR_SEP)bin$(DIR_SEP)$(OUTPUT_FILE).*
	make -f makefiles/Makefile_output
	-${HIDE}$(MKDIR) $(OUTPUT_PATH)$(DIR_SEP)bin
	@echo Copy output files ...
	${HIDE}$(CP) $(OUTPUT_PATH)$(DIR_SEP)$(OUTPUT_FILE).bin $(OUTPUT_PATH)$(DIR_SEP)bin
	${HIDE}$(CP) $(OUTPUT_PATH)$(DIR_SEP)$(OUTPUT_FILE).map $(OUTPUT_PATH)$(DIR_SEP)bin
	${HIDE}$(CP) $(OUTPUT_PATH)$(DIR_SEP)$(OUTPUT_FILE).elf $(OUTPUT_PATH)$(DIR_SEP)bin

_makedirs_:
	@-$(MKDIR) $(DEPEND_PATH)
	@-$(MKDIR) $(OUTPUT_PATH)


.PHONY: _makedirs_


.PHONY: external_srcs
external_srcs: Startup HAL FreeRTOS Libraries BSP DatablockProcessor GENERATED \
    SENSOR_AUDIO FS_MONITOR
	@echo "External sources built"

.PHONY: appdir
appdir:
	make -f makefiles/Makefile_appdir
clean_appdir:
	make -f makefiles/Makefile_appdir clean

CLEAN_TARGET += appdir

.PHONY:Startup
Startup:
	make -f makefiles/Makefile_Startup
clean_Startup:
	make -f makefiles/Makefile_Startup clean

CLEAN_TARGET += Startup

.PHONY:HAL
HAL:
	make -f makefiles/Makefile_HAL
clean_HAL:
	make -f makefiles/Makefile_HAL clean

CLEAN_TARGET += HAL

.PHONY:CLI
CLI:
	make -f makefiles/Makefile_cli
clean_CLI:
	make -f makefiles/Makefile_cli clean

CLEAN_TARGET += CLI

FreeRTOS:
	make -f makefiles/Makefile_FreeRTOS
clean_FreeRTOS:
	make -f makefiles/Makefile_FreeRTOS clean

CLEAN_TARGET += FreeRTOS

.PHONY:BSP
BSP:
	make -f makefiles/Makefile_quickfeather_BSP
clean_BSP:
	make -f makefiles/Makefile_quickfeather_BSP clean

CLEAN_TARGET += BSP

.PHONY:Libraries
Libraries:
	make -f makefiles/Makefile_Libraries
clean_Libraries:
	make -f makefiles/Makefile_Libraries clean

CLEAN_TARGET += Libraries

.PHONY:gateware
gateware:
	make -f makefiles/Makefile_gateware
clean_gateware:
	make -f makefiles/Makefile_gateware clean

CLEAN_TARGET += gateware

.PHONY:s3gw_drivers
s3gw_drivers:
	make -f makefiles/Makefile_s3gw_drivers
clean_s3gw_drivers:
	make -f makefiles/Makefile_s3gw_drivers clean

CLEAN_TARGET += s3gw_drivers

.PHONY:DatablockProcessor
DatablockProcessor:
	make -f makefiles/Makefile_DatablockProcessor
clean_DatablockProcessor:
	make -f makefiles/Makefile_DatablockProcessor clean

CLEAN_TARGET += DatablockProcessor

.PHONY:FS_MONITOR
FS_MONITOR:
	make -f makefiles/Makefile_fs_monitor
clean_FS_MONITOR:
	make -f makefiles/Makefile_fs_monitor clean

CLEAN_TARGET += FS_MONITOR

.PHONY: GENERATED
GENERATED:
	make -f makefiles/Makefile_generated
clean_GENERATED:
	make -f makefiles/Makefile_generated clean

CLEAN_TARGET += GENERATED

.PHONY:SENSOR_AUDIO
SENSOR_AUDIO:
	make -f makefiles/Makefile_sensor_audio
clean_SENSOR_AUDIO:
	make -f makefiles/Makefile_sensor_audio clean

CLEAN_TARGET += SENSOR_AUDIO

DELETEME=
DELETEME += $(wildcard $(OUTPUT_PATH)$(DIR_SEP)bin$(DIR_SEP)$(OUTPUT_FILE).*)
DELETEME += $(wildcard $(OUTPUT_PATH)$(DIR_SEP)*.o)
DELETEME += $(wildcard $(DEPEND_PATH)$(DIR_SEP)*.d)
DELETEME := $(strip $(DELETEME))
# Clean
clean: $(CLEAN_TARGET:%=clean_%)
	make -f makefiles/Makefile_output clean
ifneq (,$(DELETEME))
	-$(HIDE)$(RM) $(DELETEME)
endif



