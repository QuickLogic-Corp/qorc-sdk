QuickFeather UART to USB-serial adapter Application
================================================

This project is intended to mock an UART to USB adapter. Data received on the
quickfeather UART Rx is transmitted to the host PC using the USB-Serial IP. 
Similarly data received from the host PC over USB-Serial IP to the quickfeather
is transmitted to the quickfeather UART Tx signal.

Building and running the project:
----------------------------------------------------------

1. Use the provided `Makefile <GCC_Project/Makefile>`__ and an
   appropriate ARM GCC toolchain to build the project

2. Use the flash programming procedure to flash the binary to
   Quickfeather board.

3. Reset the board to start running the qf_ssi_ai_app application.

4. Open a terminal application program such as TeraTerm and conncect to the
   quickfeather UART using 460800bps, 8-bit data data, no-parity, 1 stop-bit
   and no flow control settings. Similarly open a terminal and connect to the
   quickfeather USB-Serial port.
   Data entered on the UART terminal would appear on the USB-Serial terminal.
   Similarly data entered on the USB-Serial terminal would appear on the UART
   terminal.

