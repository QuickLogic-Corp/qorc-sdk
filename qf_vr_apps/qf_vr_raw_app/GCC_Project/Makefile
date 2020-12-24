#
# Makefile for Quick-AI SDK
#

include config.mk

VPATH:=$(OUTPUT_PATH)

all: _makedirs_ external_srcs Appdir AMAZON
	-${HIDE}$(RM) $(OUTPUT_PATH)$(DIR_SEP)bin$(DIR_SEP)$(OUTPUT_FILE).*
	make -f makefiles/Makefile_output
	@echo Copy output files ...
	${HIDE}$(CP) $(OUTPUT_PATH)$(DIR_SEP)$(OUTPUT_FILE).bin $(OUTPUT_PATH)$(DIR_SEP)bin
	${HIDE}$(CP) $(OUTPUT_PATH)$(DIR_SEP)$(OUTPUT_FILE).map $(OUTPUT_PATH)$(DIR_SEP)bin
	${HIDE}$(CP) $(OUTPUT_PATH)$(DIR_SEP)$(OUTPUT_FILE).elf $(OUTPUT_PATH)$(DIR_SEP)bin


.PHONY: _makedirs_
_makedirs_:
	@echo Trying to create output folders if do not exist ...
	-$(MKDIR) $(OUTPUT_PATH)
	-$(MKDIR) $(DEPEND_PATH)
	-$(MKDIR) $(OUTPUT_PATH)$(DIR_SEP)bin
	@echo Starting Build ...

.PHONY: external_srcs
external_srcs: Startup HAL FreeRTOS Libraries BSP fsm DatablockProcessor Control
	@echo "External sources built"

.PHONY: Appdir
Appdir:
	make -f makefiles/Makefile_Appdir
clean_Appdir:
	make -f makefiles/Makefile_Appdir clean

CLEAN_TARGET += Appdir

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

FreeRTOS:
	make -f makefiles/Makefile_FreeRTOS
clean_FreeRTOS:
	make -f makefiles/Makefile_FreeRTOS clean

CLEAN_TARGET += FreeRTOS

.PHONY:BSP
BSP:
	make -f makefiles/Makefile_quickfeather_BOARD
clean_BSP:
	make -f makefiles/Makefile_quickfeather_BOARD clean

CLEAN_TARGET += BSP

.PHONY: AMAZON
AMAZON:
	make -f makefiles/Makefile_Amazon
clean_AMAZON:
	make -f makefiles/Makefile_Amazon clean

CLEAN_TARGET += AMAZON

.PHONY:DSPC
DSPC:
	make -f makefiles/Makefile_DSPC
clean_DSPC:
	make -f makefiles/Makefile_DSPC clean

CLEAN_TARGET += DSPC

.PHONY:DSPC_Schematics
DSPC_Schematics:
	make -f makefiles/Makefile_DSPC_Schematics
clean_DSPC_Schematics:
	make -f makefiles/Makefile_DSPC_Schematics clean

CLEAN_TARGET += DSPC_Schematics

.PHONY:Libraries
Libraries:
	make -f makefiles/Makefile_Libraries
clean_Libraries:
	make -f makefiles/Makefile_Libraries clean

CLEAN_TARGET += Libraries

.PHONY:DatablockProcessor
DatablockProcessor:
	make -f makefiles/Makefile_DatablockProcessor
clean_DatablockProcessor:
	make -f makefiles/Makefile_DatablockProcessor clean

CLEAN_TARGET += DatablockProcessor

.PHONY:fsm
fsm:
	make -f makefiles/Makefile_fsm
clean_fsm:
	make -f makefiles/Makefile_fsm clean

CLEAN_TARGET += fsm

.PHONY:Control
Control:
	make -f makefiles/Makefile_Control
clean_Control:
	make -f makefiles/Makefile_Control clean

CLEAN_TARGET += Control

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



