#
# make all to make the 
#
#
# need to figure out a way to use these in the command line
RTL_SRCS:=$(wildcard *.v)
# RTL_SRCS:= $(wildcard ../*.v) $(wildcard *.v)
filters=
SRCS:=$(filter-out $(filters),$(notdir $(RTL_SRCS)))

all:		build/top.bit

build/top.bit:	${SRCS}
		ql_symbiflow -compile -d ql-eos-s3 -P pu64 -v ../../../IP_modules/UART_16550/rtl/*.v *.v -t top -p quickfeather.pcf


clean:
		rm -rf build Makefile.symbiflow
