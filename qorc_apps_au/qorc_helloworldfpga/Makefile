# simple makefile wrapper for QORC SDK projects
SHELL:=bash

# for all targets, check whether the QORC SDK environment has been setup first.
ifndef QORC_SDK_PATH
$(info )
$(info QORC_SDK_PATH is not set in the environment, is the QORC SDK initialized?)
$(info initialize with: 'source envsetup.sh' from the QORC SDK directory in the current shell)
$(info )
$(error QORC_SDK_PATH not defined)
endif

# get the path of the directory where this makefile resides
MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR_PATH := $(patsubst %/,%,$(dir $(MAKEFILE_PATH)))
MAKEFILE_DIR_NAME := $(notdir $(MAKEFILE_DIR_PATH))
# $(info )
# $(info MAKEFILE_LIST=$(MAKEFILE_LIST))
# $(info MAKEFILE_PATH=$(MAKEFILE_PATH))
# $(info MAKEFILE_DIR_PATH=$(MAKEFILE_DIR_PATH))
# $(info MAKEFILE_DIR_NAME=$(MAKEFILE_DIR_NAME))
# $(info )

# get the expected paths of the 'GCC_Project' and 'fpga' directories
# the existence of these indicate whether m4 code exists, and fpga design exists
PROJECT_FPGA_DIR:=$(MAKEFILE_DIR_PATH)/fpga
PROJECT_M4_DIR:=$(MAKEFILE_DIR_PATH)/GCC_Project
$(info )
$(info PROJECT_FPGA_DIR=$(PROJECT_FPGA_DIR))
$(info PROJECT_M4_DIR=$(PROJECT_M4_DIR))
$(info )
ifneq ($(wildcard $(PROJECT_FPGA_DIR)),)
$(info fpga design exists)
endif
ifneq ($(wildcard $(PROJECT_M4_DIR)),)
$(info m4 code exists)
endif
$(info )
#$(error uncomment for using make to print info only and stop)


.PHONY: all
all: fpga m4


.PHONY: clean
clean: clean-fpga clean-m4


.PHONY: m4
# execute only if m4-code exists
ifneq ($(wildcard $(PROJECT_M4_DIR)),)
m4:
	@$(MAKE) -C GCC_Project
endif


.PHONY: clean-m4
# execute only if m4-code exists
ifneq ($(wildcard $(PROJECT_M4_DIR)),)
clean-m4:
	@$(MAKE) -C GCC_Project clean
endif


.PHONY: fpga
# execute only if fpga-design exists
ifneq ($(wildcard $(PROJECT_FPGA_DIR)),)
fpga:
	@$(MAKE) -C fpga
endif


.PHONY: clean-fpga
# execute only if fpga-design exists
ifneq ($(wildcard $(PROJECT_FPGA_DIR)),)
clean-fpga:
	@$(MAKE) -C fpga clean
endif


.PHONY: load-jlink
load-jlink:
	.scaffolding/load_fpga_m4_jlink.sh


# QORC_OCD_IF_CFG is used, to be able to use the correct (interface) debug adapter config in OpenOCD
# 1. set QORC_OCD_IF_CFG=/path/to/cfg in shell env, before invoking make load-openocd (recommended, needed only once)
# or
# 2. pass in value while invoking: make load-openocd QORC_OCD_IF_CFG=/path/to/cfg
# default fallback, if not specified, is to assume a JLink adapter, with the cfg file in the .scaffolding dir
QORC_OCD_IF_CFG?=
.PHONY: load-openocd
load-openocd:
	@{ \
	if [ -z $$QORC_OCD_IF_CFG ] ; then \
		printf "\nERROR: QORC_OCD_IF_CFG not defined\n\n" ;\
		printf "    use: 'export QORC_OCD_IF_CFG=/path/to/cfg', before invoking make $@\n" ;\
		printf "    >>OR<<\n" ;\
		printf "    use 'make $@ QORC_OCD_IF_CFG=/port/name'\n\n" ;\
		exit 1 ;\
	else \
		.scaffolding/load_fpga_m4_openocd_gdb.sh --openocd-if-cfg=$(QORC_OCD_IF_CFG) ;\
	fi \
	}


# QORC_PORT is used, to be able to flash using the correct serial port.
# 1. set QORC_PORT in shell env, before invoking make flash (recommended, needed only once)
# or
# 2. pass in the value while invoking make flash like so: make flash QORC_PORT=/dev/ttyPORTNAME
QORC_PORT?=
.PHONY: flash
flash:
	@{ \
	if [ -z $$QORC_PORT ] ; then \
		printf "\nERROR: QORC_PORT not defined\n\n" ;\
		printf "    use: 'export QORC_PORT=/port/name', before invoking make $@\n" ;\
		printf "    >>OR<<\n" ;\
		printf "    use 'make $@ QORC_PORT=/port/name'\n\n" ;\
		exit 1 ;\
	else \
		.scaffolding/flash_fpga_m4.sh --port=$(QORC_PORT) ;\
	fi \
	}
