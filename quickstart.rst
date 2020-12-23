QuickStart With QORC SDK
========================

This one-page guide should get you quickly setup and running with the QORC SDK.


.. contents::
   :depth: 4



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

Clone the `qorc-sdk <https://github.com/QuickLogic-Corp/qorc-sdk>`_ repository :

::

  git clone https://github.com/QuickLogic-Corp/qorc-sdk.git

**NOTE:** All the next steps below should be executed from the qorc-sdk directory.

::

  cd qorc-sdk

Setup Development Environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Automated Setup
^^^^^^^^^^^^^^^

The rest of the components can be automatically installed, and initialized using the helper script in the qorc-sdk :code:`envsetup.sh`

This is the recommended method for new users.

::

  source envsetup.sh

This will setup the following components within the :code:`qorc-sdk` directory (on first run) and on subsequent run, initialize the tools to ready the development setup.

- ARM Cortex M4 Build Toolchain

- QuickLogic FPGA Build Toolchain

- QuickLogic TinyFPGA-Programmer-Application

- Minimal set of submodules in qorc-sdk repo

Alternatively, each of the components can be setup manually using the **"Manual Setup"** below.



.. _qorc-sdk-components-manual-setup:

Manual Setup
^^^^^^^^^^^^

QORC SDK Submodules
###################

Init and Update the (minimal) submodules in the qorc-sdk : :code:`s3-gateware`, :code:`qorc-testapps`, :code:`qorc-example-apps`.

::
  
  git submodule update --init qorc-example-apps
  git submodule update --init qorc-testapps
  git submodule update --init s3-gateware


ARM Cortex M4 Build Toolchain
#############################

1. Download archive according to the system configuration from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

   Current stable version tested with qorc-sdk is :code:`9-2020-q2-update`

   For Linux systems, you can use:

   ::

     wget -O gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -q --show-progress --progress=bar:force 2>&1 "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118"

2. Extract the archive to :code:`arm_toolchain_install` dir inside qorc-sdk

   ::

     mkdir arm_toolchain_install
     tar xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C ${PWD}/arm_toolchain_install

3. Add the toolchain to PATH (only for current terminal session)

   ::

     export PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH


4. Test the toolchain installation:

   ::

     which arm-none-eabi-gcc

   should show something like:

   ::

     "current directory path"/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc


5. If the path settings need to be permanent, it can be added to the :code:`~/.bashrc` or :code:`~/.bash_profile`.

   Examples and illustrations are for example here: https://stackabuse.com/how-to-permanently-set-path-in-linux/


QuickLogic FPGA Build Toolchain
###############################

The recommended way to install the toolchain is using the latest prebuilt release here: `quicklogic-fpga-toolchain releases <https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases>`_

1. Download the installation script. For :code:`v1.3.1`, download : `v1.3.1 installer script <https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run>`_

   You can use for download:

   ::

     wget -O Symbiflow_v1.3.1.gz.run -q --show-progress --progress=bar:force 2>&1 https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain/releases/download/v1.3.1/Symbiflow_v1.3.1.gz.run

2. Set the directory path where the toolchain should be installed to :code:`fpga_toolchain_install/v1.3.1` dir inside the qorc-sdk

   ::

     export INSTALL_DIR=${PWD}/fpga_toolchain_install/v1.3.1
   
3. Run the installation script:

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
##########################################

1. Clone the `TinyFPGA-Programmer <https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application>`_ repository recursively:
   
   ::

     git clone --recursive https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git

2. Install the :code:`tinyfpgab` dependency:

   ::

     pip3 install tinyfpgab

3. Install the :code:`apio` drivers, and enable the serial driver:

   ::

     pip3 install apio
     apio drivers --serial-enable

4. Setup an alias to the programmer:

   ::

     alias qfprog="python3 ${PWD}/TinyFPGA-Programmer-Application/tinyfpga-programmer-gui.py"

   This can be added to :code:`.bashrc` or :code:`.bash_profile` to make it permanent as well. 

5. Test the programmer application:

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

Use your favorite serial terminal application, common ones include:

- `GTKTerm <https://github.com/Jeija/gtkterm>`_

- `PuTTY <https://www.putty.org/>`_

- `screen <https://wiki.archlinux.org/index.php/Working_with_the_serial_console#Screen>`_

- `tio <https://github.com/tio/tio>`_

- `minicom <https://linux.die.net/man/1/minicom>`_

- `picocom <https://github.com/npat-efault/picocom>`_


Linux Note For Serial Ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^
For accessing serial ports on Linux, the user must be added into the :code:`dialout` group.

To check if user is already part of :code:`dialout`:

::

  id -Gn $USER | grep -c "dialout"

will output :code:`1` if already in group or :code:`0` if not.

To add current user to :code:`dialout` group:

::

  sudo usermod -a -G dialout $USER

You may need to logout and back in for the new group to take effect.


Test the QuickFeather USB-CDC Port
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Plug in the QuickFeather board and set it to :code:`flash mode`, with the sequence:

- press :code:`RST` button, blue LED should start flashing
- within 5 seconds, press the :code:`USR` button, green LED should now start flashing/breathing.
- This indicates that the QuickFeather board is in :code:`flash mode`

Check the output of :code:`lsusb` like below to see if the QuickFeather USB-CDC is detected correctly:

::

  lsusb | grep OpenMoko

should display one of the IDs :code:`1d50:6140` or :code:`1d50:6130`, like below:

:code:`Bus 002 Device 029: ID 1d50:6140 OpenMoko, Inc.` or :code:`Bus 002 Device 029: ID 1d50:6130 OpenMoko, Inc.`


Bootloader Update
~~~~~~~~~~~~~~~~~

As this is a developing ecosystem, expect some churn at this time, with changes in the toolchains, flash programmer and bootloader.

To ensure that you are using the bleeding edge of the setup (recommended, unless you are specifically looking for a particular version), it is recommended to update the bootloader.

Bootloader has updates in the following qorc-sdk versions:

- v1.5.0 [https://github.com/QuickLogic-Corp/qorc-sdk/releases/tag/v1.5.0] [08 DEC 2020]

To update the Bootloader on the Development Kit, follow the Build, Flash, Run as below.


Build
^^^^^

Navigate to :code:`qf_bootloader` application build folder and run make  

::

  cd qf_apps/qf_bootloader/GCC_Project
  make 


Flash
^^^^^

Set the QuickFeather in :code:`flash mode`, with the sequence:

- press :code:`RST` button, blue LED should start flashing
- within 5 seconds, press the :code:`USR` button, green LED should now start flashing/breathing.
- This indicates that the QuickFeather board is in :code:`flash mode`

Flash the :code:`qf_bootloader` app into QuickFeather with:

::

  qfprog --port <PORTNAME> --bootloader output/bin/qf_bootloader.bin --mode m4

where the :code:`<PORTNAME>`: is the UART port exposed by the QuickFeather.

This would be of the form:

- :code:`COM##` on PC/Windows
- :code:`/dev/ttyS##` on PC/wsl1/wsl2/Ubuntu18+ (where the ## is the same as the COM## shown by device manager under Windows)
- :code:`/dev/ttyACM#` on PC/Ubuntu18+

Run
^^^

Once flashing is completed, press the :code:`RST` button to reset the QuickFeather, the new bootloader should be executed.


A Traditional Hello World Application
--------------------------------------

The :code:`qf_apps/qf_helloworldsw` application in the qorc-sdk is the (almost) traditional hello world application.

| The M4 code prints out a banner and a "Hello World" message onto the :code:`USB2SERIAL` peripheral.
| The :code:`USB2SERIAL` peripheral is a USB-CDC (UART) peripheral programmed into the FPGA of the EOSS3 by the M4 code.
| When the QuickFeather board is connected with the Micro-USB cable, this appears as a USB-CDC Serial Port on the PC.

Build
~~~~~

Navigate to :code:`qf_helloworldsw` build folder and run make  

::

  cd qf_apps/qf_helloworldsw/GCC_Project
  make 


Flash
~~~~~

Set the QuickFeather in :code:`flash mode`, with the sequence:

- press :code:`RST` button, blue LED should start flashing
- within 5 seconds, press the :code:`USR` button, green LED should now start flashing/breathing.
- This indicates that the QuickFeather board is in :code:`flash mode`

Flash the :code:`qf_helloworldsw` app into QuickFeather with:

::

  qfprog --port <PORTNAME> --m4app output/bin/qf_helloworldsw.bin --mode m4

where the :code:`<PORTNAME>`: is the UART port exposed by the QuickFeather.

This would be of the form:

- :code:`COM##` on PC/Windows
- :code:`/dev/ttyS##` on PC/wsl1/wsl2/Ubuntu18+ (where the ## is the same as the COM## shown by device manager under Windows)
- :code:`/dev/ttyACM#` on PC/Ubuntu18+


Run
~~~

Once flashing is completed, press the :code:`RST` button to reset the QuickFeather.

Do not press :code:`USR` button this time, and then the bootloader would load the newly flashed application above.

Use your favorite Serial Port Application to connect to the QuickFeather USB-CDC port.

You should see an output like below:

::
  
  ##########################
  Quicklogic QuickFeather LED / User Button Test
  SW Version: qorc-sdk/qf_apps/qf_helloworldsw
  Sep 20 2020 14:24:43
  ##########################

  #*******************
  Command Line Interface
  App SW Version: qorc-sdk/qf_apps/qf_helloworldsw
  #*******************
  [0] >

| The :code:`diag` submenu option is available, and can be used to toggle the RGB leds or detect the USR button press:
|
| Toggle LEDs:
| At the :code:`[0] >` prompt, which is the level 0 prompt, use:
    
- :code:`diag red` to toggle the red led
- :code:`diag green` to toggle the green led
- :code:`diag blue` to toggle the blue led
    
| Detect USR button press:
| At the :code:`[0] >` prompt, which is the level 0 prompt, do: 

- | Keep the USR button pressed (connected to IO_6 on QuickFeather)
  | Execute: :code:`diag userbutton` to check state
  | You should see  :code:`Pressed`
  |

- | Without the USR button pressed
  | Execute: :code:`diag userbutton` to check state
  | You should see :code:`Not pressed`
  |


A "Hardware" Hello World Application
-------------------------------------

The :code:`qf_apps/qf_helloworldhw` application in the qorc-sdk is a traditional fpga hello world application - a "blinky"

The M4 code loads a basic FPGA design which blinks the green LED on the quickfeather.

| The FPGA design is in verilog, and can be found in the :code:`qf_apps/qf_helloworldhw/fpga/rtl` directory
| The FPGA build is part of the makefiles, and as part of the :code:`make`, the FPGA design is compiled into a bitstream, in a header format.
| The M4 code uses this header, and loads the FPGA design at runtime.

| Note that, because the M4 code loads the blinky FPGA design in this case, and not the USB2SERIAL FPGA Design, the USB-CDC of the QuickFeather Micro-USB will not be available.
| Hence, this example will print out the banner via the EOSS3 HW UART port.
| To check the output, you would need to connect a USB-UART cable to the QuickFeather UART pins.
| Refer to `QuickFeather UART Usage <https://github.com/QuickLogic-Corp/quick-feather-dev-board#advanced>`_ for the connection diagram

Build
~~~~~

Navigate to :code:`qf_helloworldhw` build folder and run make  

::

  cd qf_apps/qf_helloworldhw/GCC_Project
  make 


Flash
~~~~~

Set the QuickFeather in :code:`flash mode`, with the sequence:

- press :code:`RST` button, blue LED should start flashing
- within 5 seconds, press the :code:`USR` button, green LED should now start flashing/breathing.
- This indicates that the QuickFeather board is in :code:`flash mode`

Flash the :code:`qf_helloworldhw` app into QuickFeather with:

::

  qfprog --port <PORTNAME> --m4app output/bin/qf_helloworldhw.bin --mode m4

where the :code:`<PORTNAME>`: is the UART port exposed by the QuickFeather.

This would be of the form:

- :code:`COM##` on PC/Windows
- :code:`/dev/ttyS##` on PC/wsl1/wsl2/Ubuntu18+ (where the ## is the same as the COM## shown by device manager under Windows)
- :code:`/dev/ttyACM#` on PC/Ubuntu18+


Run
~~~

Once flashing is completed, press the :code:`RST` button to reset the QuickFeather.

Do not press :code:`USR` button this time, and then the bootloader would load the newly flashed application above.

Once the application is loaded, you can see the blinky in action - the green LED of the QuickFeather starts blinking.

[Optional]

If you want to look at the debug output, connect a USB-UART cable between the QuickFeather UART and the PC as described above.

Use your favorite Serial Port Application to connect to the port exposed by the USB-UART cable.

You should see an output like below, and nothing else will follow:

::
  
  ##########################
  Quicklogic QuickFeather Standalone FPGA
  SW Version: qorc-sdk/qf_apps/qf_helloworldhw
  Sep 20 2020 14:24:43
  ##########################


Running Examples In A New Terminal
----------------------------------

Anytime a new terminal is opened, the already installed tools need to be initialized before any of the apps can be used.

Automated Setup
~~~~~~~~~~~~~~~
If you had used the automated setup using the helper script, then the same script can be run again to initialize all components.

::

  cd "path to qorc-sdk"
  source envsetup.sh


Manual Setup
~~~~~~~~~~~~~~~

If you have setup the components manually, then corresponding to the install instructions above, the initialization seqeunce can be followed.

The sequence that can be followed is:

::

  cd "path to qorc-sdk"
  export PATH=${PWD}/arm_toolchain_install/gcc-arm-none-eabi-9-2020-q2-update/bin:$PATH
  export INSTALL_DIR=${PWD}/fpga_toolchain_install/v1.3.1
  export PATH="$INSTALL_DIR/quicklogic-arch-defs/bin:$INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
  source "$INSTALL_DIR/conda/etc/profile.d/conda.sh"
  conda activate
  alias qfprog="python3 ${PWD}/TinyFPGA-Programmer-Application/tinyfpga-programmer-gui.py"

Once the sequence is executed, the environment is ready to be used for build/flash/run of any qorc-sdk application.

You can open a new terminal, execute the initialization sequence as above, and try the "Hello World" applications


Epilogue
--------
That's it!

If you have got this far, then the setup is verified, and you can now build and flash any application in the qorc-sdk to the QuickFeather.

The next step would be checkout the :code:`goingfurther.rst` for exploring a few more advanced applications, which can give a feel of the various ways to use the qorc-sdk to build applications **(coming soon)**.

You can look at the applications in :code:`qf_apps`, :code:`qorc-testapps`, :code:`qorc-example-apps` and :code:`qf_vr_apps` for more examples of what can be done with the qorc-sdk.
