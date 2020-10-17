qf_fpgauart_app
===============

Purpose
-------

The purpose of this app is to provide a simple example code to show usage of 
FPGA UART APIs. This application loads the FPGA UART gateware, starts the
FPGA clocks and runs a CLI (Command Line Interface) that allows the user to
- send a string of characters to the FPGA UART terminal
- receive specified length of bytes from the FPGA UART terminal
- run a speed test

Setup
-----

Build the project using the Makefile located in the folder qorc-sdk/qf_apps/qf_fpgauart_app. Load the binary to the Quickfeather board.
Connect one UART cable to the Quickfeather board HEADER_TX and HEADER_RX headers and another UART cable to the FPGA UART configured pins. The table below gives the default assignment. These can be changed by updating the quickfeather.pcf file in the FPGA UART gateware project.

UART | Quickfeather pad# | Quickfeather IO# | Quickfeather Header |
---- | ------------      | ---------------- | ------------------- |
Tx   |   2               | IO_2             |  J3.7               |
Rx   |   3               | IO_3             |  J2.10              |
GND  |   10              | IO_10            |  J8.13              |

