#
# make all to make the 
#

IPMODULES = UART_16550
MODULEDIRS = $(addprefix ../../../ip_modules/,${IPMODULES})
RTL_SRCS:=$(wildcard *.v) $(wildcard ${MODULEDIRS}/rtl/*.v)
# RTL_SRCS:= $(wildcard ../*.v) $(wildcard *.v)
filters=
SRCS:=$(filter-out $(filters),${RTL_SRCS})

all:
			make --directory rtl all

clean:
			make --directory rtl clean
