#!/bin/bash

# this script should be sourced, not run, so that all commands run in current shell process.
# https://unix.stackexchange.com/questions/424492/how-to-define-a-shell-script-to-be-sourced-not-run
if [ "${BASH_SOURCE[0]}" -ef "$0" ]
then
    echo "This script should be sourced, not executed!"
    exit 1
fi


# cd ${QORC_SDK_DIR_PATH}
# source ./envsetup.sh

# TODOs
# intelligent decision of which steps are complete (optional) instead of just checking if file/dir exist
# using json or similar to store state, and allow ctrl-c, later resume from correct place. overkill ?
# all prints go to log...
# either we fail and show error on console, or pass and only show all ok print
# fpga toolchain verif step
# flash programmer verif step



# we want to have the structure as:
# qorc-sdk : ${PWD}
# - arm toolchain install at : ${PWD}/arm_toolchain_install/VERSION
# - fpga toolchain install at : ${PWD}/fpga_toolchain_install/VERSION
# - flash programmer install at : ${PWD}/TinyFPGA-Programmer-Application (skip this if adding as submodule!)
# - rest as is from the git repo.
# this will be a self contained installation with all tools internal to qorc-sdk repo directory.
# remember to add the stuff installed here into gitignore - so we don't have things clouding git status


QORC_SDK_QUICKSETUP_VER=1.5.0


ARM_TOOLCHAIN_ARCHIVE_FILE=gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
ARM_TOOLCHAIN_INSTALL_DIR=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update
EXPECTED_ARM_TOOLCHAIN_GCC_PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc


FPGA_TOOLCHAIN_INSTALLER=Symbiflow_v1.3.1.gz.run
FPGA_TOOLCHAIN_INSTALL_DIR=${PWD}/fpga_toolchain_install/v1.3.1
EXPECTED_FPGA_TOOLCHAIN_QLSYMBIFLOW_OUTPUT=""


FLASH_PROGRAMMER_INSTALL_DIR=${PWD}/TinyFPGA-Programmer-Application
EXPECTED_FLASH_PROGRAMMER_QFPROG_OUTPUT=""


echo
echo
echo "========================="
echo "qorc-sdk quicksetup "${QORC_SDK_QUICKSETUP_VER}
echo "========================="
echo
echo


echo "PWD --> " ${PWD}

#---------------------------------------------------------
echo
echo "[1] check qorc-sdk submodules"
# TODO add required submodules check and init!
echo "    all ok."
#---------------------------------------------------------


#---------------------------------------------------------
# ARM GCC TOOLCHAIN
#---------------------------------------------------------
echo
echo "[2] check arm gcc toolchain"
if [ ! -d $ARM_TOOLCHAIN_INSTALL_DIR ]; then
    
    echo "    creating toolchain directory : ${PWD}/arm_toolchain_install"
    mkdir arm_toolchain_install

    if [ ! -f $ARM_TOOLCHAIN_ARCHIVE_FILE ]; then

        echo "    downloading toolchain archive."
        wget -O gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -q --show-progress --progress=bar:force 2>&1 "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118"
    
    fi

    echo "    extracting toolchain archive."
    tar xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C ${PWD}/arm_toolchain_install

fi


echo "    initializing toolchain."
export PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH


ACTUAL_ARM_TOOLCHAIN_GCC_PATH=`which arm-none-eabi-gcc`

if [ $ACTUAL_ARM_TOOLCHAIN_GCC_PATH == $EXPECTED_ARM_TOOLCHAIN_GCC_PATH ]; then

    echo "    all ok."

else

    echo "    !!ARM GCC Toolchain path not as expected, are there multiple toolchains on the path?!!"
    return

fi
#---------------------------------------------------------



#---------------------------------------------------------
# FPGA TOOLCHAIN
#---------------------------------------------------------
echo
echo "[3] check fpga toolchain"

if [ ! -d $FPGA_TOOLCHAIN_INSTALL_DIR ]; then

    if [ ! -f $FPGA_TOOLCHAIN_INSTALLER ]; then

        echo "    downloading toolchain installer."
        wget -O $FPGA_TOOLCHAIN_INSTALLER  -q --show-progress --progress=bar:force 2>&1 https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run
    
    fi

    export INSTALL_DIR=$FPGA_TOOLCHAIN_INSTALL_DIR
    echo "    installing toolchain."
    bash Symbiflow_v1.3.1.gz.run

fi

echo "    initializing toolchain."
export PATH="$FPGA_TOOLCHAIN_INSTALL_DIR/quicklogic-arch-defs/bin:$FPGA_TOOLCHAIN_INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
source "$FPGA_TOOLCHAIN_INSTALL_DIR/conda/etc/profile.d/conda.sh"
conda activate

# ql_symbiflow -h
# TODO check expected output to verify
echo "    all ok."
#---------------------------------------------------------


#---------------------------------------------------------
# TinyFPGA PROGRAMMER APPLICATION
#---------------------------------------------------------
echo
echo "[4] check flash programmer"

if [ ! -d $FLASH_PROGRAMMER_INSTALL_DIR ]; then

    echo "    downloading flash programmer."
    git clone --recursive https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git

fi


IS_TINYFPGAB_INSTALLED=`python3 -c 'import pkgutil; print(1 if pkgutil.find_loader("tinyfpgab") else 0)'`
if [ ! $IS_TINYFPGAB_INSTALLED == "1" ]; then

    echo "    setting up tinyfpgab."
    pip3 install tinyfpgab

fi

IS_APIO_INSTALLED=`python3 -c 'import pkgutil; print(1 if pkgutil.find_loader("apio") else 0)'`
if [ ! $IS_APIO_INSTALLED == "1" ]; then

    echo "    setting up drivers."
    pip3 install apio
    apio drivers --serial-enable

fi

echo "    initializing flash programmer."
alias qfprog="python3 $FLASH_PROGRAMMER_INSTALL_DIR/tinyfpga-programmer-gui.py"

# qfprog --help
# TODO check expected output to verify
echo "    all ok."
#---------------------------------------------------------


#---------------------------------------------------------
echo
echo
echo "qorc-sdk env initialized."
echo
echo
#---------------------------------------------------------



#---------------------------------------------------------
# FUTURE STUFF - maybe overkill for now.
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