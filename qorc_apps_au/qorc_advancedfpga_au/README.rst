QORC AdvancedFPGA Application (AU)
==================================

This test/example contains the :code:`qf_advancedfpga` example, but with the fpga design compiled into a separate binary, and the m4 code compiled into a separate binary.

The original example has the m4 code include the fpga code (in a C header form) into a single m4 binary.

This example test app can be used as a guide for apps which have both m4 code and fpga rtl to be built within the same application project, and flashed as separate binary images.


Usage
-------


Once the code(fpga+m4) is loaded and running 
(load using debugger/debug using debugger/reset after flashing on the board), you should see a banner like below:

.. code-block:: none

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

:code:`note: all the commands below are run from the root of this directory.`

Initialize Environment
**********************

Before clean/build/load/flash, ensure that the bash environment is setup by doing the below:

:code:`note: the order below is important, initialize Aurora *after* QORC SDK is initialized!`

1. Ensure that QORC-SDK is initialized and ready:

   .. code-block:: bash

     source <QORC_SDK_PATH>/envsetup.sh

2. Ensure Aurora is initialized and ready: (assumes install path is :code:`$HOME/aurora_64`)

   .. code-block:: bash

     cd $HOME/aurora_64 && source setup_au.sh && export quicklogic_LICENSE=<PATH_TO_LIC_FILE> && cd -


Clean/Build/Load/Flash (Command Line)
*************************************

- Clean using:

  fpga: :code:`make clean-fpga`

  m4: :code:`make clean-m4`

  both: :code:`make clean`

- Build using:

  fpga: :code:`make fpga`

  m4: :code:`make m4`

  both: :code:`make`

- Load and run the code/design on the board using JLinkExe, using:

  (assumes the board has been booted in DEBUG mode)

  .. code-block:: bash

    make load-jlink

- Flash and run the code/design on the board using qfprog:
  
  (assumes the board is put into :code:`programming` mode)

  .. code-block:: bash

    export QORC_PORT=/path/to/serial/port   # needs to be done only once in current shell
    make flash

  Set the serial port as applicable, this is generally :code:`export QORC_PORT=/dev/ttyACM0`


VS Code Usage
~~~~~~~~~~~~~

Dependencies
************

- | VS Code Extension: :code:`ms-vscode.cpptools`
  | link: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
  | why: C/C++ Intellisense, Debugging
  |

- | VS Code Extension: :code:`marus25.cortex-debug`
  | link: https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug
  | why: Cortex-M Debug Launch Configuration
  |

- | VS Code Extension: :code:`augustocdias.tasks-shell-input`
  | link: https://marketplace.visualstudio.com/items?itemName=augustocdias.tasks-shell-input
  | why: Scan serial-ports for :code:`flash` task, Select FPGA '.openocd' file for :code:`Debug (OpenOCD)` debug launch config
  |


Initialize Project Configuration
********************************

The first time the project is going to be used from VS Code, we need to do the following:

1. copy :code:`.vscode/settings.template.jsonc` as :code:`.vscode/settings.json`

   Ensure the following variables are correctly defined:

   .. code-block:: none

     "qorc_sdk_path" : "${workspaceFolder}/../..",
     "aurora_install_path" : "${env:HOME}/aurora_64",
     "aurora_license_path" : "${env:HOME}/aurora_64/aurora.lic"

   In VS Code:

   :code:`${env:HOME}` refers to $HOME of the current user

   :code:`${workspaceFolder}` refers to the current directory

   Remaining variables don't need to be changed.

2. Open the current directory in VS Code using :code:`File > Open Folder` menu
   
   To be able to run the 'flash' task, remember to install the extension: :code:`augustocdias.tasks-shell-input`

   To be able to 'debug' the code with gdb, remember to install the extension: :code:`marus25.cortex-debug`

   On opening the folder, VS Code should prompt to install these "recommended extensions", if not already installed, 
   select :code:`Install All` to automatically install them.


Clean/Build/Load/Flash (VS Code)
********************************

Any "task" can be run in VS Code using the :code:`Terminal > Run Task` menu, which shows a drop down list of tasks

-OR-

Using keyboard shortcuts: :code:`ctrl+p` and then type :code:`task<space>`, which shows a drop down list of tasks

- Clean using:
  
  - fpga: run the :code:`clean-fpga` task
  - m4: run the :code:`clean-m4` task
  - both: run the :code:`clean` task

- Build using:

  - fpga: run the :code:`build-fpga` task
  - m4: run the :code:`build-m4` task
  - both: run the :code:`build` task

- Load and run the code/design on the board using JLinkExe, using:
  
  (assumes the board has been booted in DEBUG mode)

  run the :code:`load (JLink)` task

- Flash and run the code/design on the board using qfprog:

  (assumes the board is put into :code:`programming` mode)

  run the :code:`flash` task

  This will show a 'pickstring' drop down menu with the available serial ports in the system, select the appropriate one.
  
  (This is usually :code:`/dev/ttyACM0`)

- :code:`load-fpga-debug (JLink)` : This is a special task required only while debugging the code with JLink.

  Refer to the Debug sections for details.

- :code:`x-get-ports` : this is an **internal** task, which is used by the :code:`flash` task to obtain a list of
  available serial ports on the system to use for flashing. This list is displayed to the user as a 'pickstring'
  dropdown menu, as described in the :code:`flash` task above.


Debug
*****

- Debug the code via JLink :

  1. To bring up the :code:`Run and Debug` view, select the Run icon in the Activity Bar on the side of VS Code.
  
  2. Select :code:`Debug (JLink)` from the drop down at the top of the side bar
  
  3. Start Debugging by clicking the green :code:`Play Button`
  
  4. The code should load and break at :code:`main()`
  
  5. Run the :code:`load-fpga-debug (JLink)` task at this point, to load the FPGA design
  
  6. Resume/Continue debugging using the blue :code:`Continue/Break` button at the top or using :code:`F8`


- Common Debugging Steps with the :code:`Cortex-Debug` extension in VS Code:

  1. Place breakpoints in the code by clicking near the line number
  
  2.  Use the :code:`Step Over`, :code:`Step Into`, :code:`Step Out`, :code:`Restart`, :code:`Stop` buttons to control the debugging session

References
~~~~~~~~~~

1. https://code.visualstudio.com/docs/editor/debugging
2. https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug
3. https://mcuoneclipse.com/2021/05/09/visual-studio-code-for-c-c-with-arm-cortex-m-part-4/
