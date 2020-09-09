#
# Makefile for Quick-AI SDK
#

include config.mk

VPATH:=$(OUTPUT_PATH)

all: _makedirs_ gateware external_srcs appdir s3gw_drivers
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
               ADC SENSOR_AD7476 SENSOR_AUDIO
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

.PHONY: GENERATED
GENERATED:
	make -f makefiles/Makefile_generated
clean_GENERATED:
	make -f makefiles/Makefile_generated clean

CLEAN_TARGET += GENERATED

.PHONY:DatablockProcessor
DatablockProcessor:
	make -f makefiles/Makefile_DatablockProcessor
clean_DatablockProcessor:
	make -f makefiles/Makefile_DatablockProcessor clean

CLEAN_TARGET += DatablockProcessor

.PHONY:ADC
ADC:
	make -f makefiles/Makefile_ADC
clean_ADC:
	make -f makefiles/Makefile_ADC clean

CLEAN_TARGET += ADC

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

.PHONY:SENSOR_AD7476
SENSOR_AD7476:
	make -f makefiles/Makefile_sensor_ad7476
clean_SENSOR_AD7476:
	make -f makefiles/Makefile_sensor_ad7476 clean

CLEAN_TARGET += SENSOR_AD7476

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



