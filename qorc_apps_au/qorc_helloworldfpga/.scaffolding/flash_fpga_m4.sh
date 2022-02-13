#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# check if QORC_SDK_PATH is already setup (source envsetup.sh has already been invoked in the current shell)
# paranoid check - Makefile does this already.
if [ -z "$QORC_SDK_PATH" ] ; then
    printf "\nERROR: QORC_SDK_PATH is not set in the environment, is the QORC SDK initialized?\n\n"
    printf "    initialize with: 'source envsetup.sh' from the QORC SDK directory in the current shell\n"
    exit 1
fi

################################################################################
#   getopt based parsing
################################################################################
# option strings
SHORT="" # no short options at all, clearer that way.
# reference on how to enforce no short options: https://unix.stackexchange.com/questions/162624/how-to-use-getopt-in-bash-command-line-with-only-long-options
LONG="port:,help"


# read the options
OPTS=$(getopt --options "$SHORT" --long "$LONG" --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "ERROR: failed to parse options...exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

usage()
{
    printf "\n"
    printf "[%s] usage:\n" $(basename $0)
    printf "\n"
    printf "flash m4/fpga/both onto the board\n"
    printf "\n"
    printf " syntax: $0 --port=serial-port\n"
    printf "\n"
    printf "example: $0 --port=/dev/ttyUSB0\n"
    printf "\n"
}

# extract options and their arguments into variables
while true ; do
    case "$1" in
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

if [ -z "$PORT" ] ; then
    printf "\nERROR: PORT is not defined!\n"
    usage
    exit 1
fi


# confirmation print
printf "\n"
printf "PORT=$PORT\n"
printf "\n"
################################################################################

PROJECT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)
PROJECT_M4_DIR="${PROJECT_DIR}/GCC_Project"
PROJECT_FPGA_DIR="${PROJECT_DIR}/fpga"


# check if we have m4 code in the current project
PROJECT_M4_BIN=
if [ -d "$PROJECT_M4_DIR" ] ; then
    PROJECT_OUTPUT_BIN_DIR="${PROJECT_DIR}/GCC_Project/output/bin"
    PROJECT_M4_BIN=$(ls "$PROJECT_OUTPUT_BIN_DIR"/*.bin)
    if [ ! -f "$PROJECT_M4_BIN" ] ; then
        printf "\nERROR: m4 binary does not exist! (is build done?)\n"
        exit 1
    fi
fi


# check if we have fpga design in the current project
PROJECT_FPGA_DESIGN_BIN=
if [ -d "$PROJECT_FPGA_DIR" ] ; then
    PROJECT_RTL_DIR="${PROJECT_FPGA_DIR}/rtl"
    PROJECT_FPGA_DESIGN_BIN=$(ls "$PROJECT_RTL_DIR"/*.bin)
    if [ ! -f "$PROJECT_FPGA_DESIGN_BIN" ] ; then
        printf "\nERROR: fpga .bin does not exist! (is build done?)\n"
        exit 1
    fi
fi


# sanity: atleast one of m4 code, or fpga design should exist.
if [ ! -d "$PROJECT_M4_DIR" ] && [ ! -d "$PROJECT_FPGA_DIR" ] ; then
    printf "\nERROR: neither 'GCC_Project', nor 'fpga' directories exist!\n"
    printf "check the code strucure!\n\n"
    exit 1
fi


if [ -f "$PROJECT_FPGA_DESIGN_BIN" ] && [ -f "$PROJECT_M4_BIN" ] ; then
    # both fpga bin and m4 bin are produced in this project, so mode=fpga-m4
    printf "\nrunning command:\n"
    printf "qfprog --port $PORT --m4app $PROJECT_M4_BIN --appfpga $PROJECT_FPGA_DESIGN_BIN --mode fpga-m4 --reset\n\n"
    qfprog --port "$PORT" --m4app "$PROJECT_M4_BIN" --appfpga "$PROJECT_FPGA_DESIGN_BIN" --mode fpga-m4 --reset
elif [ -f "$PROJECT_FPGA_DESIGN_BIN" ] ; then
    # only fpga bin is produced, so mode=fpga
    printf "\nrunning command:\n"
    printf "qfprog --port $PORT --appfpga $PROJECT_FPGA_DESIGN_BIN --mode fpga --reset\n\n"
    qfprog --port "$PORT" --appfpga "$PROJECT_FPGA_DESIGN_BIN" --mode fpga --reset
elif [ -f "$PROJECT_M4_BIN" ] ; then
    # only m4 bin is produced, so mode=m4
    printf "\nrunning command:\n"
    printf "qfprog --port $PORT --m4app $PROJECT_M4_BIN--mode m4 --reset\n\n"
    qfprog --port "$PORT" --m4app "$PROJECT_M4_BIN" --mode m4 --reset
else
    printf "\nshould not reach here!\n"
    exit 1
fi
