#
# Symbiflow options for QORC SDK
#
export RTL_TOP_MODULE=

#
# GCC Configuration options for Quick-AI SDK
#

DASH_G=-gdwarf-4
DASH_O=-Os

#Assembler flags
export AS_FLAGS= -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(DASH_O) -fmessage-length=0 \
        -fsigned-char -ffunction-sections -fdata-sections  $(DASH_G) -MMD -MP 

#Preprocessor macros

export MACROS=-D__FPU_USED=1 \
        -D__M4_DEBUG \
        -D__EOSS3_CHIP \
        -D__RTOS \
        -D__GNU_SOURCE \
        -D_DEFAULT_SOURCE \
        -DARM_MATH_CM4 \
        -DFFE_NEWARCH \
        -DARM_MATH_MATRIX_CHECK \
        -DARM_MATH_ROUNDING \
        -D__FPU_PRESENT \
        -DconfigUSE_STATS_FORMATTING_FUNCTIONS \
        -DconfigUSE_TRACE_FACILITY \
        -D$(TOOLCHAIN) \
        -DNDEBUG\
        -DGCC_MAKE

export OPT_FLAGS=-fmerge-constants -fomit-frame-pointer -fcrossjumping -fexpensive-optimizations -ftoplevel-reorder
export LIBCMSIS_GCC_DIR=$(PROJ_ROOT)$(DIR_SEP)Libraries$(DIR_SEP)CMSIS_5$(DIR_SEP)CMSIS$(DIR_SEP)DSP$(DIR_SEP)Lib$(DIR_SEP)GCC
export LIBAWWE_DIR=$(PROJ_ROOT)$(DIR_SEP)Licensed3rdParty$(DIR_SEP)amazon$(DIR_SEP)lib

export INCLUDE_DIRS=-I"$(PROJ_DIR)" \
                 -I"$(APP_DIR)/inc" \
                 -I"$(APP_DIR)/fsm" \
                 -I"$(PROJ_ROOT)/BSP/quickfeather/inc" \
                 -I"$(PROJ_ROOT)/HAL/inc" \
                 -I"$(PROJ_ROOT)/FreeRTOS/include" \
                 -I"$(PROJ_ROOT)/FreeRTOS/portable/GCC/ARM_CM4F_quicklogic_s3XX" \
                 -I"$(PROJ_ROOT)/Licensed3rdParty/amazon/inc" \
                 -I"$(PROJ_ROOT)/Libraries/Audio/inc" \
                 -I"$(PROJ_ROOT)/Libraries/CMSIS_5/CMSIS/Core/Include" \
                 -I"$(PROJ_ROOT)/Libraries/CMSIS_5/CMSIS/DSP/Include" \
                 -I"$(PROJ_ROOT)/Libraries/CMSIS_5/CMSIS/NN/Include" \
                 -I"$(PROJ_ROOT)/Libraries/Power/inc" \
                 -I"$(PROJ_ROOT)/Libraries/Utils/inc" \
                 -I"$(PROJ_ROOT)/Libraries/cli/inc" \
                 -I"$(PROJ_ROOT)/Libraries/DatablockManager/inc" \
                 -I"$(PROJ_ROOT)/Libraries/D2HProtocol/inc" \
                 -I"$(PROJ_ROOT)/Tasks/Control/inc" \
                 -I"$(PROJ_ROOT)/Tasks/DatablockProcessor/inc" \




# C compiler flags
export CFLAGS= $(MACROS) \
        -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
        $(DASH_O) $(OPT_FLAGS) -fmessage-length=0 -lm \
        -fsigned-char -ffunction-sections -fdata-sections  $(DASH_G) -std=c99 -MMD -MD -MP


export LD_FLAGS_1= -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	$(DASH_O) $(OPT_FLAGS) -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections \
	$(DASH_G) -T "$(PROJ_DIR)/quickfeather.ld" -Xlinker --gc-sections -Wall -Werror \
	-Wl,--fatal-warnings -Wl,--print-memory-usage -Wl,-Map,"$(OUTPUT_PATH)/$(OUTPUT_FILE).map" \
    --specs=nano.specs -u _printf_float --specs=nosys.specs -Wl,--no-wchar-size-warning \
    -o "$(OUTPUT_PATH)/$(OUTPUT_FILE).elf" 

export LD_FLAGS_2=-L$(LIBCMSIS_GCC_DIR) -lm -larm_cortexM4lf_math
export ELF2BIN_OPTIONS=-O binary

#
# Export the files and Directories that work for both Windows and Linux
# The DIR_SEP is needed only for OS specific command, whereas make can deal with any
#
export COMMON_STUB =$(PROJ_DIR)$(DIR_SEP)makefiles$(DIR_SEP)Makefile_common

export MAIN_DIR         = $(APP_DIR)$(DIR_SEP)src
export FSM_DIR          = $(APP_DIR)$(DIR_SEP)fsm

export QUICKFEATHER_DIR = $(PROJ_ROOT)$(DIR_SEP)BSP$(DIR_SEP)quickfeather$(DIR_SEP)src
export HAL_DIR          = $(PROJ_ROOT)$(DIR_SEP)HAL$(DIR_SEP)src
export FREERTOS_DIR     = $(PROJ_ROOT)$(DIR_SEP)FreeRTOS

export LIB_DIR          = $(PROJ_ROOT)$(DIR_SEP)Libraries
export AUDIO_DIR        = $(LIB_DIR)$(DIR_SEP)Audio$(DIR_SEP)src
export CLI_DIR          = $(LIB_DIR)$(DIR_SEP)cli$(DIR_SEP)src
export POWER_DIR        = $(LIB_DIR)$(DIR_SEP)Power$(DIR_SEP)src
export UTILS_DIR        = $(LIB_DIR)$(DIR_SEP)Utils$(DIR_SEP)src
export DBM_DIR          = $(LIB_DIR)$(DIR_SEP)DatablockManager$(DIR_SEP)src
export D2H_DIR          = $(LIB_DIR)$(DIR_SEP)D2HProtocol$(DIR_SEP)src
export CMSIS_DIR        = $(LIB_DIR)$(DIR_SEP)CMSIS_5$(DIR_SEP)CMSIS$(DIR_SEP)NN$(DIR_SEP)Source

export DBP_DIR          = $(PROJ_ROOT)$(DIR_SEP)Tasks$(DIR_SEP)DatablockProcessor$(DIR_SEP)src
export CONTROL_DIR      = $(PROJ_ROOT)$(DIR_SEP)Tasks$(DIR_SEP)Control$(DIR_SEP)src

# Enable the below lines to link with pryon_lite and DSPC
export AMAZON_DIR       = $(PROJ_ROOT)$(DIR_SEP)Licensed3rdParty$(DIR_SEP)amazon$(DIR_SEP)src
export DSPC_DIR         = $(PROJ_ROOT)$(DIR_SEP)Licensed3rdParty$(DIR_SEP)dspc$(DIR_SEP)ns$(DIR_SEP)src
export DSPC_SCHEMATICS_DIR   = $(PROJ_ROOT)$(DIR_SEP)Licensed3rdParty$(DIR_SEP)dspc$(DIR_SEP)ns$(DIR_SEP)Schematics

ifneq ("", "$(strip $(wildcard $(AMAZON_DIR)/*.c))")
export LIBS+=-L$(LIBAWWE_DIR) -lpryon_lite-U -lpryon_lite-PRL1000
endif
