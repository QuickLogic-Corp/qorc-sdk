Flashing-Loading Changes For Multi Image Usage
==============================================

Problem
-------

We need a way to support usage scenarios as below:

1. M4 only

   This is currently supported.
   This covers both M4 only usage as well as M4 + FPGA usage where the FPGA code is included inside the M4 binary itself via the header inclusion method.

2. FPGA only

   A way for the users to flash FPGA image, and which the Bootloader can load without need for a M4 image.

3. M4 + (independent)FPGA

   A way to flash the M4 image and FPGA image separately, and ability to load the FPGA image (by Bootloader) followed by loading the M4 image.

4. [FUTURE] FFE combinations

   A way to flash/load independent combinations of M4+FFE, M4+FFE+FPGA in the future.

Current plan is to add support for use cases 2 and 3, 4 being on the future roadmap.


Solution Design Part 1 - FPGA bin
---------------------------------

We need to generate a binary file for the FPGA, which includes the below components:

1. FPGA bitstream (already being generated as ${TOP}.bit)
2. FPGA meminit (as a separate bin) for FPGA RAM initialization
3. FPGA iomux info (as a separate bin) for proper pad configuration of pads used by the FPGA

Then, FPGA bin [${TOP}.bin] == bitstream bin + meminit bin + iomux bin

We will need a header in the FPGA bin which indicates the component info, for which we will adapt the FFE style of binary structure.

The diagrams below show a complete picture of the FPGA bin structure proposed.

FPGA bin header
~~~~~~~~~~~~~~~

.. image:: fpga-bin-structure-header.png

The header contains the fields:

- FPGA BIN VERSION - which will help handle future changes in bin structure if needed. Currently at v0.1.
- BITSTREAM BIN SIZE, CRC - size in bytes, and crc of the bitstream binary
- MEMINIT BIN SIZE, CRC - size in bytes, and crc of the meminit binary
- IOMUX BIN SIZE, CRC - size in bytes, and crc of the iomux binary

FPGA bitstream bin
~~~~~~~~~~~~~~~~~~

.. image:: fpga-bin-structure-bitstream.png

The bitstream bin will have 4B words, as is currently generated in ${TOP}.bit

FPGA meminit bin
~~~~~~~~~~~~~~~~

.. image:: fpga-bin-structure-meminit.png

The meminit bin will have the same structure as is currently generated in the header method.

For each RAM block, we will have:

- RAM block start address 4B
- RAM block size 4B
- size B of initialization values

This set will be repeated for as many RAM blocks in the design.

FPGA iomux bin
~~~~~~~~~~~~~~

.. image:: fpga-bin-structure-iomux.png

The iomux bin will have a set of pad configurations, each pad configuration is:

- 4B Reg Address
- 4B Reg Value

The number of pad configurations would be equal to the number of pads used by the FPGA design.


Solution Design Part 2 - TinyFPGAProgrammer/Bootloader
------------------------------------------------------

We need to add support to the TinyFPGAProgrammer and Bootloader, along with Flash Memory Map Changes.

The Flash Memory Map changes are covered separately in the `Flash Memory Map<flash_memory_map.rst>` document.

TinyFPGAProgrammer
~~~~~~~~~~~~~~~~~~

- Add support for the :code:`--appfpga` param which will flash the FPGA image into the APPFPGA partition.
  
  It will also add the required metadata (CRC, SIZE, IMAGE_INFO) in the APPFPGA Metadata section.

- Add support for a new :code:`--mode` param to denote the use case scenario.

  Examples for the use case scenarios:

  1. M4 only - :code:`--mode m4` (continues support for m4 with fpga included as header, as supported today)
  2. FPGA only - :code:`--mode fpga`
  3. M4 + (independent)FPGA - :code:`--mode m4-fpga` or :code:`--mode fpga-m4`
  4. M4 + FFE (future) - :code:`--mode m4-ffe` or :code:`--mode ffe-m4`
  5. M4 + FPGA + FFE (future) - any order of :code:`m4`, :code:`fpga` and :code:`ffe` in :code:`--mode m4-fpga-ffe`

  Add logic, which will, depending on the :code:`mode` argument value, will set the :code:`IMAGE ACTIVE FLAG` value of the corresponding flash partition.

  For example, with :code:`--mode fpga`, the FPGA partition will be marked ACTIVE, and others (M4/FFE/...) will be marked as INACTIVE.

  Add logic, which can support reading or setting only the :code:`mode` param from flash memory, without needing to actually flash images too.

  For example both of the below usages are ok:
  
  1. :code:`qfprog --port /dev/ttyACM0 --m4app output/bin/m4app.bin --mode m4` will set the mode as well as flash the image.
  2. :code:`qfprog --port /dev/ttyACM0 --mode m4` will set the mode only.
  3. :code:`qfprog --port /dev/ttyACM0 --mode` will read and display the mode set in flash.


Bootloader
~~~~~~~~~~

As per the Flash Memory Map changes, the booloader will use the :code:`IMAGE ACTIVE FLAG` and decide to load the corresponding images.

We would use the following order of loading in the general case:

1. If FPGA image is marked ACTIVE, do FPGA Load Process
2. If FFE image is marked ACTIVE, do FFE Load Process (future)
3. If M4 image is marked ACTIVE, do M4 Load Process

FPGA Load Process
+++++++++++++++++

1. Check the FPGA bin is ok (CRC)
2. Read the FPGA binary into RAM (including header, bitstream, meminit, iomux)
3. Check the FPGA bin header VERSION (future use, if needed), currently :code:`v0.1`
4. Read the fields of BITSTREAM SIZE/CRC, MEMINIT SIZE/CRC, IOMUX SIZE/CRC
5. Read the BITSTREAM bin using the SIZE, and execute FPGA Configuration.
6. Read the MEMINIT bin using the SIZE, and execute FPGA RAM initialization.
7. Read the IOMUX bin using the SIZE, and set the pad configurations accordingly.

FFE Load Process
++++++++++++++++

::

    NOTE: Future Usage.


M4 Load Process
+++++++++++++++

Keep same as current implementation, load the bin into SRAM 0x0 and release M4 core reset.


Implementation Phases
---------------------

Due to the fact that there will be breaking changes, and there are changes needed across the Symbiflow toolchain, qorc-sdk as well as the TinyFPGAProgrammer repos, we need to split the implementation going into mainline into phases.

In phase-1, we should be completing the Symbiflow toolchain changes fully, so we don't have to change this in Phase 2.

.. note:: We should ideally always have the TinyFPGAProgrammer repo as a submodule of the qorc sdk, this will help in keeping them in sync, especially for releases.
          This is because the programmer and the bootloader changes always go together.

Phase 1
~~~~~~~

1. **TinyFPGAProgrammer**

- no flash memory map changes
- add support for :code:`--appfpga` in the programmer
- ensure to set the metadata(size,crc) of M4APP to 0xFFFFFFFF in this case
- in case of :code:`--m4app` option, ensure to the set the metadata (size,crc) of APPFPGA to 0xFFFFFFFF
- PR : https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application/pull/7

2. **qorc-sdk bootloader/bootloader_uart**

- no flash memory map changes
- add logic, which will decide looking at the metadata to load *one of* M4APP -OR- APPFPGA only!
- FPGA load process will include the configuration of bitstream, mem init, and iomux init.
- PR : https://github.com/QuickLogic-Corp/qorc-sdk/pull/73


3. **Symbiflow scripts**

- add :code:`-dump binary` option to generate fpga bin (header + bitstream bin + meminit bin + iomux bin)
- add iomux generation with the :code:`-dump header` option (bitstream and meminit array is already being generated)
- update openocd generation with support for meminit (and fix issues with iomux)
- streamline common approach for all: header/binary/jlink/openocd

- PRs :

  - https://github.com/QuickLogic-Corp/quicklogic-fasm/pull/6
  - merge to master
  - https://github.com/QuickLogic-Corp/symbiflow-arch-defs/pull/181
  - merge to quicklogic-upstream-rebase
  - make a release with minor version change?

The order of changes should preferaby be:

1. Symbiflow toolchain update, do a minor release (1.3.1?)
2. TinyFPGAProgrammer update, require - specific Symbiflow toolchain version or above 
3. qorc-sdk update, require specific Symbiflow toolchain version and TinyFPGAProgrammer version

Easiest way to ensure this is to make both the Symbiflow and TinyFPGAProgrammer repos submodules of the qorc-sdk.
This would mean that each qorc-sdk version has a specific relation/requirement of specific version of the submodules.
This would also have it easier to bundle the codebase and the interdependencies.

With these, the current operation can continue as is, and there will not be breaking changes, at the same time we support the FPGA standalone loading.


Phase 2
~~~~~~~

Here we will introduce the major changes to support multiple scenarios of usage (and the --mode), including M4App + (independent)AppFPGA, M4App only, AppFPGA only, and others in the future.

Flash Memory Map changes will also be brought in.

This will bring a change in the TinyFPGAProgrammer and the qorc-sdk Bootloader repos only, which need to be in sync. (perhaps as a release?)

1. **TinyFPGAProgrammer**

- add --mode flag for operation (m4, m4+fpga, others)
- add logic to parse and update the mode into the new Flash Memory Map (active partition info)
- PR : https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application/pull/8

2. **qorc-sdk bootloader/bootloader_uart**

- add support for new Flash Memory Map, and use the "active partition" info to load the images in predefined order.
- the preferred order is AppFPGA, FFE(future), M4App (in any combination)
- PR : https://github.com/QuickLogic-Corp/qorc-sdk/pull/75
