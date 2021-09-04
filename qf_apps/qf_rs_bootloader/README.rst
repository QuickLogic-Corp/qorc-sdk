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
   
