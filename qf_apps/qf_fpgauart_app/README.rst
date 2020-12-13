QuickFeather FPGA UART Application
==================================

Purpose
-------

The purpose of this app is to provide a simple example code to show
usage of FPGA UART APIs. This application loads the FPGA UART gateware,
starts the FPGA clocks and runs a CLI (Command Line Interface) that
allows the user to

-  send a string of characters to the FPGA UART terminal
-  receive specified length of bytes from the FPGA UART terminal
-  run a speed test

Setup
-----

Build the project using the Makefile located in the folder
qorc-sdk/qf_apps/qf_fpgauart_app. Load the binary to the Quickfeather
board. Connect one UART cable to the Quickfeather board HEADER_TX and
HEADER_RX headers and another UART cable to the FPGA UART0 configured
pins. The table below gives the default assignment. These can be changed
by updating the quickfeather.pcf file in the FPGA UART gateware project.
The CLI (command line interface) attached to the first UART terminal may
be used to send and receive data from the FPGA UART0 terminal. Connect a
third UART cable to FPGA UART1 configured pins. The CLI attached to the
first UART terminal may be used to send and receive data from the FPGA
UART1 terminal.

======== ================= ================ ===================
Name     Quickfeather pad# Quickfeather IO# Quickfeather Header
======== ================= ================ ===================
UART0 Tx 2                 IO_2             J3.7
UART0 Rx 3                 IO_3             J2.10
UART1 Tx 4                 IO_4             J8.7
UART1 Rx 5                 IO_5             J8.9
GND      10                IO_10            J8.8
======== ================= ================ ===================

CLI UART0/UART1 Commands
------------------------

The following commands are available for sending and receiving data from
the FPGA UART terminals.

::

   [0] > help
   help-path: (top)
   readid         - Get FPGA device-id
   diag           - QuickFeather diagnostic commands
   uart0          - QuickFeather FPGA-UART0 commands
   uart1          - QuickFeather FPGA-UART1 commands
   exit           - exit/leave menu
   help           - show help
   ?              - show help
   help-end:

To send data to FPGA UART0, issue the following command:

::

   [0] > uart0 send HelloWorld!

The string HelloWorld! would appear on the terminal application
connected FPGA UART0 configured pins.

Similarly to receive data from FPGA UART0, issue the following commnad:

::

   [0] > uart0 recv 5

The CLI will wait for data to be input on the FPGA UART0 terminal. Type
in 5 characters on the FPGA UART0 terminal. These characters will then
be echoed on the CLI terminal.
