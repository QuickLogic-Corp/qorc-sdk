#
# make all to make the 
#

IPMODULES = UART_16550
MODULEDIRS = $(addprefix ../../../ip_modules/,${IPMODULES})
RTL_SRCS:=$(wildcard *.v) $(wildcard ${MODULEDIRS}/rtl/*.v)
# RTL_SRCS:= $(wildcard ../*.v) $(wildcard *.v)
filters=
SRCS:=$(filter-out $(filters),${RTL_SRCS})

all:			../top_bit.h

top_bit.h:		${SRCS}
				ql_symbiflow -compile -d ql-eos-s3 -P pu64 -v ${RTL_SRCS} -t top -p quickfeather.pcf -dump header
		
../top_bit.h:	top_bit.h
				mv top_bit.h ../top_bit.h


clean:
		rm -rf build Makefile.symbiflow
