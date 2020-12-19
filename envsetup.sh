#!/bin/bash

# this script should be sourced, not run, so that all commands run in current shell process.
# needs python3 installed!
# cd ${QORC_SDK_DIR_PATH}
# source ./envsetup.sh

# TODOs
# intelligent decision of which steps are complete (optional) instead of just checking if file/dir exist
# using json or similar to store state, and allow ctrl-c, later resume from correct place. overkill ?
# all prints go to log...
# either we fail and show error on console, or pass and only show all ok print
# fpga toolchain verif step
# flash programmer verif step



QORC_SDK_QUICKSETUP_VER=1.5.0

# we want to have the structure as:
# qorc-sdk : ${PWD}
# - arm toolchain install at : ${PWD}/arm_toolchain_install/VERSION
# - fpga toolchain install at : ${PWD}/fpga_toolchain_install/VERSION
# - flash programmer install at : ${PWD}/TinyFPGA-Programmer-Application (skip this if adding as submodule!)
# - rest as is from the git repo.
# this will be a self contained installation with all tools internal to qorc-sdk repo directory.
# remember to add the stuff installed here into gitignore - so we don't have things clouding git status



ARM_TOOLCHAIN_ARCHIVE_FILE=gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
ARM_TOOLCHAIN_INSTALL_DIR=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update
EXPECTED_ARM_TOOLCHAIN_GCC_PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc


FPGA_TOOLCHAIN_INSTALLER=Symbiflow_v1.3.1.gz.run
FPGA_TOOLCHAIN_INSTALL_DIR=${PWD}/fpga_toolchain_install/v1.3.1


FLASH_PROGRAMMER_INSTALL_DIR=${PWD}/TinyFPGA-Programmer-Application


echo
echo
echo
echo "========================="
echo "qorc-sdk quicksetup "${QORC_SDK_QUICKSETUP_VER}
echo "========================="
echo
echo
echo


echo "PWD --> " ${PWD}

echo
echo "step 1 : update qorc-sdk submodules"

echo
echo "    done."


#---------------------------------------------------------
echo
echo "step 2.1 : download arm gcc toolchain archive and verify integrity"

if [ ! -f $ARM_TOOLCHAIN_ARCHIVE_FILE ]; then
    wget -O gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -q --show-progress --progress=bar:force 2>&1 "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118"
else
    echo
    echo "    arm gcc toolchain archive already exists"
fi
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 2.2 : extract arm gcc toolchain"

if [ ! -d $ARM_TOOLCHAIN_INSTALL_DIR ]; then
    mkdir arm_toolchain_install
    tar xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C ${PWD}/arm_toolchain_install
else
    echo
    echo "    arm gcc toolchain dir already exists"
fi
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 2.3 : export arm gcc toolchain to PATH"

export PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 2.4 : verify arm gcc toolchain"

ACTUAL_ARM_TOOLCHAIN_GCC_PATH=`which arm-none-eabi-gcc`

if [ $ACTUAL_ARM_TOOLCHAIN_GCC_PATH == $EXPECTED_ARM_TOOLCHAIN_GCC_PATH ]; then
    echo
    echo "    ok"
else
    echo
    echo "    !!GCC path not as expected, are there multiple toolchains on the path?!!"
    return
fi
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 3.1 : download fpga toolchain installer script"

if [ ! -f $ARM_TOOLCHAIN_ARCHIVE ]; then
    wget -O $FPGA_TOOLCHAIN_INSTALLER  -q --show-progress --progress=bar:force 2>&1 https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run
else
    echo
    echo "    fpga toolchain installer already exists"
fi
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 3.2 : run the fpga toolchain installer script"

export INSTALL_DIR=$FPGA_TOOLCHAIN_INSTALL_DIR
if [ ! -d $INSTALL_DIR ]; then
    bash Symbiflow_v1.3.1.gz.run
else
    echo
    echo "    fpga toolchain dir already exists"
fi
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 3.3 : initialize fpga toolchain environment"

export PATH="$INSTALL_DIR/quicklogic-arch-defs/bin:$INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
source "$INSTALL_DIR/conda/etc/profile.d/conda.sh"
conda activate
#---------------------------------------------------------


#---------------------------------------------------------
echo 
echo "step 3.4 : verify fpga toolchain"

#ql_symbiflow -h
# expected output?
echo
echo "    ok"
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 4.1 : install flash programmer - TinyFPGA-Programmer-Application"

if [ ! -d $FLASH_PROGRAMMER_INSTALL_DIR ]; then

    git clone --recursive https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git
    pip3 install tinyfpgab
    pip3 install apio
    apio drivers --serial-enable
else
    echo
    echo "    flash programmer dir already exists"
fi
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 4.2 : initialize flash programmer for use"

alias qfprog="python3 $FLASH_PROGRAMMER_INSTALL_DIR/tinyfpga-programmer-gui.py"
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo "step 4.3 : verify flash programmer"
#qfprog --help
#expected output ?
echo
echo "    ok"
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo
echo "qorc-sdk env initialized."
echo
echo
#---------------------------------------------------------



#---------------------------------------------------------
# FUTURE STUFF
#
# for each component, check install, if already done, initialize (if any)
# qorc-sdk
# arm gcc toolchain
# quicklogic fpga toolchain
# quicklogic tinyfpga programmer application

# check install : local file which tracks status, and we update the file as we proceed with the steps.
# use json ?
# components : {

#     qorc-sdk : {
        
#         download : true,
#         install : true,
#         timestamp : xxxxxx,
#         version : yyyy
        
#     },
    
#     arm-gcc-toolchain : {
        
#         download : true,
#         install : true,
#         timestamp : xxxxxx,
#         version : yyyy
        
#     },
    
#     quicklogic-fpga-toolchain : {
        
#         download : true,
#         install : true,
#         timestamp : xxxxxx,
#         version : yyyy
        
#     },
    
#     quicklogic-tinyfpga-programmer : {
        
#         download : true,
#         install : true,
#         timestamp : xxxxxx,
#         version : yyyy
        
#     }

# }
#---------------------------------------------------------