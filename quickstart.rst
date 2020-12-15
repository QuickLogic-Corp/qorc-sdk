QuickStart With QORC SDK
========================

This one-page guide should get you quickly setup and running with the QORC SDK.


.. contents::
   :depth: 2



Hardware
---------

The hardware required is a development kit and a USB-UART cable.


Development Kit
~~~~~~~~~~~~~~~

QuickFeather Development Board + Micro-USB cable


USB-UART cable (Optional)
~~~~~~~~~~~~~~~~~~~~~~~~~

Any 3.3V USB-UART cable, most common ones are FTDI FT232x or SiLabs CP210x based cables.



Software
--------


QORC SDK
~~~~~~~~

Clone the `qorc-sdk <https://github.com/QuickLogic-Corp/qorc-sdk>`_ repository recursively :

::

  git clone --recursive https://github.com/QuickLogic-Corp/qorc-sdk.git


ARM Cortex M4 Build Toolchain
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Download tarball according to the system configuration from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

   Current stable version tested with qorc-sdk is :code:`9-2020-q2-update`

2. Extract the tarball to a preferred path(/BASEPATH/TO/TOOLCHAIN/)

   ::

     sudo tar xvjf gcc-arm-none-eabi-your-version.tar.bz2 -C /BASEPATH/TO/TOOLCHAIN/

   The usual preferred path is for example /usr/share

   ::

     sudo tar xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C /usr/share/

3. Add the /BASEPATH/TO/TOOLCHAIN/gcc-arm-none-eabi-your-version/bin/ to PATH (only for current terminal session)

   ::

     export PATH=/BASEPATH/TO/TOOLCHAIN/gcc-arm-none-eabi-your-version/bin/:$PATH

   For the preferred path of /usr/share and current tested stable version 9-2020-q2-update for example:

   ::

     export PATH=/usr/share/gcc-arm-none-eabi-9-2020-q2-update/bin/:$PATH

4. Test the toolchain installation:

   ::

     which arm-none-eabi-gcc

   should show something like:

   ::

     /usr/share/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc


5. If the path settings need to be permanent, it can be added to the ~/.bashrc or ~/.bash_profile.

   Examples and illustrations are for example here: https://stackabuse.com/how-to-permanently-set-path-in-linux/


QuickLogic FPGA Build Toolchain
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The recommended way to install the toolchain is using the latest prebuilt release here: `quicklogic-fpga-toolchain releases <https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases>`_

For example, for the v1.3.1 toolchain:

1. Download the installation script. For :code:`v1.3.1` for example, download : `v1.3.1 installer script <https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run>`_

2. Set the directory path where you want the toolchain installed (/BASEPATH/TO/TOOLCHAIN/)

   ::

     export INSTALL_DIR=/BASEPATH/TO/TOOLCHAIN/
   
   The usual preferred path is inside :code:`/usr/share/quicklogic-fpga-toolchain`.
   
   For :code:`v1.3.1` for example:

   ::

     export INSTALL_DIR=/usr/share/quicklogic-fpga-toolchain/v1.3.1/

3. Run the installation script, from wherever it was downloaded, for example:

   ::

     bash Symbiflow_v1.3.1.gz.run

   This will setup the toolchain.

4. Initialize the toolchain environment :

   ::

     export PATH="$INSTALL_DIR/quicklogic-arch-defs/bin:$INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
     source "$INSTALL_DIR/conda/etc/profile.d/conda.sh"
     conda activate

5. Test the toolchain installation:

   ::

     ql_symbiflow -h

   should display output similar to the below:

   ::

     Below are the supported commands: 
     To synthesize and dump a eblif file:
         >ql_symbiflow -synth -src <source_dir path> -d <device> -P <package> -t <top> -v <verilog file/files> -p <pcf file>  
     To run synthesis, pack, place and route, generate bitstream:
         >ql_symbiflow -compile -src <source_dir path> -d <device> -P <package> -t <top> -v <verilog file/files> -p <pcf file>  
     To dump the jlink/post_verilog/header/binary file: 
         >ql_symbiflow -compile -src <source_dir path> -d <device> -P <package> -t <top> -v <verilog file/files> -p <pcf file> -dump <jlink/post_verilog/header/openocd/binary> 
     Device supported:ql-eos-s3
     Packages supported PD64,PU64,WR42 
     -h


QuickLogic TinyFPGA-Programmer-Application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Clone the `TinyFPGA-Programmer <https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application>`_ repository recursively:
   
   It is preferred to clone this inside the qorc-sdk directory.

   ::

     git clone https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git

2. Install the :code:`tinyfpgab` dependency:

   ::

     pip3 install tinyfpgab

3. Install the :code:`apio` drivers, and enable the serial driver:

   ::

     pip3 install apio
     apio drivers --serial-enable

4. Setup an alias to the programmer, from where you cloned the repo:

   ::

     alias qfprog="python3 /PATH/TO/BASE/DIR/TinyFPGA-Programmer-Application/tinyfpga-programmer-gui.py"

   This can be added to :code:`.bashrc` or `.bash_profile` to make it permanent as well. 

5. Test the QuickFeather USB port:

   Plug in the QuickFeather board and set it to :code:`flash mode`, and:

   - press :code:`RST` button, blue LED should start flashing
   - within 5 seconds, press the :code:`USR` button, green LED should now start flashing/breathing.
   - This indicates that the QuickFeather board is in :code:`flash mode`

   Check the output of :code:`lsusb` like below to see if the QuickFeather USB-CDC is detected correctly:

   ::

     lsusb | grep OpenMoko

   should display one of the IDs :code:`1d50:6140` or :code:`1d50:6130`, like below:

   :code:`Bus 002 Device 029: ID 1d50:6140 OpenMoko, Inc.` or :code:`Bus 002 Device 029: ID 1d50:6130 OpenMoko, Inc.`

6. Test the programmer application:

   ::

     qfprog --help

   should show an output similar to below:

   ::

     usage: tinyfpga-programmer-gui.py [-h] --mode [fpga-m4] [--m4app app.bin]
                                     [--appfpga appfpga.bin]
                                     [--bootloader boot.bin]
                                     [--bootfpga fpga.bin] [--reset]
                                     [--port /dev/ttySx] [--crc] [--checkrev]
                                     [--update] [--mfgpkg qf_mfgpkg/]

     optional arguments:
     -h, --help            show this help message and exit
     --mode [fpga-m4]      operation mode - m4/fpga/fpga-m4
     --m4app app.bin       m4 application program
     --appfpga appfpga.bin
                             application FPGA binary
     --bootloader boot.bin, --bl boot.bin
                             m4 bootloader program WARNING: do you really need to
                             do this? It is not common, and getting it wrong can
                             make you device non-functional
     --bootfpga fpga.bin   FPGA image to be used during programming WARNING: do
                             you really need to do this? It is not common, and
                             getting it wrong can make you device non-functional
     --reset               reset attached device
     --port /dev/ttySx     use this port
     --crc                 print CRCs
     --checkrev            check if CRC matches (flash is up-to-date)
     --update              program flash only if CRC mismatch (not up-to-date)
     --mfgpkg qf_mfgpkg/   directory containing all necessary binaries


Serial Terminal Application
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use your favorite serial terminal applications, common ones include:

- `GTKTerm <https://github.com/Jeija/gtkterm>`_

- `PuTTY <https://www.putty.org/>`_

- `screen <https://wiki.archlinux.org/index.php/Working_with_the_serial_console#Screen>`_

- `tio <https://github.com/tio/tio>`_

- `minicom <https://linux.die.net/man/1/minicom>`_

- `picocom <https://github.com/npat-efault/picocom>`_



A Traditional Hello World Application
---------------------------------------

Build
~~~~~

Flash
~~~~~

Run
~~~


A "Hardware" Hello World Application
-----------------------------------

Build
~~~~~

Flash
~~~~~

Run
~~~


That's it!
Now all requirements are installed, and you can build, flash and run any application using the QORC SDK.
