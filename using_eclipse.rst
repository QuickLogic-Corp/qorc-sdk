Using Eclipse With QORC SDK
===========================

The recommended Eclipse for QORC SDK is :code:`Eclipse IDE for Embedded C/C++ Developers`.

The IDE is available for download from : https://projects.eclipse.org/projects/iot.embed-cdt/downloads

The current tested version with QORC SDK is :code:`Eclipse IDE for Embedded C/C++ Developers 2020-09`.

This Eclipse IDE is a continuation of the GNU MCU Eclipse project (https://gnu-mcu-eclipse.github.io/) and is now an Eclipse Incubation Project, and is recommended over the older GNU MCU for new installations.

This guide assumes that all the pre-requisites for the QORC SDK applications are already installed and you are able to build the applications using :code:`make` from the command line.

.. contents::

It is recommended to keep a separate workspace for the QORC SDK applications, but it is not mandatory.

Convert Existing Makefile GCC_Project to an Eclipse Project
-----------------------------------------------------------

All QORC SDK applications have Makefile support, and the :code:`application_dir/GCC_Project/` contains the necessary Makefiles.

To convert this to an Eclipse Project, the following steps can be followed:

1. Create a directory :code:`eclipse_make_project` in the application's :code:`GCC_Project dir`.

2. Create a file :code:`eclipse_custom_build_command.sh` in the application's :code:`GCC_Project dir`.
   
   Copy the below content into the file :

   ::

     #!/usr/bin/env bash

     cd $1
     echo $INSTALL_DIR
     export PATH="$INSTALL_DIR/quicklogic-arch-defs/bin:$INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
     source "$INSTALL_DIR/conda/etc/profile.d/conda.sh"
     conda activate
     make $2

   Save the file, and mark it executable with :code:`chmod +x eclipse_custom_build_command.sh`

3. Open Eclipse and Select Preferred Workspace.

4. Select :code:`File -> New -> Project`
   
   From the :code:`New Project` dialog box, 
   
   Select :code:`Makefile project with existing code` under :code:`C++` and click Next.
   

5. Specify the Project Name ("Use Your Preferred Name") in the :code:`Project Name` text box.
   
   Specify the path to :code:`eclipse_make_project` that was created in Step [1] in the :code:`Existing Code Location` text box
   
   Keep both :code:`C` and :code:`C++` checked in :code:`Languages`
   
   Select :code:`ARM Cross GCC` in the :code:`Toolchain for Indexer Settings` options
   
   Click Finish
   

6. Add files and folder to the Eclipse Virtual Filesystem
   
   Select the following directories in the :code:`File Explorer` from the QORC SDK repo directory:
   
   - BSP
   - HAL
   - FreeRTOS
   - Libraries
   - Tasks

   Drag the selected directories and drop them onto the newly created project in Eclipse IDE
   
   Once dropped, a :code:`File and Folder Operation` dialog pops up.
   
   Choose :code:`Link to files and recreate folder structure with virtual folders` option.
    
   Ensure that the option :code:`Check the Create link locations relative to PROJECT_LOC` is checked.
   
   Click OK.
   
   Any other QORC SDK directories can be added to the eclipse project in the same way.
   

7. Create Virtual Folder named "App". 
   
   Ensure that the newly create project is selected in Eclipse IDE
   
   Select :code:`File -> New -> Folder`
   
   Specify folder name as :code:`App`
   
   Click the :code:`Advanced` tab and ensure to select the option :code:`Folder is not located in the file system (Virtual Folder)`
   
   Click :code:`Finish`
   
   Select the following directories in the :code:`File Explorer` from the application directory:

   - GCC_Project
   - src
   - inc

   Drag the selected directories and drop them onto the newly :code:`App` Virtual Folder in Eclipse IDE

Now, the project structure is ready.

Setup Build Configuration For Project
-------------------------------------

1. Ensure that the newly create project is selected in Eclipse IDE

2. Select :code:`Project -> Properties`
   
   1. Select C/C++ Build from the left pane
   
      Select the :code:`Builder Settings` tab
   
      Deselect :code:`Use default build command`
   
      Enter the following in the :code:`Build command` text box:

      ::

        ${workspace_loc:/${ProjName}}/../eclipse_custom_build_command.sh ${workspace_loc:/${ProjName}}/../

   2. Select sub option :code:`Settings` under :code:`C/C++ Build`
   
      Select :code:`Toolchains` tab and pull-down :code:`Name` option and select :code:`GNU Tools for ARM Embedded Processors (arm-none-eabi-gcc)`
   
   3. Select sub option :code:`Environment` under :code:`C/C++ Build`
   
      Click option :code:`Add`, input :code:`Name` as :code:`INSTALL_DIR` and :code:`Value` as :code:`"Path to the FPGA Toolchain Installation Directory"` (same as in regular setup)
   
      Click option :code:`Add`, input :code:`Name` as :code:`PATH` and :code:`Value` as :code:`"Path to the ARM GCC Toolchain Directory":"$INSTALL_DIR/install/bin:$INSTALL_DIR/install/bin/python:$PATH`
   
   Click :code:`Apply and Close` the :code:`Project Properties`

3. Right-Click on the project in :code:`Project Explorer`, select :code:`Clean Project`

   You should be able to see the project cleaned successfully.

   The :code:`CDT Build Console` output should look to be the same as what you would see while executing :code:`make clean` from the command line.

4. Right-Click on the project in :code:`Project Explorer`, select :code:`Build Project`

   You should be able to see the project build successfully.

   The :code:`CDT Build Console` output should look to be the same as what you would see while executing :code:`make` from the command line.

5. Select the :code:`bin` directory in the :code:`File Explorer` from the application's :code:`GCC_Project/output/` directory.

   Drag the selected directory and drop it onto the :code:`App/GCC_Project/output/` Virtual Folder in Eclipse IDE

   This is required for setting up the Debug Configuration in the next section.



Setup Debug Configuration For Project
-------------------------------------

Segger J-Link
~~~~~~~~~~~~~

1. Install the Segger J-Link Software Package for Ubuntu:

   Recommended to use the :code:`tgz` archive (J-Link Software and Documentation pack for Linux, TGZ archive, 32 or 64 bit according to host architecture) from :

   ::

     https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack

   Tested with v6.92 at the time of writing this guide.

   Extract the archive to any preferred location.

2. Ensure the project is selected in Eclipse IDE :code:`Project Explorer`

   Select :code:`Run -> Debug Configurations`

3. Select :code:`GDB SEGGER J-Link Debugging`, and then click on :code:`New launch configuration` (the top left most icon with only a "+" in the corner)

4. On the :code:`Main` tab :
   
   - Select the :code:`Project` (It should show the project you created)
   
   - Specify the :code:`C/C++ Application` as :code:`App/GCC_Project/output/bin/"application_name".elf` (Ideally, this should have been automatically selected by Eclipse)
   
     Click on :code:`Search Project` and select the correct :code:`elf` file.

5. On the :code:`Debugger` tab :

   - Set the :code:`Executable path` to point to the :code:`JLinkGDBServerCLExe` file in the Segger J-Link Installation above.

   - Set the :code:`Device name` to :code:`Cortex-M4` 

6. On the :code:`Startup` tab :

   - set :code:`Initial Reset and Halt` type to :code:`0`

   - Check the :code:`RAM application (reload after reach reset/restart)` option

   - Uncheck the :code:`Pre-run/Restart reset` option

7. Click on :code:`Debug` button

   The debug session should start launching (Answer :code:`Yes` if Eclipse asks to switch to Debug Perspective)

   You should be able to see the code loaded and debugger halted on :code:`int main()`


OpenOCD
~~~~~~~

:code:`coming soon!`
