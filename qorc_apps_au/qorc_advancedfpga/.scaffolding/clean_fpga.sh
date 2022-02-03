#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# to adapt to a new fpga-only project: nothing needs to be changed

PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)
PROJECT_RTL_DIR="${PROJECT_ROOT_DIR}/fpga/rtl"

rm -rf "$PROJECT_RTL_DIR"/build 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*bit.h 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.bin 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.jlink 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.openocd 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/Makefile.symbiflow 2> /dev/null || true
