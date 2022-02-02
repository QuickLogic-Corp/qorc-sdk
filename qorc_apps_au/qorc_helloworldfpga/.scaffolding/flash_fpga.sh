#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

################################################################################
#   getopt based parsing
################################################################################
# option strings
SHORT="" # no short options at all, clearer that way.
# reference on how to enforce no short options: https://unix.stackexchange.com/questions/162624/how-to-use-getopt-in-bash-command-line-with-only-long-options
LONG="qorc-sdk-path:,port:,help"


# read the options
OPTS=$(getopt --options "$SHORT" --long "$LONG" --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "ERROR: failed to parse options...exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

usage()
{
    printf "\n"
    printf "[%s] usage:\n" $(basename $0)
    printf "\n"
    printf "build the fpga design\n"
    printf "\n"
    printf " syntax: $0 --qorc-sdk-path=/path/to/qorc/sdk --port=serial_port_id\n"
    printf "\n"
    printf "example: $0 --qorc-sdk-path=$HOME/qorc-sdk --port=/dev/ttyUSB0\n"
    printf "\n"
}

# extract options and their arguments into variables
while true ; do
    case "$1" in
        --qorc-sdk-path )
            QORC_SDK_PATH="$2"
            shift 2
        ;;
        --port )
            PORT="$2"
            shift 2
        ;;
        -- )
            shift
            break
        ;;
        -h | --help | *)
            usage
            exit 0
        ;;
    esac
done

# arg checks
if [ -z "$QORC_SDK_PATH" ] ; then
    printf "\nWARNING: QORC_SDK_PATH is not defined!\n"
fi

if [ -z "$PORT" ] ; then
    printf "\nERROR: PORT is not defined!\n"
    usage
    exit 1
fi


# confirmation print
printf "\n"
printf "QORC_SDK_PATH=$QORC_SDK_PATH\n"
printf "\n"
################################################################################

# setup QORC_SDK environment
if [ ! -z "$QORC_SDK_PATH" ] ; then
    cd $QORC_SDK_PATH
    source envsetup.sh
    cd - > /dev/null
fi


PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)

PROJECT_RTL_DIR="${PROJECT_ROOT_DIR}/fpga/rtl"
PROJECT_FPGA_DESIGN_BIN=$(ls "$PROJECT_RTL_DIR"/*.bin)

printf "flash using port [%s], design [%s] with mode [%s]\n\n" "$PORT" "$PROJECT_FPGA_DESIGN_BIN" "fpga"

# qfprog is a function created in envsetup.sh
printf "running flash command:\n\n"
printf "qfprog --port $PORT --appfpga $PROJECT_FPGA_DESIGN_BIN --mode fpga --reset\n\n"
qfprog --port "$PORT" --appfpga "$PROJECT_FPGA_DESIGN_BIN" --mode fpga --reset
