#!/bin/bash

# this script should be sourced, not run, so that all commands run in current shell process.
if [ "${BASH_SOURCE[0]}" -ef "$0" ]
then
    printf "This script should be sourced, not executed!\n"
    exit 1
fi

# usage
# source "${QORC_SDK_PATH}/envsetup.sh"

# obtain the directory path of this script, which must be the qorc-sdk path
QORC_SDK_PATH=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export QORC_SDK_PATH

# save dir path from where the 'source' is being executed
CURRENT_DIR=$(pwd)

# move into the QORC_SDK_PATH
cd "$QORC_SDK_PATH"


# we want to have the structure as:
# qorc-sdk : ${PWD}
# - arm toolchain install at : ${PWD}/arm_toolchain_install/VERSION
# - fpga toolchain install at : ${PWD}/fpga_toolchain_install/VERSION
# - flash programmer install at : ${PWD}/TinyFPGA-Programmer-Application (skip this if adding as submodule!)
# - rest as is from the git repo.
# this will be a self contained installation with all tools internal to qorc-sdk repo directory.
# remember to add the stuff installed here into gitignore - so we don't have things clouding git status


QORC_SDK_ENVSETUP_VER=1.5.1

GIT_REPO_URL_EXPECTED_LOWERCASE=https://github.com/quicklogic-corp/qorc-sdk.git
GIT_REPO_URL_EXPECTED_LOWERCASE_ALT=https://github.com/quicklogic-corp/qorc-sdk

ARM_TOOLCHAIN_ARCHIVE_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118"
ARM_TOOLCHAIN_ARCHIVE_FILE="gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2"
ARM_TOOLCHAIN_INSTALL_BASE_DIR="${QORC_SDK_PATH}/arm_toolchain_install"
ARM_TOOLCHAIN_INSTALL_DIR="${ARM_TOOLCHAIN_INSTALL_BASE_DIR}/gcc-arm-none-eabi-9-2020-q2-update"
EXPECTED_ARM_TOOLCHAIN_GCC_PATH="${ARM_TOOLCHAIN_INSTALL_DIR}/bin/arm-none-eabi-gcc"


FPGA_TOOLCHAIN_INSTALLER_URL="https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run"
FPGA_TOOLCHAIN_INSTALLER_FILE="Symbiflow_v1.3.1.gz.run"
FPGA_TOOLCHAIN_INSTALLER_LOG="fpga_toolchain_install_v1.3.1.log"
FPGA_TOOLCHAIN_INSTALL_BASE_DIR="${QORC_SDK_PATH}/fpga_toolchain_install"
FPGA_TOOLCHAIN_INSTALL_DIR="${FPGA_TOOLCHAIN_INSTALL_BASE_DIR}/v1.3.1"
EXPECTED_FPGA_TOOLCHAIN_QLSYMBIFLOW_PATH="${FPGA_TOOLCHAIN_INSTALL_DIR}/quicklogic-arch-defs/bin/ql_symbiflow"


FLASH_PROGRAMMER_REPO_URL="https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git"
FLASH_PROGRAMMER_INSTALL_DIR="${QORC_SDK_PATH}/TinyFPGA-Programmer-Application"
EXPECTED_FLASH_PROGRAMMER_QFPROG_OUTPUT=""


printf "\n\n=========================\n"
printf "qorc-sdk envsetup %s\n" ${QORC_SDK_ENVSETUP_VER}
printf "=========================\n\n\n"


printf "executing envsetup.sh from:\n%s\n" ${QORC_SDK_PATH}

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

# if not installed, obtain archive and extract it
if [ ! -d "$ARM_TOOLCHAIN_INSTALL_DIR" ]; then

    printf "    creating arm toolchain directory : %s\n" "${ARM_TOOLCHAIN_INSTALL_BASE_DIR}"
    mkdir "$ARM_TOOLCHAIN_INSTALL_BASE_DIR"

    if [ ! -f "$ARM_TOOLCHAIN_ARCHIVE_FILE" ]; then

        printf "    downloading arm toolchain archive.\n"
        wget -O "$ARM_TOOLCHAIN_ARCHIVE_FILE" -q --show-progress --progress=bar:force 2>&1 "$ARM_TOOLCHAIN_ARCHIVE_URL"

    fi

    printf "    extracting arm toolchain archive.\n"
    tar -xf "$ARM_TOOLCHAIN_ARCHIVE_FILE" -C "${ARM_TOOLCHAIN_INSTALL_BASE_DIR}"

    rm "$ARM_TOOLCHAIN_ARCHIVE_FILE"

fi

# add toolchain to path, and set the QORC_TC_PATH to the toolchain dir
printf "    initializing arm toolchain.\n"
export QORC_TC_PATH="$ARM_TOOLCHAIN_INSTALL_DIR/bin"
export PATH="$ARM_TOOLCHAIN_INSTALL_DIR/bin:$PATH"

# check if expected toolchain is available in the env now
ACTUAL_ARM_TOOLCHAIN_GCC_PATH=`which arm-none-eabi-gcc`
if [ "$ACTUAL_ARM_TOOLCHAIN_GCC_PATH" == "$EXPECTED_ARM_TOOLCHAIN_GCC_PATH" ]; then

    printf "    ok.\n"

else

    printf "    !!ARM GCC Toolchain path not as expected, install/permission problems?!!\n"
    printf "    expected: %s\n" "$EXPECTED_ARM_TOOLCHAIN_GCC_PATH"
    printf "         got: %s\n" "$ACTUAL_ARM_TOOLCHAIN_GCC_PATH"
    return

fi
#---------------------------------------------------------


#---------------------------------------------------------
# FPGA TOOLCHAIN
#---------------------------------------------------------
printf "\n[3] check fpga toolchain\n"

# if not installed, obtain installer and run it
if [ ! -d "$FPGA_TOOLCHAIN_INSTALL_DIR" ]; then

    printf "    creating fpga toolchain directory : %s\n" "${FPGA_TOOLCHAIN_INSTALL_BASE_DIR}"
    mkdir "$FPGA_TOOLCHAIN_INSTALL_BASE_DIR"

    if [ ! -f "$FPGA_TOOLCHAIN_INSTALLER_FILE" ]; then

        printf "    downloading toolchain installer.\n"
        wget -O "$FPGA_TOOLCHAIN_INSTALLER_FILE"  -q --show-progress --progress=bar:force 2>&1 "$FPGA_TOOLCHAIN_INSTALLER_URL"

    fi

    printf "    installing toolchain.\n"
    export INSTALL_DIR="$FPGA_TOOLCHAIN_INSTALL_DIR"
    #bash "$FPGA_TOOLCHAIN_INSTALLER_FILE" > "${FPGA_TOOLCHAIN_INSTALL_BASE_DIR}/${FPGA_TOOLCHAIN_INSTALLER_LOG}" 2>&1 
    bash "$FPGA_TOOLCHAIN_INSTALLER_FILE"

    rm "$FPGA_TOOLCHAIN_INSTALLER_FILE"

fi

# add fpga toolchain to path and activate conda env
printf "    initializing fpga toolchain.\n"
export PATH="$FPGA_TOOLCHAIN_INSTALL_DIR/quicklogic-arch-defs/bin:$FPGA_TOOLCHAIN_INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
source "$FPGA_TOOLCHAIN_INSTALL_DIR/conda/etc/profile.d/conda.sh"
conda activate

# check if expected toolchain is available in the env now
ACTUAL_FPGA_TOOLCHAIN_QLSYMBIFLOW_PATH=`which ql_symbiflow`
if [ "$ACTUAL_FPGA_TOOLCHAIN_QLSYMBIFLOW_PATH" == "$EXPECTED_FPGA_TOOLCHAIN_QLSYMBIFLOW_PATH" ]; then

    printf "    ok.\n"

else

    printf "    !!fpga toolchain path not as expected, install/permission problems?!!\n"
    printf "    expected: %s\n" "$EXPECTED_FPGA_TOOLCHAIN_QLSYMBIFLOW_PATH"
    printf "         got: %s\n" "$ACTUAL_FPGA_TOOLCHAIN_QLSYMBIFLOW_PATH"
    return

fi
#---------------------------------------------------------


#---------------------------------------------------------
# TinyFPGA PROGRAMMER APPLICATION
#---------------------------------------------------------
printf "\n[4] check flash programmer\n"

if [ ! -d $FLASH_PROGRAMMER_INSTALL_DIR ]; then

    printf "    downloading flash programmer.\n"
    git clone --recursive "$FLASH_PROGRAMMER_REPO_URL"

fi


IS_TINYFPGAB_INSTALLED=$(python3 -c 'import pkgutil; print(1 if pkgutil.find_loader("tinyfpgab") else 0)')
if [ ! $IS_TINYFPGAB_INSTALLED == "1" ]; then

    printf "    setting up tinyfpgab.\n"
    pip3 install tinyfpgab

fi

IS_APIO_INSTALLED=$(python3 -c 'import pkgutil; print(1 if pkgutil.find_loader("apio") else 0)')
if [ ! $IS_APIO_INSTALLED == "1" ]; then

    printf "    setting up drivers.\n"
    pip3 install apio
    apio drivers --serial-enable

fi

printf "    initializing flash programmer.\n"
QORC_FLASH_PROGRAMMER_PATH="${FLASH_PROGRAMMER_INSTALL_DIR}/tinyfpga-programmer-gui.py"
export QORC_FLASH_PROGRAMMER_PATH
qfprog() {
    python3 "$QORC_FLASH_PROGRAMMER_PATH" "$@"
}
export -f qfprog

# check if expected programmer is available in the env now
qfprog -h > /dev/null
QFPROG_STATUS=$?

if [ $QFPROG_STATUS -eq 0 ] ; then

    printf "    ok.\n"

else

    printf "    flash programmer installation problem detected!\n"
    return

fi
#---------------------------------------------------------


#---------------------------------------------------------
printf "\n\nqorc-sdk build env initialized.\n\n\n"
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

