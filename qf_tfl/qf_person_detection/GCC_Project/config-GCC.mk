#
# Symbiflow options for QORK SDK
#
export RTL_TOP_MODULE=

#
# GCC Configuration options for QORC SDK
#

#DASH_G=-gdwarf-4
DASH_G=-ggdb
DASH_O=-Os

#Assembler flags
export AS_FLAGS= -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(DASH_O) -fmessage-length=0 \
        -fsigned-char -ffunction-sections -fdata-sections  $(DASH_G) -MMD -MP 

#Preprocessor macros

export MACROS=-D__FPU_USED=1 -D__FPU_USED=1 \
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
export LIBCMSIS_GCC_DIR=$(PROJ_ROOT)$(DIR_SEP)Libraries$(DIR_SEP)CMSIS$(DIR_SEP)lib$(DIR_SEP)GCC

export INCLUDE_DIRS=\
		-I"$(realpath $(TF_DIR))" \
		-I"$(realpath $(TF_DIR)/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include/)" \
		-I"$(realpath $(PROJ_DIR))" \
		-I"$(realpath $(PROJ_ROOT)/$(APP_DIR)/$(PROJ_NAME)/inc)" \
		-I"$(realpath $(PROJ_ROOT)/$(APP_DIR)/$(PROJ_NAME)/fpga/rtl)" \
		-I"$(PROJ_ROOT)/$(APP_DIR)/$(PROJ_NAME)/fpga/inc" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/CMSIS/inc/)" \
		-I"$(realpath $(PROJ_ROOT)/HAL/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/cli/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/Power/inc)" \
		-I"$(realpath $(PROJ_ROOT)/FreeRTOS/include)" \
		-I"$(realpath $(PROJ_ROOT)/FreeRTOS/portable/GCC/ARM_CM4F_quicklogic_s3XX)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/Power/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/Utils/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/SysFlash/inc)" \
		-I"$(realpath $(PROJ_ROOT)/BSP/quickfeather/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/Utils/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/FPGA/inc)" \
		-I"$(realpath $(PROJ_ROOT)/Libraries/DatablockManager/inc)"
    

# C compiler flags
export CFLAGS= $(MACROS) \
        -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
        ${DASH_O} $(OPT_FLAGS) -fmessage-length=0 -lm --specs=nano.specs\
        -fsigned-char -ffunction-sections -fdata-sections  ${DASH_G} -MMD -MD -MP -DTF_LITE_STATIC_MEMORY
#        -fsigned-char -ffunction-sections -fdata-sections  ${DASH_G} -std=c99 -MMD -MD -MP

export CXFLAGS= $(MACROS) \
        -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
        ${DASH_O} $(OPT_FLAGS) -fmessage-length=0 -lm --specs=nano.specs\
        -fsigned-char -ffunction-sections -fdata-sections  ${DASH_G} -std=c++11 -MMD -MD -MP -DTF_LITE_STATIC_MEMORY


export LD_FLAGS_BASE= -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            ${DASH_O} $(OPT_FLAGS) -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  \
            ${DASH_G} -Xlinker --gc-sections -Wall -Werror \
			-Wl,--fatal-warnings  \
            --specs=nano.specs --specs=nosys.specs -Wl,--no-wchar-size-warning \
            -o "$(OUTPUT_PATH)/$(OUTPUT_FILE).elf" -lm\
			-L$(LIBCMSIS_GCC_DIR) -larm_cortexM4lf_math 
	
export LD_FLAGS_1= $(LD_FLAGS_BASE) -T "$(PROJ_DIR)/$(OUTPUT_FILE).ld" \
			-Wl,-Map,"$(OUTPUT_PATH)/$(OUTPUT_FILE).map" 
			
export LD_FLAGS_DUMMY= $(LD_FLAGS_BASE) -T "$(PROJ_DIR)/qf_fake.ld" \
			-Wl,-Map,"$(OUTPUT_PATH)/qf_fake.map" 


export ELF2BIN_OPTIONS=-O binary

#
# Export the files and Directoris that work for both Windows and Linux
# The DIR_SEP is needed only for OS specific command, whereas make can deal with any
#
export COMMON_STUB =$(PROJ_DIR)$(DIR_SEP)makefiles$(DIR_SEP)Makefile_common

export BSP_DIR        = $(PROJ_ROOT)$(DIR_SEP)BSP$(DIR_SEP)quickfeather$(DIR_SEP)src
export HAL_DIR        = $(PROJ_ROOT)$(DIR_SEP)HAL$(DIR_SEP)src
export FREERTOS_DIR   = $(PROJ_ROOT)$(DIR_SEP)FreeRTOS
export LIB_DIR        = $(PROJ_ROOT)$(DIR_SEP)Libraries

export POWER_DIR        = $(LIB_DIR)$(DIR_SEP)Power$(DIR_SEP)src
export SYSFLASH_DIR     = $(LIB_DIR)$(DIR_SEP)SysFlash$(DIR_SEP)src
export UTILS_DIR        = $(LIB_DIR)$(DIR_SEP)Utils$(DIR_SEP)src
export FPGA_DIR       		= $(LIB_DIR)$(DIR_SEP)FPGA$(DIR_SEP)src
export CLI_DIR        		= $(LIB_DIR)$(DIR_SEP)cli$(DIR_SEP)src
export MAIN_DIR       		= $(PROJ_ROOT)$(DIR_SEP)$(APP_DIR)$(DIR_SEP)$(PROJ_NAME)$(DIR_SEP)src
export MAIN_FPGA_RTL_DIR	= $(PROJ_ROOT)$(DIR_SEP)$(APP_DIR)$(DIR_SEP)$(PROJ_NAME)$(DIR_SEP)fpga$(DIR_SEP)rtl
export MAIN_FPGA_SRC_DIR	= $(PROJ_ROOT)$(DIR_SEP)$(APP_DIR)$(DIR_SEP)$(PROJ_NAME)$(DIR_SEP)fpga$(DIR_SEP)src

