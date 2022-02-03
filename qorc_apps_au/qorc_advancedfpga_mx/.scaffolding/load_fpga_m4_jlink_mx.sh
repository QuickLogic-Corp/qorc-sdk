#!/bin/bash

# fpga-m4 load:
# load both fpga and m4 'images' -> m4 binary generated from gcc, fpga jlink script generated from matrixide tools

# note that the entire sequence is done using jlink gdb server itself, without gdb due to the scripting interface it exposes.
# alternatively, it can also be done with gdb script, talking to jlink gdb server.

# REQ:
# 1. JLink probe is connected to EOS_S3 and it is in DEBUG mode
# 2. JLinkExe is available in the path ('source debugenvsetup.sh') OR the path-to-JLinkExe is passed in as argument.
# 3. fpga jlink commandfile (.jlink) has been generated by adding a 'jlink' to the -dump command to ql_symbiflow 
#       - can also use the fpga-build task for this.

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# fallback to the one provided in PATH(if any) if not passed in as argument:
JLINK_EXE_PATH=$(which JLinkExe)

################################################################################
#   getopt based parsing
################################################################################
# option strings
SHORT="" # no short options at all, clearer that way.
# reference on how to enforce no short options: https://unix.stackexchange.com/questions/162624/how-to-use-getopt-in-bash-command-line-with-only-long-options
LONG="jlink-exe-path:,help"


# read the options
OPTS=$(getopt --options "$SHORT" --long "$LONG" --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "ERROR: failed to parse options...exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

usage()
{
    printf "\n"
    printf "[%s] usage:\n" $(basename $0)
    printf "\n"
    printf "load the fpga and m4 using JLink Commander\n"
    printf "\n"
    printf " syntax: $0 --jlink-exe-path=/path/to/JLinkExe\n"
    printf "\n"
    printf "example: $0 --jlink-exe-path=/usr/bin/JLinkExe\n"
    printf "\n"
}

# extract options and their arguments into variables
while true ; do
    case "$1" in
        --jlink-exe-path )
            JLINK_EXE_PATH="$2"
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
################################################################################


# sanity checks
if [ -z "$JLINK_EXE_PATH" ] ; then
    printf "\nERROR: JLINK_EXE_PATH is not defined!\n"
    printf "\nJLinkExe should be on path, or passed in as an argument!\n"
    usage
    exit 1
fi


# confirmation print
printf "\n"
printf "JLINK_EXE_PATH=$JLINK_EXE_PATH\n"
printf "\n"



PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)

PROJECT_OUTPUT_BIN_DIR="${PROJECT_ROOT_DIR}/GCC_Project/output/bin"
PROJECT_M4_BIN=$(ls "$PROJECT_OUTPUT_BIN_DIR"/*.bin)

if [ ! -f "$PROJECT_M4_BIN" ] ; then
    printf "\nERROR: m4 binary does not exist!\n"
    exit 1
fi

PROJECT_RTL_DIR="${PROJECT_ROOT_DIR}/fpga/rtl"
PROJECT_FPGA_DESIGN_JLINK=$(ls "$PROJECT_RTL_DIR"/config_bit_gen/*.jlink)

if [ ! -f "$PROJECT_FPGA_DESIGN_JLINK" ] ; then
    printf "\nERROR: fpga .jlink does not exist!\n"
    exit 1
fi


# generate a 'custom' jlink script to:
# [step 1] init the EOS_S3 (reset)
# [step 2] load the fpga design (.jlink generated)
# [step 3] load the m4 binary into RAM and run it

CUSTOM_JLINK_SCRIPT="custom_eoss3_m4_fpga.jlink"
CUSTOM_JLINK_SCRIPT_LOG="custom_eoss3_m4_fpga.jlink.log"

# write "NOTHING" into file, i.e. reset the contents, faster than delete + touch.
#https://askubuntu.com/a/549672
: > "$CUSTOM_JLINK_SCRIPT"

# [step 1] init the EOS_S3 (reset) and load the m4 binary
echo "connect" >> "$CUSTOM_JLINK_SCRIPT"
echo "RSetType 3" >> "$CUSTOM_JLINK_SCRIPT"
echo "r" >> "$CUSTOM_JLINK_SCRIPT"
echo "loadbin $PROJECT_M4_BIN, 0x0" >> "$CUSTOM_JLINK_SCRIPT"
echo "r" >> "$CUSTOM_JLINK_SCRIPT"
echo "" >> "$CUSTOM_JLINK_SCRIPT"


# [step 2] load the fpga design (.jlink generated)
# copy the contents of the generated .jlink as is
cat "$PROJECT_FPGA_DESIGN_JLINK" >> "$CUSTOM_JLINK_SCRIPT"
echo "" >> "$CUSTOM_JLINK_SCRIPT"
echo "" >> "$CUSTOM_JLINK_SCRIPT"


# [step 3] run the m4 code
echo "g" >> "$CUSTOM_JLINK_SCRIPT"
# note we can automatically quit jlink after this point, uncomment the below line this is needed.
#echo "q" >> "$CUSTOM_JLINK_SCRIPT"
echo "" >> "$CUSTOM_JLINK_SCRIPT"


# moar: https://wiki.segger.com/J-Link_Commander
# 'h' to halt
# 'i' to read JTAG ID (0x2BA01477)
# 'mem32 ADDR COUNT' to read COUNT words from ADDR 'mem32 0x40005484 1'
# 'w4 ADDR VAL' to write 4 byte word VAL into ADDR 'w4 0x2007C000, 0xAABBCCDD'
# 'savebin PATH_TO_SAVE_BIN, FROM_ADDR, NUM_BYTES' to dump RAM content into binary file 'savebin saved_mem.bin, 0x0, 0x20000'
# 'verifybin PATH_TO_ORIG_BIN, FROM_ADDR, NUM_BYTES' to compare and verify bin is loaded


# run custom jlink script : note that the JLinkExe will still be running after load, enter 'q' to quit
"$JLINK_EXE_PATH" -Device Cortex-M4 -If SWD -Speed 4000 -commandFile "$CUSTOM_JLINK_SCRIPT" -Log "$CUSTOM_JLINK_SCRIPT_LOG"

# remove the custom script/log (disable for debugging the script)
rm "$CUSTOM_JLINK_SCRIPT"
rm "$CUSTOM_JLINK_SCRIPT_LOG"
