#!/bin/bash

# this script should be sourced, not run, so that all commands run in current shell process.
if [ "${BASH_SOURCE[0]}" -ef "$0" ]
then
    printf "This script should be sourced, not executed!\n"
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


QORC_SDK_ENVSETUP_VER=1.5.0

GIT_REPO_URL_EXPECTED_LOWERCASE=https://github.com/quicklogic-corp/qorc-sdk.git
GIT_REPO_URL_EXPECTED_LOWERCASE_ALT=https://github.com/quicklogic-corp/qorc-sdk

ARM_TOOLCHAIN_ARCHIVE_FILE=gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
ARM_TOOLCHAIN_INSTALL_DIR=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update
EXPECTED_ARM_TOOLCHAIN_GCC_PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc


FPGA_TOOLCHAIN_INSTALLER=Symbiflow_v1.3.1.gz.run
FPGA_TOOLCHAIN_INSTALL_DIR=${PWD}/fpga_toolchain_install/v1.3.1
EXPECTED_FPGA_TOOLCHAIN_QLSYMBIFLOW_OUTPUT=""


FLASH_PROGRAMMER_INSTALL_DIR=${PWD}/TinyFPGA-Programmer-Application
EXPECTED_FLASH_PROGRAMMER_QFPROG_OUTPUT=""


printf "\n\n=========================\n"
printf "qorc-sdk envsetup %s\n" ${QORC_SDK_ENVSETUP_VER}
printf "=========================\n\n\n"


printf "executing envsetup.sh from:\n%s\n" ${PWD}


#---------------------------------------------------------
printf "\n[0] are we inside qorc-sdk?\n"

# there seems to be strange behaviour on certain systems, the URL obtained below
# seems to have some letter lowercase, whereas on most, it is as it should be.
# to avoid this stuff we do all lowercase only comparison, use "tr" to do this.
GIT_REPO_URL_LOWERCASE=`git config --get remote.origin.url | tr '[:upper:]' '[:lower:]'`

if [ ! "$GIT_REPO_URL_LOWERCASE" = "$GIT_REPO_URL_EXPECTED_LOWERCASE" ]; then

    if [ ! "$GIT_REPO_URL_LOWERCASE" = "$GIT_REPO_URL_EXPECTED_LOWERCASE_ALT" ]; then

        printf "This script should be executed from within the qorc-sdk directory!\n"
        return

    fi

fi

# # we can also add check for if we are in the git repo root as well...
# # this is probably overkill, we will enable if needed in the future.
# GIT_REPO_GIT_DIR=`git rev-parse --git-dir`

# # if we are in root, we should get ".git", else we will get different path, should cd to that.

# if [ ! "$GIT_REPO_GIT_DIR" = ".git" ]; then

#     printf "We are not in repo root, cd to repo root dir: [%s]\n" $GIT_REPO_GIT_DIR/..

#     cd $GIT_REPO_GIT_DIR/..

# fi

printf "    ok.\n"
#---------------------------------------------------------


#---------------------------------------------------------
printf "\n[1] check (minimal) qorc-sdk submodules\n"

git submodule update --init --recursive qorc-example-apps
git submodule update --init --recursive qorc-testapps
git submodule update --init --recursive s3-gateware
git submodule update --init --recursive TinyFPGA-Programmer-Application

printf "    ok.\n"
#---------------------------------------------------------


#---------------------------------------------------------
# ARM GCC TOOLCHAIN
#---------------------------------------------------------
printf "\n[2] check arm gcc toolchain\n"
if [ ! -d $ARM_TOOLCHAIN_INSTALL_DIR ]; then

    printf "    creating toolchain directory : %s\n" "${PWD}/arm_toolchain_install"
    mkdir arm_toolchain_install

    if [ ! -f $ARM_TOOLCHAIN_ARCHIVE_FILE ]; then

        printf "    downloading toolchain archive.\n"
        wget -O gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -q --show-progress --progress=bar:force 2>&1 "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118"

    fi

    printf "    extracting toolchain archive.\n"
    tar xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C ${PWD}/arm_toolchain_install

fi


printf "    initializing toolchain.\n"
export PATH=$ARM_TOOLCHAIN_INSTALL_DIR/bin:$PATH
export QORC_TC_PATH=$ARM_TOOLCHAIN_INSTALL_DIR/bin


ACTUAL_ARM_TOOLCHAIN_GCC_PATH=`which arm-none-eabi-gcc`

if [ $ACTUAL_ARM_TOOLCHAIN_GCC_PATH == $EXPECTED_ARM_TOOLCHAIN_GCC_PATH ]; then

    printf "    ok.\n"

else

    printf "    !!ARM GCC Toolchain path not as expected, are there multiple toolchains on the path?!!\n"
    return

fi
#---------------------------------------------------------


#---------------------------------------------------------
# FPGA TOOLCHAIN
#---------------------------------------------------------
printf "\n[3] check fpga toolchain\n"

if [ ! -d $FPGA_TOOLCHAIN_INSTALL_DIR ]; then

    if [ ! -f $FPGA_TOOLCHAIN_INSTALLER ]; then

        printf "    downloading toolchain installer.\n"
        wget -O $FPGA_TOOLCHAIN_INSTALLER  -q --show-progress --progress=bar:force 2>&1 https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run

    fi

    export INSTALL_DIR=$FPGA_TOOLCHAIN_INSTALL_DIR
    printf "    installing toolchain.\n"
    bash Symbiflow_v1.3.1.gz.run

fi

printf "    initializing toolchain.\n"
export PATH="$FPGA_TOOLCHAIN_INSTALL_DIR/quicklogic-arch-defs/bin:$FPGA_TOOLCHAIN_INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
source "$FPGA_TOOLCHAIN_INSTALL_DIR/conda/etc/profile.d/conda.sh"
conda activate

# ql_symbiflow -h
# TODO check expected output to verify
printf "    ok.\n"
#---------------------------------------------------------


#---------------------------------------------------------
# TinyFPGA PROGRAMMER APPLICATION
#---------------------------------------------------------
printf "\n[4] check flash programmer\n"

if [ ! -d $FLASH_PROGRAMMER_INSTALL_DIR ]; then

    printf "    downloading flash programmer.\n"
    git clone --recursive https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git

fi


IS_TINYFPGAB_INSTALLED=`python3 -c 'import pkgutil; print(1 if pkgutil.find_loader("tinyfpgab") else 0)'`
if [ ! $IS_TINYFPGAB_INSTALLED == "1" ]; then

    printf "    setting up tinyfpgab.\n"
    pip3 install tinyfpgab

fi

IS_APIO_INSTALLED=`python3 -c 'import pkgutil; print(1 if pkgutil.find_loader("apio") else 0)'`
if [ ! $IS_APIO_INSTALLED == "1" ]; then

    printf "    setting up drivers.\n"
    pip3 install apio
    apio drivers --serial-enable

fi

printf "    initializing flash programmer.\n"
alias qfprog="python3 $FLASH_PROGRAMMER_INSTALL_DIR/tinyfpga-programmer-gui.py"

# qfprog --help
# TODO check expected output to verify
printf "    ok.\n"
#---------------------------------------------------------


#---------------------------------------------------------
printf "\n\nqorc-sdk env initialized.\n\n\n"
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
#         version : yyyyendif #QORC_TC_PATH

#---------------------------------------------------------

# references for choices made while writing the sh script:
# https://unix.stackexchange.com/questions/424492/how-to-define-a-shell-script-to-be-sourced-not-run
# https://stackoverflow.com/a/2237103/3379867
# https://unix.stackexchange.com/questions/65803/why-is-printf-better-than-echo/65819
# https://tldp.org/LDP/Bash-Beginners-Guide/html/sect_08_02.html

