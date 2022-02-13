# simple makefile wrapper for MatrixIDE FPGA build
SHELL:=bash

# for all targets, check whether the MatrixIDE environment has been setup first.
ifndef AU_INSTALL_ROOT
$(info )
$(info AU_INSTALL_ROOT is not set in the environment, is MatrixIDE initialized?)
$(info initialize with: 'source setup_au.sh' from the MatrixIDE directory in the current shell)
$(info )
$(error AU_INSTALL_ROOT not defined)
endif


# we should be squatting in the 'fpga' dir
export QORC_FPGA_DIR=$(shell pwd)
export QORC_FPGA_RTL_DIR=$(QORC_FPGA_DIR)/rtl


# "top" module name : change as per the project
export QORC_FPGA_TOP_MODULE=AL4S3B_FPGA_top

# PD64/PU64/WR42 : choose the package as per the project
export QORC_FPGA_PACKAGE=PU64

# if there are multiple .qdc files, we take the first one by default
export QORC_FPGA_QDC_FILE=$(notdir $(firstword $(wildcard $(QORC_FPGA_RTL_DIR)/*.qdc)))
# or enable the line below to use fixed qdc
#export QORC_FPGA_QDC_FILE="helloworldfpga.qdc"

# device is fixed, RSMX3TFF512
export QORC_FPGA_DEVICE=RSMX3TFF512

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
FPGA_RTL_BIN_OBJ:=$(QORC_FPGA_RTL_DIR)/config_bit_gen/$(QORC_FPGA_DEVICE)_$(QORC_FPGA_TOP_MODULE).bin
endif

$(info )
$(info top: $(QORC_FPGA_TOP_MODULE))
$(info rtl-dir: $(QORC_FPGA_RTL_DIR))
$(info rtl-file-names: $(FPGA_RTL_SRCS))
#$(info rtl-file-paths: $(FPGA_RTL_SRC_PATHS))
$(info qdc: $(QORC_FPGA_QDC_FILE))
#$(info bin : $(FPGA_RTL_BIN_OBJ))
$(info )

.PHONY: all
all: $(FPGA_RTL_BIN_OBJ)
# the FPGA binary output should be rebuilt if verilog source, or qdc file changes.
$(FPGA_RTL_BIN_OBJ): $(FPGA_RTL_SRC_PATHS) $(QORC_FPGA_RTL_DIR)/$(QORC_FPGA_QDC_FILE)
	@{	\
		\
		# switch to the RTL sources dir before running MatrixIDE \
		cd $(QORC_FPGA_RTL_DIR) ; \
		\
		printf "running matrixide command:\n\n" ; \
		printf "matrixide --console --compile_design $(FPGA_RTL_SRCS) --top $(QORC_FPGA_TOP_MODULE) -o $(QORC_FPGA_QDC_FILE) -d $(QORC_FPGA_DEVICE) -k $(QORC_FPGA_PACKAGE) --run_all\n\n" ; \
		\
		time matrixide --console --compile_design $(FPGA_RTL_SRCS) --top $(QORC_FPGA_TOP_MODULE) -o $(QORC_FPGA_QDC_FILE) -d $(QORC_FPGA_DEVICE) -k $(QORC_FPGA_PACKAGE) --run_all ; \
	}



.PHONY: clean
clean:
	@rm -r "$(QORC_FPGA_RTL_DIR)"/config_bit_gen 		2> /dev/null || true
	@rm -r "$(QORC_FPGA_RTL_DIR)"/*_linux_cache 			2> /dev/null || true
	@rm -r "$(QORC_FPGA_RTL_DIR)"/logic_optimizer 		2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*_yosys.log 				2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.blif 					2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.edf 					2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.html 					2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.log 					2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.rpt 					2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.tcl 					2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.ys 						2> /dev/null || true
	@rm "$(QORC_FPGA_RTL_DIR)"/*.timing.db 				2> /dev/null || true
