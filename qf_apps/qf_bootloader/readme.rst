
QuickFeather Bootloader
=======================

The QuickFeather bootloader has a flash memory map for 5 bin files, and
corresponding CRC for each of them. The 5 bin files are:

::

   bootloader
   bootfpga
   m4app
   appfpga (for future use)
   appffe (for future use)

The bootloader is loaded by a reset. It handles either communicating
with the TinyFPGA-Programmer to load new bin files into the flash, or it
loads m4 app binary and transfers control to it. The bootfpga area
contains the binary for the fpga image that the bootlaoder uses. The m4
app image is expected to contain and load any fpga image that it
requires.

The flash memory map defined for q-series devices is:

+----------------+--------+------------+-------------+------------+
| Item           | Status | Start      | End         | Size       |
+================+========+============+=============+============+
| bootloader     | Used   | 0x00000000 | 0x0000FFFF  | 0x00010000 |
|                |        |            |             |            |
|                |        | [0]        | [65535]     | [65536]    |
+----------------+--------+------------+-------------+------------+
| bootfpga CRC   | Used   | 0x00010000 | 0x00010007  | 0x00000008 |
|                |        |            |             |            |
|                |        | [65536]    | [65544]     | [8]        |
+----------------+--------+------------+-------------+------------+
| appfpga CRC    | Future | 0x00011000 | 0x0001_1007 | 0x00000008 |
|                |        |            |             |            |
|                |        | [69632]    | [69640]     | [8]        |
+----------------+--------+------------+-------------+------------+
| appffe CRC     | Future | 0x00012000 | 0x00012007  | 0x00000008 |
|                |        |            |             |            |
|                |        | []         | []          | [8]        |
+----------------+--------+------------+-------------+------------+
| M4app CRC      | Used   | 0x00013000 | 0x00013007  | 0x00000008 |
|                |        |            |             |            |
|                |        | []         | []          | [8]        |
+----------------+--------+------------+-------------+------------+
| bootloader CRC | Used   | 0x00014000 | 0x00014007  | 0x0000008  |
|                |        |            |             |            |
|                |        | []         | []          | [8]        |
+----------------+--------+------------+-------------+------------+
| bootfpga       | Used   | 0x00020000 | 0x0003FFFF  | 0x00020000 |
|                |        |            |             |            |
|                |        | []         | []          | []         |
+----------------+--------+------------+-------------+------------+
| appfpga        | Future | 0x00040000 | 0x0005FFFF  | 0x00020000 |
|                |        |            |             |            |
|                |        | []         | []          | []         |
+----------------+--------+------------+-------------+------------+
| appffe         | Future | 0x00060000 | 0x0007FFFF  | 0x00020000 |
|                |        |            |             |            |
|                |        | []         | []          |            |
|                |        |            |             | []         |
+----------------+--------+------------+-------------+------------+
| M4app          | Used   | 0x00080000 | 0x000EDFFF  | 0x0006E000 |
|                |        |            |             |            |
|                |        | []         | []          | []         |
+----------------+--------+------------+-------------+------------+

Communication Interface
-----------------------

This bootloader project uses USB-Serial interface available on the
Quickfeather board to receive and transmit commands from TinyFPGA
Programmer.
