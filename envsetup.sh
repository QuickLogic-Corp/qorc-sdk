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

# move into the QORC_SDK_PATH, if not already in it:
if [ ! "$CURRENT_DIR" == "$QORC_SDK_PATH" ] ; then
    cd "$QORC_SDK_PATH"
fi


# structure :
# qorc-sdk : ${QORC_SDK_PATH}
# - arm toolchain install at : ${QORC_SDK_PATH}/arm_toolchain_install/VERSION
# - fpga toolchain install at : ${QORC_SDK_PATH}/fpga_toolchain_install/VERSION
# - flash programmer install at : ${QORC_SDK_PATH}/TinyFPGA-Programmer-Application (skip this if adding as submodule!)
# - jlink tooling install at : ${QORC_SDK_PATH}/jlink_install
# - openocd tooling install at : ${QORC_SDK_PATH}/openocd_install
# - rest as is from the git repo.
# this will be a self contained installation with all tools internal to qorc-sdk repo directory.


QORC_SDK_ENVSETUP_VER=1.5.1

GIT_REPO_OWNER="quicklogic-corp"
GIT_REPO_NAME="qorc-sdk"


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


# OpenOCD: https://github.com/xpack-dev-tools/openocd-xpack/releases/ -> get the linux-x64 archive from this page
# also, some systems may need udev rules depending on debug probe used, put into /etc/udev/rules -> this is upto the end user.
OPENOCD_ARCHIVE_URL="https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v0.11.0-3/xpack-openocd-0.11.0-3-linux-x64.tar.gz"
OPENOCD_ARCHIVE_FILE="xpack-openocd-0.11.0-3-linux-x64.tar.gz"
OPENOCD_INSTALL_BASE_DIR="${QORC_SDK_PATH}/openocd_install"
OPENOCD_INSTALL_DIR="${OPENOCD_INSTALL_BASE_DIR}/xpack-openocd-0.11.0-3"
EXPECTED_OPENOCD_PATH="${OPENOCD_INSTALL_DIR}/bin/openocd"

# JLink: https://www.segger.com/downloads/jlink/ -> get the Linux/'64-bit TGZ Archive' from this page
# note that there is a EULA which needs a POST request, as described here: https://github.com/ScoopInstaller/Scoop/issues/4336
# also, some systems may need the udev rules from the JLink archive put into /etc/udev/rules -> this is upto the end user.
JLINK_ARCHIVE_URL="https://www.segger.com/downloads/jlink/JLink_Linux_V760d_x86_64.tgz"
JLINK_ARCHIVE_FILE="JLink_Linux_V760d_x86_64.tgz"
JLINK_INSTALL_BASE_DIR="${QORC_SDK_PATH}/jlink_install"
JLINK_INSTALL_DIR="${JLINK_INSTALL_BASE_DIR}/JLink_Linux_V760d_x86_64"
EXPECTED_JLINK_GDBSERVER_PATH="${JLINK_INSTALL_DIR}/JLinkGDBServerCLExe"



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
    mkdir -p "$ARM_TOOLCHAIN_INSTALL_BASE_DIR"

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
    mkdir -p "$FPGA_TOOLCHAIN_INSTALL_BASE_DIR"

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
# OpenOCD
#---------------------------------------------------------
printf "\n[5] check openocd\n"
if [ ! -d "$OPENOCD_INSTALL_DIR" ]; then

    printf "    creating openocd directory : %s\n" "${OPENOCD_INSTALL_BASE_DIR}"
    mkdir "${OPENOCD_INSTALL_BASE_DIR}"

    if [ ! -f "$OPENOCD_ARCHIVE_FILE" ]; then

        printf "    downloading openocd archive.\n"
        wget -O "$OPENOCD_ARCHIVE_FILE" -q --show-progress --progress=bar:force 2>&1 "$OPENOCD_ARCHIVE_URL"

    fi

    printf "    extracting openocd archive.\n"
    tar -xf "$OPENOCD_ARCHIVE_FILE" -C "${OPENOCD_INSTALL_BASE_DIR}"

    rm "$OPENOCD_ARCHIVE_FILE"

    if [ ! -f "$EXPECTED_OPENOCD_PATH" ]; then
        printf "\n    ERROR: openocd installation problem detected!\n"
        return 1
    else
        printf "    openocd installation looks good\n"
    fi

fi

# add OpenOCD to path
export PATH="${OPENOCD_INSTALL_DIR}/bin:$PATH"
ACTUAL_OPENOCD_GDBSERVER_PATH=`which openocd`
if [ $ACTUAL_OPENOCD_GDBSERVER_PATH == $EXPECTED_OPENOCD_PATH ]; then

    printf "    ok.\n"

else

    printf "    !!openocd path not as expected, install/permission problems?!!\n"
    printf "    expected: %s\n" ${EXPECTED_OPENOCD_PATH}
    printf "         got: %s\n" ${ACTUAL_OPENOCD_GDBSERVER_PATH}
    return

fi
#---------------------------------------------------------


#---------------------------------------------------------
# JLink
#---------------------------------------------------------
printf "\n[6] check jlink\n"
if [ ! -d "$JLINK_INSTALL_DIR" ]; then

    printf "    creating jlink directory : %s\n" "${JLINK_INSTALL_BASE_DIR}"
    mkdir "${JLINK_INSTALL_BASE_DIR}"

    if [ ! -f "$JLINK_ARCHIVE_FILE" ]; then

        printf "    downloading jlink archive.\n"
        # https://github.com/ScoopInstaller/Scoop/issues/4336
        wget --post-data "accept_license_agreement=accepted" -O "$JLINK_ARCHIVE_FILE" -q --show-progress --progress=bar:force 2>&1 "$JLINK_ARCHIVE_URL"

    fi

    printf "    extracting jlink archive.\n"
    tar -xf "$JLINK_ARCHIVE_FILE" -C "${JLINK_INSTALL_BASE_DIR}"

    rm "$JLINK_ARCHIVE_FILE"

    if [ ! -f "$EXPECTED_JLINK_GDBSERVER_PATH" ]; then
        printf "\n    ERROR: jlink installation problem detected!\n"
    else
        printf "    jlink installation looks good\n"
    fi

fi

# add JLink to path
export PATH="${JLINK_INSTALL_DIR}:$PATH"
ACTUAL_JLINK_GDBSERVER_PATH=`which JLinkGDBServerCLExe`
if [ $ACTUAL_JLINK_GDBSERVER_PATH == $EXPECTED_JLINK_GDBSERVER_PATH ]; then

    printf "    ok.\n"

else

    printf "    !!jlink path not as expected, install/permission problems?!!\n"
    printf "    expected: %s\n" ${EXPECTED_JLINK_GDBSERVER_PATH}
    printf "         got: %s\n" ${ACTUAL_JLINK_GDBSERVER_PATH}
    return

fi
#---------------------------------------------------------


#---------------------------------------------------------
printf "\n\nqorc-sdk build env initialized.\n\n\n"
#---------------------------------------------------------

# move back into the original dir, if it was different from QORC_SDK_PATH:
if [ ! "$CURRENT_DIR" == "$QORC_SDK_PATH" ] ; then
    cd - > /dev/null
fi

# references for choices made while writing the sh script:
# https://unix.stackexchange.com/questions/424492/how-to-define-a-shell-script-to-be-sourced-not-run
# https://stackoverflow.com/a/2237103/3379867
# https://unix.stackexchange.com/questions/65803/why-is-printf-better-than-echo/65819
# https://tldp.org/LDP/Bash-Beginners-Guide/html/sect_08_02.html
# functions are better than aliases: https://www.gnu.org/software/bash/manual/html_node/Aliases.html
