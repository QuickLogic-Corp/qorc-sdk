# simple makefile wrapper for QORC SDK project FPGA build
SHELL:=bash

# for all targets, check whether the QORC SDK environment has been setup first.
ifndef QORC_SDK_PATH
$(info )
$(info QORC_SDK_PATH is not set in the environment, is the QORC SDK initialized?)
$(info initialize with: 'source envsetup.sh' from the QORC SDK directory in the current shell)
$(info )
$(error QORC_SDK_PATH not defined)
endif


# we should be squatting in the 'fpga' dir
export QORC_FPGA_DIR=$(shell pwd)
export QORC_FPGA_RTL_DIR=$(QORC_FPGA_DIR)/rtl


# "top" module name : change as per the project
export QORC_FPGA_TOP_MODULE=AL4S3B_FPGA_top

# PD64/PU64/WR42 : choose the package as per the project
export QORC_FPGA_PACKAGE=PU64

# if there are multiple .pcf files, we take the first one by default
# if this is already defined by user in the current environment, we use that instead.
export QORC_FPGA_PCF_FILE?=$(notdir $(firstword $(wildcard $(QORC_FPGA_RTL_DIR)/*.pcf)))

# device is fixed, ql-eos-s3
export QORC_FPGA_DEVICE=ql-eos-s3

# get the RTL source file (paths) - all verilog files in the fpga/rtl directory
FPGA_RTL_SRC_PATHS:=$(wildcard $(QORC_FPGA_RTL_DIR)/*.v)
# file-names only
FPGA_RTL_SRCS:=$(notdir $(FPGA_RTL_SRC_PATHS))
# filter-out some files, enable if needed
#filters=
#FPGA_RTL_SRCS:=$(filter-out $(filters),$(FPGA_RTL_SRCS))
ifeq ($(FPGA_RTL_SRCS),)
$(error No FPGA RTL Sources Found in $(QORC_FPGA_RTL_DIR))
else
# define the target for the build as the generated FPGA binary
FPGA_RTL_BIN_OBJ:=$(QORC_FPGA_TOP_MODULE).bin
endif

$(info )
$(info top: $(QORC_FPGA_TOP_MODULE))
$(info rtl-dir: $(QORC_FPGA_RTL_DIR))
$(info rtl-file-names: $(FPGA_RTL_SRCS))
#$(info rtl-file-paths: $(FPGA_RTL_SRC_PATHS))
$(info pcf: $(QORC_FPGA_PCF_FILE))
#$(info bin : $(FPGA_RTL_BIN_OBJ))
$(info )

.PHONY: all
all: $(FPGA_RTL_BIN_OBJ)

$(FPGA_RTL_BIN_OBJ): $(FPGA_RTL_SRC_PATHS) $(QORC_FPGA_RTL_DIR)/$(QORC_FPGA_PCF_FILE)
	@time ql_symbiflow -compile -src $(QORC_FPGA_RTL_DIR) -d $(QORC_FPGA_DEVICE) -t $(QORC_FPGA_TOP_MODULE) -v $(FPGA_RTL_SRCS) -p $(QORC_FPGA_PCF_FILE) -P $(QORC_FPGA_PACKAGE) -dump binary openocd jlink



.PHONY: clean
clean:
	@rm -r $(QORC_FPGA_RTL_DIR)/build 				2> /dev/null || true
	@rm $(QORC_FPGA_RTL_DIR)/*bit.h 				2> /dev/null || true
	@rm $(QORC_FPGA_RTL_DIR)/*.bin 					2> /dev/null || true
	@rm $(QORC_FPGA_RTL_DIR)/*.jlink 				2> /dev/null || true
	@rm $(QORC_FPGA_RTL_DIR)/*.openocd 				2> /dev/null || true
	@rm $(QORC_FPGA_RTL_DIR)/Makefile.symbiflow 	2> /dev/null || true
