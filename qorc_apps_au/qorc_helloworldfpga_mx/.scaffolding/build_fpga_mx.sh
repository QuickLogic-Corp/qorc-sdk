#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )


################################################################################
#   getopt based parsing
################################################################################
# option strings
SHORT="" # no short options at all, clearer that way.
# reference on how to enforce no short options: https://unix.stackexchange.com/questions/162624/how-to-use-getopt-in-bash-command-line-with-only-long-options
LONG="matrixide-install-path:,matrixide-license-path:,help"


# read the options
OPTS=$(getopt --options "$SHORT" --long "$LONG" --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "ERROR: failed to parse options...exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

usage()
{
    printf "\n"
    printf "[%s] usage:\n" $(basename $0)
    printf "\n"
    printf "build the fpga design using matrixide\n"
    printf "\n"
    printf " syntax: $0 --matrixide-install-path=/path/to/matrixide/dir --matrixide-license-path=/path/to/matrixide/lic\n"
    printf "\n"
    printf "example: $0 --matrixide-install-path=$HOME/matrixide_64 --matrixide-license-path=$HOME/matrixide_64/matrixide.lic\n"
    printf "\n"
}

# extract options and their arguments into variables
while true ; do
    case "$1" in
        --matrixide-install-path )
            MATRIXIDE_INSTALL_PATH="$2"
            shift 2
        ;;
        --matrixide-license-path )
            MATRIXIDE_LICENSE_PATH="$2"
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
if [ -z "$MATRIXIDE_INSTALL_PATH" ] ; then
    printf "\nWARNING: MATRIXIDE_INSTALL_PATH is not defined!\n\n"
fi

if [ -z "$MATRIXIDE_LICENSE_PATH" ] ; then
    printf "\nWARNING: MATRIXIDE_LICENSE_PATH is not defined!\n\n"
fi


# confirmation print
printf "\n"
printf "MATRIXIDE_INSTALL_PATH=$MATRIXIDE_INSTALL_PATH\n"
printf "MATRIXIDE_LICENSE_PATH=$MATRIXIDE_LICENSE_PATH\n"
printf "\n"
################################################################################


# setup MATRIXIDE environment
if [ ! -z "$MATRIXIDE_INSTALL_PATH" ] ; then
    cd "$MATRIXIDE_INSTALL_PATH"
    source setup.sh
    export rapidsilic_LICENSE="$MATRIXIDE_LICENSE_PATH"
    cd - > /dev/null
fi


PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)
PROJECT_RTL_DIR="${PROJECT_ROOT_DIR}/fpga/rtl"

#PROJECT_VERILOG_FILES="helloworldfpga.v"
PROJECT_VERILOG_FILES=$(cd ${PROJECT_RTL_DIR};ls *.v)
echo "$PROJECT_VERILOG_FILES" > ${PROJECT_RTL_DIR}/tmp_v_list
sed '/^$/d' $PROJECT_RTL_DIR/tmp_v_list > $PROJECT_RTL_DIR/tmp_f_list
PROJECT_VERILOG_FILES=$(cat $PROJECT_RTL_DIR/tmp_f_list)
rm $PROJECT_RTL_DIR/tmp_v_list $PROJECT_RTL_DIR/tmp_f_list

# name of the "top" module in the fpga design
PROJECT_TOP_MODULE="helloworldfpga"

#PROJECT_QDC_FILE="helloworldfpga.qdc"
PROJECT_QDC_FILE=$(cd ${PROJECT_RTL_DIR};ls *.qdc)

# PD64/PU64/WD48
PROJECT_PACKAGE="PU64"

PROJECT_DEVICE="RSMX3TFF512"

# switch to the RTL sources dir before running MATRIXIDE
cd $PROJECT_RTL_DIR

printf "running matrixide command:\n\n"
printf "matrixide --console --compile_design $PROJECT_VERILOG_FILES --top $PROJECT_TOP_MODULE -o $PROJECT_QDC_FILE -d $PROJECT_DEVICE -k $PROJECT_PACKAGE --run_all\n\n"

matrixide --console --compile_design $PROJECT_VERILOG_FILES --top $PROJECT_TOP_MODULE -o $PROJECT_QDC_FILE -d $PROJECT_DEVICE -k $PROJECT_PACKAGE --run_all

# switch back to original dir
cd - > /dev/null
