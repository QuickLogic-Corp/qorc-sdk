#
# Symbiflow options for QORK SDK
#
export RTL_TOP_MODULE=top

#
# GCC Configuration options for QORC SDK
#

DASH_G=-gdwarf-4
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
export LIBSENSIML_DIR=$(APP_DIR)$(DIR_SEP)knowledgepack$(DIR_SEP)sensiml
export LIBCMSIS_GCC_DIR=$(PROJ_ROOT)$(DIR_SEP)Libraries$(DIR_SEP)CMSIS$(DIR_SEP)lib$(DIR_SEP)GCC

export INCLUDE_DIRS=-I"$(PROJ_DIR)" \
                 -I"$(APP_DIR)/inc" \
                 -I"$(APP_DIR)/IOP_MQTTSN/inc" \
                 -I"$(APP_DIR)/knowledgepack/sensiml/inc" \
                 -I"$(APP_DIR)/knowledgepack/inc" \
                 -I"$(APP_DIR)/ad7476/inc" \
                 -I"$(APP_DIR)/sensor_audio/inc" \
                 -I"$(PROJ_ROOT)/s3-gateware" \
                 -I"$(PROJ_ROOT)/s3-gateware/adc_ad7476_if/rtl" \
                 -I"$(PROJ_ROOT)/freertos_gateware/inc" \
                 -I"$(PROJ_ROOT)/Libraries/CMSIS/inc" \
                 -I"$(PROJ_ROOT)/Libraries/CMSIS/Core/Include" \
                 -I"$(PROJ_ROOT)/HAL/inc" \
                 -I"$(PROJ_ROOT)/Libraries/cli/inc" \
                 -I"$(PROJ_ROOT)/Libraries/FreeRTOS_FAT/include" \
                 -I"$(PROJ_ROOT)/Libraries/FreeRTOS_FAT/portable/QL" \
                 -I"$(PROJ_ROOT)/Libraries/FreeRTOS_FAT" \
                 -I"$(PROJ_ROOT)/Libraries/Power/inc" \
                 -I"$(PROJ_ROOT)/Libraries/Audio/inc" \
                 -I"$(PROJ_ROOT)/Libraries/MQTTSN/inc" \
                 -I"$(PROJ_ROOT)/Libraries/MQTTSN_SML/inc" \
                 -I"$(PROJ_ROOT)/Libraries/QLFS/inc" \
                 -I"$(PROJ_ROOT)/FreeRTOS/include" \
                 -I"$(PROJ_ROOT)/FreeRTOS/portable/GCC/ARM_CM4F_quicklogic_s3XX" \
                 -I"$(PROJ_ROOT)/Libraries/Power/inc" \
                 -I"$(PROJ_ROOT)/Libraries/riff_file/inc" \
                 -I"$(PROJ_ROOT)/Libraries/SensorFramework/inc" \
                 -I"$(PROJ_ROOT)/Libraries/SensorFramework/drivers/M4/mc3635" \
                 -I"$(PROJ_ROOT)/Libraries/Utils/inc" \
                 -I"$(PROJ_ROOT)/Libraries/SysFlash/inc" \
                 -I"$(PROJ_ROOT)/BSP/quickfeather/inc" \
                 -I"$(PROJ_ROOT)/Libraries/Utils/inc" \
                 -I"$(PROJ_ROOT)/Libraries/FPGA/inc"\
                 -I"$(PROJ_ROOT)/Libraries/DatablockManager/inc" \
                 -I"$(PROJ_ROOT)/Tasks/DatablockProcessor/inc" \
                 -I"$(PROJ_ROOT)/Tasks/ADC/inc" 
    

# C compiler flags
export CFLAGS= $(MACROS) \
        -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
        ${DASH_O} $(OPT_FLAGS) -fmessage-length=0 -lm \
        -fsigned-char -ffunction-sections -fdata-sections  ${DASH_G} -std=c99 -MMD -MD -MP


export LD_FLAGS_1= -mcpu=cortex-m4 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	$(DASH_O) $(OPT_FLAGS) -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections \
	$(DASH_G) -T "$(PROJ_DIR)/quickfeather.ld" -Xlinker --gc-sections -Wall -Werror \
	-Wl,--fatal-warnings -Wl,-Map,"$(OUTPUT_PATH)/$(OUTPUT_FILE).map" \
    --specs=nano.specs -u _printf_float --specs=nosys.specs -Wl,--no-wchar-size-warning \
    -o "$(OUTPUT_PATH)/$(OUTPUT_FILE).elf" \
	-L$(LIBCMSIS_GCC_DIR) -L$(LIBSENSIML_DIR) -lsensiml -lm -larm_cortexM4lf_math


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
export FPGA_DIR       = $(LIB_DIR)$(DIR_SEP)FPGA$(DIR_SEP)src
export CLI_DIR        = $(LIB_DIR)$(DIR_SEP)cli$(DIR_SEP)src
export QLFS_DIR       = $(LIB_DIR)$(DIR_SEP)QLFS$(DIR_SEP)src
export MQTTSN_DIR     = $(LIB_DIR)$(DIR_SEP)MQTTSN$(DIR_SEP)src
export MQTTSN_SML_DIR = $(LIB_DIR)$(DIR_SEP)MQTTSN_SML$(DIR_SEP)src
export MAIN_DIR       = $(APP_DIR)$(DIR_SEP)src
export FREERTOS_FAT_DIR  = $(LIB_DIR)$(DIR_SEP)FreeRTOS_FAT
export FREERTOS_FAT_COMMON_DIR  = $(LIB_DIR)$(DIR_SEP)FreeRTOS_FAT$(DIR_SEP)portable$(DIR_SEP)common
export FREERTOS_FAT_QL_DIR  = $(LIB_DIR)$(DIR_SEP)FreeRTOS_FAT$(DIR_SEP)portable$(DIR_SEP)QL
export RIFF_FILE_DIR  = $(LIB_DIR)$(DIR_SEP)riff_file$(DIR_SEP)src
export DBP_DIR         = $(PROJ_ROOT)$(DIR_SEP)Tasks$(DIR_SEP)DatablockProcessor$(DIR_SEP)src
export DBM_DIR        = $(LIB_DIR)$(DIR_SEP)DatablockManager$(DIR_SEP)src
export SENSOR_DIR       = $(LIB_DIR)$(DIR_SEP)SensorFramework$(DIR_SEP)src
export FRAMEWORKLIB_DIR = $(LIB_DIR)$(DIR_SEP)SensorFramework$(DIR_SEP)lib
export FFE_DIR          = $(LIB_DIR)$(DIR_SEP)SensorFramework$(DIR_SEP)drivers$(DIR_SEP)FFE
export HYBRID_DIR       = $(LIB_DIR)$(DIR_SEP)SensorFramework$(DIR_SEP)drivers$(DIR_SEP)Hybrid
export M4_DIR           = $(LIB_DIR)$(DIR_SEP)SensorFramework$(DIR_SEP)drivers$(DIR_SEP)M4$(DIR_SEP)mc3635
export ADC_DIR          = $(PROJ_ROOT)$(DIR_SEP)Tasks$(DIR_SEP)ADC$(DIR_SEP)src
ifeq ($(RTL_TOP_MODULE),usb2serial)
#export MAIN_FPGA_RTL_DIR	= $(PROJ_ROOT)$(DIR_SEP)s3-gateware$(DIR_SEP)usb2serial$(DIR_SEP)rtl
export MAIN_FPGA_SRC_DIR	= $(PROJ_ROOT)$(DIR_SEP)s3-gateware$(DIR_SEP)usb2serial$(DIR_SEP)src
else ($(RTL_TOP_MODULE),top)
export MAIN_FPGA_RTL_DIR	= $(PROJ_ROOT)$(DIR_SEP)s3-gateware$(DIR_SEP)adc_ad7476_if$(DIR_SEP)rtl
export MAIN_FPGA_SRC_DIR	= $(PROJ_ROOT)$(DIR_SEP)s3-gateware$(DIR_SEP)adc_ad7476_if$(DIR_SEP)rtl
endif
export S3GW_DRIVERS_DIR = $(PROJ_ROOT)$(DIR_SEP)freertos_gateware$(DIR_SEP)src
export SENSOR_AD7476_DIR = $(APP_DIR)$(DIR_SEP)ad7476$(DIR_SEP)src
export SENSOR_AUDIO_DIR = $(APP_DIR)$(DIR_SEP)sensor_audio$(DIR_SEP)src
