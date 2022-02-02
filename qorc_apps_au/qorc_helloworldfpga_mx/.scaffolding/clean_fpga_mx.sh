#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# to adapt to a new fpga-only project: nothing needs to be changed

PROJECT_ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)
PROJECT_RTL_DIR="${PROJECT_ROOT_DIR}/fpga/rtl"

rm -rf "$PROJECT_RTL_DIR"/config_bit_gen 2> /dev/null || true
rm -rf "$PROJECT_RTL_DIR"/*_linux_cache 2> /dev/null || true
rm -rf "$PROJECT_RTL_DIR"/logic_optimizer 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*_yosys.log 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.blif 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.edf 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.html 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.log 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.rpt 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.tcl 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.ys 2> /dev/null || true
rm "$PROJECT_RTL_DIR"/*.timing.db 2> /dev/null || true
