QuickFeather Gateware Test Harness Application
==============================================

Purpose
-------

The purpose of this app is to provide a software framework that allows
gateware developrs to perform basic function tests on their gateware
before releaseing the gateware to teh software developers. The harness
loads the gatware, starts the FPGA clocks, and runs a CLI (Command Line
Interface) that allows the developer to:

-  read the ID register (register 0x00)
-  read any arbitrary register
-  write any arbitrary register

The thought process is that reading the ID register provides a basic
proof of life, and from there the developer can read and write registers
to monitor and exercise the gateware (slowly, of course).

Setup
-----

Setup gateware
~~~~~~~~~~~~~~

The app is located in qorc-sdk/qf_apps/qf_gwtestharness. This example
application uses the UART+GPIO design located in
projects/S3_FPGA_UART_GPIO folder of the s3-gateware submodule. The new
gateware should be replace the example gateware located on the gateware
directory. Gateware is split between the projects directory, which
contains the top level designs, and the modules directory, which
contains common modules. The example used by this application uses the
modules UART_16550 and GPIO located in ip_modules folder of s3-gateware
submodule. Note that all designs will have a project, but not all
designs will use modules.

| You will want to add a new directory under projects for you project:
  projects/yourproject.
| And if you project uses modules, add the appropriate module
  directories under the modules directory.

You will need a makefile in the projects directory that creates
top_bit.h in the projects directory. Recommend that you simply copy the
makefile from projects/S3_FPGA_UART_GPIO into your project directory (it
should not require modification), and copy the makefile from
projects/S3_FPGA_UART_GPIO/rtl into you rtl directory. The makefile in
the RTL directory shold be modified to specify any modules that are
used. This is done by replacing 'UART_16550' with the list of used
modules (or blank if no moduels are used) in the line 'IPMODULES =
UART_16550'.

At this point you can test the gateware build by cd gateware/yourproject
and make all. This should create top_bit.h in gateware/yourproject.

Setup software
~~~~~~~~~~~~~~

You will need to edit the config-GCC.mk file which is located in the
qf_apps/qf_gwtestharness/GCC_Project directory. On the line that says

::

   export GATEWARE_DIR=$(PROJ_ROOT)/s3-gateware/projects/S3_FPGA_UART_GPIO

replace the S3_FPGA_UART_GPIO with the name of the directory that
contains your project.

Running make from the GCC_Project directory should now create a
qf_gwtestharness app that includes your project.

The default clocks is 12MHz for both FPGA clocks. If this is not
appropriate, you will need to edit main.c and set the clock values that
you want.

Update the qf_apps/qf_gwtestharness/src/pincfg_table.c to setup S3 pin
configuration as defined in the quickfeather.pcf file in your
projects/yourproject directory

Running qf_gwtestharness
------------------------

qf_gwtestharness sends its output to the UART pins on teh Quickfeeather
board, so you will need to attach a USB to serial adaptor to the
appropriate pins, and then use a terminal emualtor, set to 115200 baud,
to interact with the application.

qf_gwtestharness will send various banner messages to the terminal and
eventually end up at the starting CLI prompt:

::

   ##########################
   Quicklogic QuickFeather Bootloader
   SW Version: qorc-sdk/qf-apps/qf_bootloader(v2) (GCC)
   Jun  7 2020 11:50:58
   ##########################

   User button not pressed: proceeding to load application


   ##########################
   Quicklogic QuickFeather Gateware Test Harness
   SW Version: qorc-sdk/qf_apps/qf_gwtestharness
   Sep 20 2020 14:24:43
   ##########################

   #*******************
   Command Line Interface
   App SW Version: qorc-sdk/qf_apps/qf_gwtestharness
   #*******************
   [0] >

If you type 'help' you will get a list of options:

::

   [0] > help
   help-path: (top)
   gwtest         - General test harness for gateware
   diag           - QuickFeather diagnostic commands
   exit           - exit/leave menu
   help           - show help
   ?              - show help
   help-end:
   [0] >

The easiest way to interact with your gateware is to switch to the
gateware test menu by typing 'gwtest':

::

   [0] > gwtest
   [1] gwtest > help
   help-path: gwtest
   readid         - -- read the gateware id reister (@0x00)
   readreg        -  0xaddr -- read register at specified address
   writereg       - 0xaddr 0xvalue -- write val to specified address
   exit           - exit/leave menu
   help           - show help
   ?              - show help
   help-end:
   [1] gwtest >

The '[1] gwtest >' indicates that you are in the first level submenu
called 'gwtest'. The 'help' shows that there are three commands that you
can issue:

-  *readid* which will read register at address 0x00 (normally the ID
   register)
-  *readreg 0xaddr* which will read the register at address
-  *writereg 0xvalue 0xaddr* which will set the register at addr to
   value

Note that readreg and writereg assume that the register is uint32, and
that the addresses are byte offsets.

Example of *readid*:

::

   [1] gwtest > readid
   id register =: 0xabcd0001
   [1] gwtest >

Example of *readreg*:

::

   [1] gwtest > readreg 0x04
   address = 4
   register at: 0x00000004
   value      : 0x00000100
   [1] gwtest >

Example of *writereg*:

::

   [1] gwtest > writereg 0xbeef 0x10
   address = 48879
   value = 16
   uxValue: 0x00000010
   register at    : 0x0000beef
   register set to: 0x00000010
   [1] gwtest >

Of course this just wrote 0xbeef to the register, whether the register
changed depends on whether the register is writable.
