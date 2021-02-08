#
# Environment Configuration options for qorc-sdk
#

# By default, many commands are hidden
# To enable VERBOSE mode, there are TWO options
# both are enabled via the command line:
# Option 1: "make HIDE="
# Option 2: "make v=1"
HIDE ?=@
ifeq (x"${v}",x"1")
override HIDE=
endif

export HIDE
# send the "v=<value>" to sub-makefiles.
ifeq (x"${HIDE}",x"")
v=1
export v
endif

# Set the build environment - WINCMD for Windows
# For Linux, comment this line out

# Auto-Detect Windows vrs Linux
# Note: $(SHELL) on windows is not always "cmd.exe"
# But - on windows the variables ProgramFiles and APPDATA exist
# so if ether exist (ie: non-blank) then we are on windows.
ifneq (,${ProgramFiles}${APPDATA})
$(info "shell Windows cmd.exe")
#SHELL :=cmd.exe
DEVNUL := NUL
WHICH := where
BUILD_SYS=WINCMD
else
$(info "shell Bash")
SHELL=bash
DEVNUL := /dev/null
WHICH := which
endif

_tmp_=$(findstring /cygdrive,${PATH})
# Exclude CYGWIN Make it is not compatible.
ifeq (/cygdrive,${_tmp_})
# CelerisStudioIDE installs Cygwin Make in your path
# And... Numerous other IDEs also use Cygwin Make
# it is not compatible with this, so stop here.
$(error This makefile does not work with CYGWIN make)
endif

#Set the toolchain (IAR/GCC)
export TOOLCHAIN=GCC

ifeq ($(TOOLCHAIN),IAR)
$(info Building using $(TOOLCHAIN) Toolchain)
include config-IAR.mk
else ifeq  ($(TOOLCHAIN),GCC)
$(info Building using $(TOOLCHAIN) Toolchain)
include config-GCC.mk
else
$(error Invalid Toolchain $(TOOLCHAIN))
exit
endif

################ Windows ###################
ifeq ($(BUILD_SYS),WINCMD)
################
$(info Building on Windows $(BUILD_SYS))

export DIR_SEP=\\


#Configuration options for GNU Win32 GCC Toolchain
export MKDIR=mkdir
export RM=del /S /Q
export LS=dir
export CP=copy
export MV=move
export ECHO=echo
#export PROJ_DIR=$(shell $(ECHO) %cd%)
# GNU make - abspath results in forward slashes
_tmp=$(abspath .)
# We require DOS style slashes, convert
_tmp2=$(subst /,\,${_tmp})
# Resolve this once, now, thus we assign with := not =
export PROJ_DIR := ${_tmp2}

$(info PROJ_DIR = ${PROJ_DIR})
export PROJ_ROOT=$(PROJ_DIR)\..\..\..
export OUTPUT_PATH=output
export DEPEND_PATH=output\depend
#COMPILER_LIBS_PATH=C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.4\arm\CMSIS\Lib\IAR

export APP_DIR = $(subst \GCC_Project,,${PROJ_DIR})
TMPVAR = $(subst \, ,${APP_DIR})
PROJ_NAME=$(word $(words ${TMPVAR}),${TMPVAR})
export PROJ_NAME

ifndef QORC_TC_PATH
FIND_TOOL_DIR := $(shell where arm-none-eabi-gcc)
ifndef FIND_TOOL_DIR
$(info using recursive search)
FIND_TOOL_DIR := $(shell where /r c:\progra~2 arm-none-eabi-gcc)
endif
endif #QORC_TC_PATH

ifdef FIND_TOOL_DIR
export QORC_TC_PATH = $(subst \arm-none-eabi-gcc.exe,,$(FIND_TOOL_DIR))
endif

#Override with your own tool direcoty
#export QORC_TC_PATH=C:\Program Files (x86)\GNU Tools ARM Embedded\7 2017-q4-major\bin
ifndef QORC_TC_PATH
$(info ######  ERROR - QORC_TC_PATH is not defined in config.mk #########)
exit
endif

export NM="$(QORC_TC_PATH)\arm-none-eabi-nm"
export LD="$(QORC_TC_PATH)\arm-none-eabi-gcc"
export AS="$(QORC_TC_PATH)\arm-none-eabi-gcc" -c
export CC="$(QORC_TC_PATH)\arm-none-eabi-gcc" -c
export ELF2BIN="$(QORC_TC_PATH)\arm-none-eabi-objcopy"
################
else
################ Linux ###################
$(info Using Linux GNU GCC Toolchain)

export DIR_SEP=/

#Configuration options for GNU Linux GCC Toolchain
export MKDIR=mkdir -p
export RM=-rm -f
export RMDIR=-rm -rf
export LS=ls
export CP=cp
export MV=mv
export ECHO=echo
export PROJ_DIR=$(shell pwd)
export PROJ_ROOT=$(PROJ_DIR)/../../..
export OUTPUT_PATH=$(PROJ_DIR)/output
export DEPEND_PATH=$(PROJ_DIR)/output/depend

export APP_DIR = $(subst /GCC_Project,,${PROJ_DIR})
TMPVAR = $(subst ${DIR_SEP}, ,${APP_DIR})
PROJ_NAME=$(word $(words ${TMPVAR}),${TMPVAR})
export PROJ_NAME

ifndef QORC_TC_PATH
FIND_TOOL_DIR := $(subst arm-none-eabi-gcc: ,,$(shell which arm-none-eabi-gcc))
export QORC_TC_PATH = $(subst /arm-none-eabi-gcc,,$(FIND_TOOL_DIR))
endif #QORC_TC_PATH

# Allow TOOL to be provided on the command line
# ie;   make -f Makefile QORC_TC_PATH=/some/path/

ifndef QORC_TC_PATH
#Override with your own tool directory
#use full path. do not use ~/ as a relative path
#export QORC_TC_PATH="~/arm-gnu/gcc-arm-none-eabi-7-2017-q4-major/bin"  <<<=== will not work
#export QORC_TC_PATH="/home/user_name/arm-gnu/gcc-arm-none-eabi-7-2017-q4-major/bin" <<<=== works
#export QORC_TC_PATH=/usr/local/gcc-arm-none-eabi-7-2017-q4-major/bin
endif

ifndef QORC_TC_PATH
$(info ######  ERROR - QORC_TC_PATH is not defined in config.mk #########)
exit
endif
export NM="$(QORC_TC_PATH)/arm-none-eabi-nm"
export LD="$(QORC_TC_PATH)/arm-none-eabi-gcc"
export AS="$(QORC_TC_PATH)/arm-none-eabi-gcc" -c
export CC="$(QORC_TC_PATH)/arm-none-eabi-gcc" -c
export ELF2BIN="$(QORC_TC_PATH)/arm-none-eabi-objcopy"
################
endif
################

$(info PROJ_NAME = ${PROJ_NAME})

#Ouput binary name
export OUTPUT_FILE=${PROJ_NAME}

#Libraries to include
export LIBS=
