QuickFeather Bootloader Application (UART)
==========================================

The QuickFeather bootloader has a flash memory map for 5 bin files, and
corresponding CRC for each of them. The 5 bin files are:

::

   bootloader
   bootfpga
   m4app
   appfpga
   appffe (for future use)

The bootloader is loaded by a reset. It handles either communicating
with the TinyFPGA-Programmer to load new bin files into the flash, or it
loads m4 app binary and transfers control to it. The bootfpga area
contains the binary for the fpga image that the bootlaoder uses. The m4
app image is expected to contain and load any fpga image that it
requires.

The flash memory map defined for q-series devices is as below:

.. image:: qorc-flash-memory-map-addresses.svg

Communication Interface
-----------------------

This bootloader project uses EOS S3 UART on the Quickfeather board to receive and transmit commands from TinyFPGA Programmer.

Copy the file cli.py to the TinyFPGA-Programmer-Application folder. Then
invoke the following on the command line to write FPGA bit-stream to the
Quickfeather Flash and run the FPGA application

::

   $ python cli.py
   

Flashing and running an FPGA bitstream
-----------------------

First-time setup
----------

Build qf_apps/qf_rs_bootloader application project. Flash the quickfeather
board using this bootloader qf_rs_bootloader.bin binary using the 
TinyFPGA-Programmer-Application. Refer qorc-sdk documentation for instructions
to flash the quickfeather board.

::

   $ python tinyfpga-programmer-gui.py --bootloader qf_rs_bootloader.bin
   

Now, power cycle the quickfeather board and put the quickfeather in Flash 
mode by pressing the userbutton on the board within 5 secs of power cycling 
the board. Run the cli.py application and change the boot-mode so the 
bootloader stays in the bootloader mode.

::

   $ python cli.py
   [S3] comport <COMxx>                  // set the comport to <COMxx>
   [S3] change-boot-mode 1
   
Now the board is ready to Flash an FPGA bitstream and run this FPGA bitstream.
Issue the following commands in the running python cli.py command line interface.

   [S3] comport   <COMxx>                      // set the comport to <COMxx>
   [S3] flashfpga <fpga-bitstream-file>        // flash the bitstream <fpga-bitstream-file>
   [S3] runfpga

