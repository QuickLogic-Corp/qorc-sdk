QORC HelloWorldFPGA Application (AU)
====================================

This test/example contains the the fpga design available as an independent component, it is basically only the FPGA part of the :code:`qf_apps/qf_helloworldhw` application project.

The FPGA design is a simple LED-toggle, which toggles the green LED regularly.


Usage
-----

Once the fpga design is loaded and running 
(load using debugger/reset after flashing on the board), 
you should see the green LED toggling periodically.


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

  fpga: :code:`make clean` -or- :code:`make clean-fpga`

- Build using:

  fpga: :code:`make` -or- :code:`make fpga`

- Load and run the design on the board using JLinkExe, using:

  (assumes the board has been booted in DEBUG mode)

  .. code-block:: bash
      
    make load-jlink

- Flash and run the design on the board using qfprog:
  
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

   On opening the folder, VS Code should prompt to install these "recommended extensions", if not already installed, 
   select :code:`Install All` to automatically install them.


Clean/Build/Load/Flash (VS Code)
********************************

Any "task" can be run in VS Code using the :code:`Terminal > Run Task` menu, which shows a drop down list of tasks

-OR-

Using keyboard shortcuts: :code:`ctrl+p` and then type :code:`task<space>`, which shows a drop down list of tasks

- Clean using:
  
  - fpga: run the :code:`clean-fpga` task

- Build using:

  - fpga: run the :code:`build-fpga` task

- Load and run the design on the board using JLinkExe, using:
  
  (assumes the board has been booted in DEBUG mode)

  run the :code:`load (JLink)` task

- Flash and run the design on the board using qfprog:

  (assumes the board is put into :code:`programming` mode)

  run the :code:`flash` task

  This will show a 'pickstring' drop down menu with the available serial ports in the system, select the appropriate one.
  
  (This is usually :code:`/dev/ttyACM0`)


- :code:`x-get-ports` : this is an **internal** task, which is used by the :code:`flash` task to obtain a list of
  available serial ports on the system to use for flashing. This list is displayed to the user as a 'pickstring'
  dropdown menu, as described in the :code:`flash` task above.


References
~~~~~~~~~~

1. https://code.visualstudio.com/docs/editor/debugging
2. https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug
3. https://mcuoneclipse.com/2021/05/09/visual-studio-code-for-c-c-with-arm-cortex-m-part-4/
