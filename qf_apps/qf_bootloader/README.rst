QuickFeather Bootloader Application
===================================

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

This bootloader project uses USB-Serial interface available on the
Quickfeather board to receive and transmit commands from TinyFPGA
Programmer.


QOMU Boot Area Protection (Experimental)
----------------------------------------

Boot area (bootloader, bootfpga, bootloader metadata, bootfpga metadata) 
is now protected from inadvertent update by user.

Code Changes
~~~~~~~~~~~~

1. bootloader id string is added into the binary, at offset 0xFF80

   This is basically 64kB - 128B, so last 128 bytes in the bootloader binary 
   is the bootloader id string.

   The format of the id string is (ASCII-encoded)
   :code:`BOARD;APPLICATION;VER_MAJOR.VER_MINOR.VER_PATCH;DD-MMM-YYYY;<OPTIONAL STUFF>`

   An example would be :code:`QOMU;BOOTLOADER;1.0.1;17-MAR-2021;`

   The #define for the string is in main.c and the corresponding linker definition 
   for placement of this 128B string is in the quickfeather.ld file.

2. A new "custom command" subsection is added to the command handling from the UART 
   in the program_flash.c file.

   Right now, we add 2 custom opcodes - 0xB0 to "unlock bootloader area" and 0xB1 
   to "lock bootloader area"

   When these opcodes are received, the bootloader will set the "boot_area_locked" flag
   accordingly.

3. While executing flash erase and flash program commands, the "boot_area_locked" 
   is checked, and the flash/program is allowed only if the bootloader area has been 
   unlocked.

Features Explained
~~~~~~~~~~~~~~~~~~

- The bootloader id allows qfprog to check if this is a device that has its bootloader 
  area locked (like QOMU).

  This allows qfprog to print useful messages to the user to caution about the locked 
  area, and allows the user to override this with :code:`--force` flag

- The custom commands required to lock/unlock the bootloader area allow us to ensure 
  that a user does not inadvertently overwrite the bootloader area using an older 
  version of qfprog (which does not know about locked bootloader area)

  With an older version, user will not see any messages warning the user about the locked 
  bootloader area, however, the actual writes will be blocked by the bootloader itself 
  because older version will not send the unlock bootloader command, and this will be 
  printed out on the debug UART console by the bootloader.

Usage
~~~~~

0. update the qf_bootloader (this app) from the :code:`qomu-bl-dump-read-protect` branch.
   
   ::

     git pull
     git checkout qomu-bl-dump-read-protect

   From the qf_bootloader app, build and then flash the new bootloader bin onto the board.

   ::

     cd GCC_Project
     make

     qfprog --port /dev/ttyACM0 --bootloader GCC_Project/output/bin/qf_bootloader.bin


1. update the programmer to the latest, on the :code:`qomu-bl-dump-read-protect` branch

   ::

     cd TinyFPGA-Programmer-Application
     git pull
     git checkout qomu-bl-dump-read-protect

2. bootloader/bootfpga programming, **without** :code:`--force` flag will fail with message to user 
   like below.

   ::

     qfprog --port /dev/ttyACM0 --bootloader GCC_Project/output/bin/qf_bootloader.bin
     
     CLI mode
     ports =  ['/dev/ttyACM0 (QuickFeather)'] 1
     Using port  /dev/ttyACM0 (QuickFeather)
     read_bootloader_id
     FastREAD 0x0B ( 128 )
     [XXXXXXXX]                                        ]
     
        board : QOMU
          app : BOOTLOADER
      version : 1.0.0
         date : 17-MAR-2021
     
     Using port  /dev/ttyACM0 (QuickFeather)
     CAUTION: This device has a locked boot area.
     Use --force if you really want to update bootloader! 

3. **WITH** :code:`--force` flag to write into the bootloader area, qfprog will ask the user 
   to confirm if this is really intentional, like below.

   ::

     qfprog --port /dev/ttyACM0 --bootloader GCC_Project/output/bin/qf_bootloader.bin --force
     
     CLI mode
     ports =  ['/dev/ttyACM0 (QuickFeather)'] 1
     Using port  /dev/ttyACM0 (QuickFeather)
     read_bootloader_id
     FastREAD 0x0B ( 128 )
     [XXXXXXXX]                                        ]
     
        board : QOMU
          app : BOOTLOADER
      version : 1.0.0
         date : 17-MAR-2021
     
     WARNING!! using --force enables writing into bootloader area, are you sure? (yes/no)    yes
     send UNLOCK command to BL
     Using port  /dev/ttyACM0 (QuickFeather)
     unlock_boot_area
     Using port  /dev/ttyACM0 (QuickFeather)
     Programming bootloader with  GCC_Project/output/bin/qf_bootloader.bin
     Erasing designated flash pages
     Erase  64.0 KiB ( 0xd8 ) at  0x0
     Writing  binary
     Write  65536  bytes
     [XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
     Verifying  binary
     FastREAD 0x0B ( 65536 )
     [XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
     Success: read_back == data
     Writing metadata
     Erasing designated flash pages
     Erase  4.0 KiB ( 0x20 ) at  0x1f000
     Writing  metadata
     Write  8  bytes
     [X]                                               ]
     Verifying  metadata
     FastREAD 0x0B ( 8 )
     [X]                                               ]
     Success: read_back == data
     send LOCK command to BL
     lock_boot_area

4. Using with :code:`raw` mode the qfprog does not know if user will be writing into 
   locked area, hence there will be a caution message, but qfprog will continue to try 
   and write the binary into the address specified.

   However, without the :code:`--force` flag specified, this operation will be blocked 
   by the bootloader itself, which will not allow the actual writes.
   At the end, the read after write verification on the qfprog will fail, which is 
   to be expected in this case.

   example:

   ::

     qfprog --port /dev/ttyACM0 raw --write --file GCC_Project/output/bin/qf_bootloader.bin --addr 0x0
     
     CLI mode
     ports =  ['/dev/ttyACM0 (QuickFeather)'] 1
     Using port  /dev/ttyACM0 (QuickFeather)
     read_bootloader_id
     FastREAD 0x0B ( 128 )
     [XXXXXXXX]                                        ]
     
        board : QOMU
          app : BOOTLOADER
      version : 1.0.1
         date : 17-MAR-2021
     
     Using port  /dev/ttyACM0 (QuickFeather)
     
     CAUTION: This device has a locked boot area.
     Use --force if you really want to update bootloader area!
     
     file_path: /media/coolbreeze413/GALACTICA/work/clients/quicklogic/qorc/github/qorc-sdk/qf_apps/qf_bootloader/GCC_Project/output/bin/qf_bootloader.bin
     Erasing designated flash pages
     Erase  64.0 KiB ( 0xd8 ) at  0x0
     Writing  binary
     Write  65536  bytes
     [XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
     Verifying  binary
     FastREAD 0x0B ( 65536 )
     [XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
     FAILED: read_back != write data
     error!!!

5. Using with :code:`raw` mode **AND** :code:`--force` flag when it is known that while using the :code:`raw` 
   mode, the bootloader area will also be updated.

   Again, note that qfprog will ask user to confirm if this is intentional, as below.

   ::

     qfprog --port /dev/ttyACM0 --force raw --write --file GCC_Project/output/bin/qf_bootloader.bin --addr 0x0
     
     CLI mode
     ports =  ['/dev/ttyACM0 (QuickFeather)'] 1
     Using port  /dev/ttyACM0 (QuickFeather)
     read_bootloader_id
     FastREAD 0x0B ( 128 )
     [XXXXXXXX]                                        ]
     
        board : QOMU
          app : BOOTLOADER
      version : 1.0.1
         date : 17-MAR-2021
     
     WARNING!! using --force enables writing into bootloader area, are you sure? (yes/no)yes
     send UNLOCK command to BL
     Using port  /dev/ttyACM0 (QuickFeather)
     unlock_boot_area
     Using port  /dev/ttyACM0 (QuickFeather)
     
     CAUTION: This device has a locked boot area.
     Use --force if you really want to update bootloader area!
     
     file_path: /media/coolbreeze413/GALACTICA/work/clients/quicklogic/qorc/github/qorc-sdk/qf_apps/qf_bootloader/GCC_Project/output/bin/qf_bootloader.bin
     Erasing designated flash pages
     Erase  64.0 KiB ( 0xd8 ) at  0x0
     Writing  binary
     Write  65536  bytes
     [XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
     Verifying  binary
     FastREAD 0x0B ( 65536 )
     [XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX]
     Success: read_back == data
     send LOCK command to BL (raw mode)
     Using port  /dev/ttyACM0 (QuickFeather)
     lock_boot_area


Usage With Older qfprog
~~~~~~~~~~~~~~~~~~~~~~~

If older qfprog is used (not updated to latest on the qomu-bl-dump-read-protect branch) 
then, the qfprog has no idea about bootloader area lock process.

Hence, there won;t be any useful messages on the qfprog output - however, the bootloader 
will prevent any erase/program operations in the bootloader area, so it can never be 
updated using the older qfprog versions which do not know about the lock process.

At the end, the read after write verification on the qfprog will fail, which is 
to be expected in this case.

The debug output from bootloader, on the UART console will indicate this, like below:

::

  Block Erase 64 KBytesAddr: 0x00000000
  bootarea locked! use --force
  Read Status Reg 1 done
  Write Enable done
  Byte/Page Program Addr: 0x00000000
  bootarea locked! use --force
  Read Status Reg 1 done
  Write Enable done
  Byte/Page Program Addr: 0x00000100
  bootarea locked! use --force
  Read Status Reg 1 done
  Write Enable done
  Byte/Page Program Addr: 0x00000200
  bootarea locked! use --force
  Read Status Reg 1 done
  ...

