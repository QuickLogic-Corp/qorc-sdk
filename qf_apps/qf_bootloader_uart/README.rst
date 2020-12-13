QuickFeather Bootloader Application (UART)
==========================================

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

+-------+-------+-------+-------+-------+-------+-------+-------+
| Item  | S     | Start | Size  | End   | Start | Size  | End   |
|       | tatus |       |       |       |       |       |       |
+=======+=======+=======+=======+=======+=======+=======+=======+
| bootl | Used  | 0     | 0     | 0     | -     | 6     | 6     |
| oader |       | x0000 | x0001 | x0000 |       | 5,536 | 5,536 |
|       |       | _0000 | _0000 | _FFFF |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| boo   | Used  | 0     | 8     | 0     | 6     | 8     | 6     |
| tfpga |       | x0001 |       | x0001 | 5,536 |       | 5,544 |
| CRC   |       | _0000 |       | _0007 |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| ap    | F     | 0     | 8     | 0     | 6     | 8     | 6     |
| pfpga | uture | x0001 |       | x0001 | 9,632 |       | 9,640 |
| CRC   |       | _1000 |       | _1007 |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| a     | F     | 0     | 8     | 0     | 7     | 8     | 7     |
| ppffe | uture | x0001 |       | x0001 | 3,728 |       | 3,736 |
| CRC   |       | _2000 |       | _2007 |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| M4app | Used  | 0     | 8     | 0     | 7     | 8     | 7     |
| CRC   |       | x0001 |       | x0001 | 7,824 |       | 7,832 |
|       |       | _3000 |       | _3007 |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| bootl | Used  | 0     | 8     | 0     | 8     | 8     | 8     |
| oader |       | x0001 |       | x0001 | 1,920 |       | 1,928 |
| CRC   |       | _4000 |       | _4007 |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| boo   | Used  | 0     | 0     | 0     | 13    | 13    | 26    |
| tfpga |       | x0002 | x0002 | x0003 | 1,072 | 1,072 | 2,144 |
|       |       | _0000 | _0000 | _FFFF |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| ap    | F     | 0     | 0     | 0     | 26    | 13    | 39    |
| pfpga | uture | x0004 | x0002 | x0005 | 2,144 | 1,072 | 3,216 |
|       |       | _0000 | _0000 | _FFFF |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| a     | F     | 0     | 0     | 0     | 39    | 13    | 52    |
| ppffe | uture | x0006 | x0002 | x0007 | 3,216 | 1,072 | 4,288 |
|       |       | _0000 | _0000 | _FFFF |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+
| M4app | Used  | 0     | 0     | 0     | 52    | 45    | 97    |
|       |       | x0008 | x0006 | x000E | 4,288 | 0,560 | 4,848 |
|       |       | _0000 | _E000 | _DFFF |       |       |       |
+-------+-------+-------+-------+-------+-------+-------+-------+

Communication Interface
-----------------------

This bootloader project uses S3 UART interface available on the
Quickfeather board to receive and transmit commands from TinyFPGA
Programmer.
