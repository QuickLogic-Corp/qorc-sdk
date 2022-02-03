QORC AdvancedFPGA Application (SYMBIFLOW)
===========================================

This test/example contains the :code:`qf_advancedfpga` example, but with the fpga design compiled into a separate binary, and the m4 code compiled into a separate binary.

The original example has the m4 code include the fpga code (in a C header form) into a single m4 binary.

This example test app can be used as a guide for apps which have both m4 code and fpga rtl to be built within the same application project, and flashed as separate binary images.


Usage
-----

This section describes the usage of the current project and how to test it.

Once the code(fpga+m4) is loaded and running 
(load using debugger/debug using debugger/reset after flashing on the board), you should see a banner like below:

::

  ##########################
  Quicklogic QuickFeather Advanced FPGA Example
  SW Version: qorc-sdk/qf_apps/qf_advancedfpga
  Sep 20 2020 14:24:43
  ##########################
  
  #*******************
  Command Line Interface
  App SW Version: qorc-sdk/qf_apps/qf_advancedfpga
  #*******************
  [0] >


| The :code:`ledctlr` submenu option is available, and can be used to set RGB led to change color at specific intervals.
|
| :code:`ledctlr` test sequence:
| At the :code:`[0] >` prompt, which is the level 0 prompt, use: 

  1. :code:`ledctlr` to enter the submenu
  2. :code:`color0 1` sets the color0 (for timeslot0) to blue, you should see the blue led turn on
  3. :code:`color1 2` sets the color1 (for timeslot1) to green, no visible change
  4. :code:`color2 4` sets the color2 (for timeslot2) to red, no visible change
  5. :code:`duration0 500` sets the duration of timeslot0 (for color0)
  6. | :code:`duration1 500` sets the duration of timeslot1 (for color1)
     | Now, color0(blue) should be seen for 500ms, and color1(green) should be seen for 500ms and should repeat.
  7. | :code:`duration2 1000` sets the duration of timeslot2 (for color2)
     | Now, color0(blue) for 500ms, color1(green) for 500ms and color2(red) for 1000ms should be seen, and should repeat.



How To
------

Command Line Usage
~~~~~~~~~~~~~~~~~~

Note that, all the commands below are run from the root of this directory.

Initialize Environment
**********************

Before clean/build/load/flash, ensure that the bash environment is setup by doing the below:

1. Ensure that QORC-SDK is initialized and ready (to use :code:`JLinkExe` or :code:`OpenOCD` for loading, :code:`qfprog` for flashing):

   ::

     cd <QORC_SDK_PATH> && source envsetup.sh && cd -


Clean/Build/Load/Flash (Command Line)
*************************************

- Clean using:

  fpga: :code:`.scaffolding/clean_fpga.sh`

  m4: :code:`make -C GCC_Project/ clean`

- Build using:

  fpga: :code:`.scaffolding/build_fpga.sh`
  
  -OR-

  fpga: :code:`ql_symbiflow -compile -src fpga/rtl -d ql-eos-s3 -t AL4S3B_FPGA_top -v AL4S3B_FPGA_IP.v AL4S3B_FPGA_QL_Reserved.v AL4S3B_FPGA_Registers.v AL4S3B_FPGA_Top.v LED_controller.v -p quickfeather.pcf -P PU64 -dump binary openocd jlink``


  m4: :code:`make -C GCC_Project/`

- Load and run the design on the board using JLinkExe, using:

  (assumes the board has been booted in DEBUG mode)

  ::

    .scaffolding/load_fpga_m4_jlink.sh

- Load and run the design on the board using OpenOCD, using:

  (assumes the board has been booted in DEBUG mode)

  ::

    .scaffolding/load_fpga_m4_openocd_gdb.sh --openocd-if-cfg=<PATH_TO_INTERFACE_CFG>

  The INTERFACE_CFG file depends on the debug adapter chosen.

  Here are a few common adapters that can be used with the EOS_S3:
  
  1. JLink EDU/EDU-Mini/BASE... : :code:`--openocd-if-cfg=.scaffolding/jlink_swd.cfg` (available in the current dir)
  2. FT2232H Breakout Boards : :code:`--openocd-if-cfg=.scaffolding/ft2232h_swd.cfg` (available in the current dir)
  3. DAPLink Boards: :code:`--openocd-if-cfg=interface/cmsis-dap.cfg` (available in the OpenOCD install scripts dir)

  Practically, any adapter that supports OpenOCD and SWD can be used with the appropriate cfg file passed in.

- Flash and run the design on the board using qfprog:
  
  (assumes the board is put into :code:`programming` mode)

  ::

    .scaffolding/flash_fpga_m4.sh --port=/dev/ttyACM0

  -OR-

  directly using qfprog:

  ::

    qfprog --port /dev/ttyACM0 --m4app GCC_Project/output/bin/qorc_advancedfpga.bin --appfpga fpga/rtl/AL4S3B_FPGA_top.bin --mode fpga-m4 --reset


VS Code Usage
~~~~~~~~~~~~~

Initialize Project Configuration
********************************

The first time the project is going to be used from VS Code, we need to do the following:

1. copy :code:`.vscode/settings.template.jsonc` as :code:`.vscode/settings.json`

   Ensure the following variables are correctly defined:

   ::

     "qorc_sdk_path" : "${workspaceFolder}/../..",

   In VS Code:

   :code:`${env:HOME}` refers to $HOME of the current user

   :code:`${workspaceFolder}` refers to the current directory

   Remaining variables don't need to be changed

2. Open the current directory in VS Code using :code:`File > Open Folder` menu
   
   To be able to run the 'flash' task, remember to install the extension: :code:`augustocdias.tasks-shell-input`

   To be able to 'debug' the code with gdb, remember to install the extension: :code:`marus25.cortex-debug`

   On opening the folder, VS Code should prompt to install "recommended extensions" and this can install them automatically.


Clean/Build/Load/Flash (VS Code)
********************************

Any "task" can be run in VS Code using the :code:`Terminal > Run Task` menu, which shows a drop down list of tasks

-OR-

Using keyboard shortcuts: :code:`ctrl+p` and then type :code:`task<space>`, which shows a drop down list of tasks

- Clean using:
  
  - fpga: :code:`clean-fpga` task
  - m4: :code:`clean-m4` task
  - both: :code:`clean` task

- Build using:

  - fpga: :code:`build-fpga` task
  - m4: :code:`build-m4` task
  - both: :code:`build` task

- Load and run the design on the board using JLinkExe, using:
  
  (assumes the board has been booted in DEBUG mode)

  :code:`load-fpga-m4 (JLink)` task

- Load and run the design on the board using OpenOCD, using:

  (assumes the board has been booted in DEBUG mode)

  :code:`load-fpga-m4 (OpenOCD)` task

  This will show a drop down menu with the options of debug adapters currently tested:

  - JLink
  - FT2232H
  - DAPLink (CMSIS-DAP)

  select the appropriate one.

- Flash and run the design on the board using qfprog:

  (assumes the board is put into :code:`programming` mode)

  :code:`flash-fpga-m4` task

  This will show a drop down menu with the available serial ports in the system, select the appropriate one.
  
  (This is usually :code:`/dev/ttyACM0`)

- :code:`debug-load-fpga (JLink)` : this is a special task used only while debugging the code with JLink.

  Refer to the Debug sections for details.


Debug
*****

- Debug the code via JLink :

  1. To bring up the :code:`Run and Debug` view, select the Run icon in the Activity Bar on the side of VS Code.
  
  2. Select :code:`Debug (JLink)` from the drop down at the top of the side bar
  
  3. Start Debugging by clicking the green :code:`Play Button`
  
  4. The code should load and break at :code:`main()`
  
  5. Run the task :code:`debug-load-fpga (JLink)` at this point, to load the FPGA design
  
  6. Resume/Continue debugging using the blue :code:`Continue/Break` button at the top or using :code:`F8`


- Debug the code via OpenOCD :

  1. To bring up the :code:`Run and Debug` view, select the Run icon in the Activity Bar on the side of VS Code.
  
  2. Select :code:`Debug (OpenOCD)` from the drop down at the top of the side bar
  
  3. Start Debugging by clicking the green :code:`Play Button`
  
  4. A drop-down menu appears to select the debug adapter being used, currently the choices are:
   
     - :code:`jlink_swd.cfg`
     - :code:`ft2232h_swd.cfg`
     - :code:`interface/cmsis-dap.cfg`

     More can be added in the :code:`launch.json` file.
     
     Select the appropriate one.

  5. The fpga bitstream (.openocd) should get loaded, then the m4 code should load and break at :code:`main()`
  
  6. Resume/Continue debugging using the blue :code:`Continue/Break` button at the top or using :code:`F8`


- Common Debugging Steps with the :code:`Cortex-Debug` extension in VS Code:

  1. Place breakpoints in the code by clicking near the line number
  
  2.  Use the :code:`Step Over`, :code:`Step Into`, :code:`Step Out`, :code:`Restart`, :code:`Stop` buttons to control the debugging session

References
~~~~~~~~~~

1. https://code.visualstudio.com/docs/editor/debugging
2. https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug
3. https://mcuoneclipse.com/2021/05/09/visual-studio-code-for-c-c-with-arm-cortex-m-part-4/
