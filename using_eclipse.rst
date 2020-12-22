===========================
Using Eclipse With QORC SDK
===========================

The recommended Eclipse edition for QORC SDK is Eclipse IDE for Embedded C/C++ Developers.

The IDE is available for download from : https://projects.eclipse.org/projects/iot.embed-cdt/downloads

The current test version with QORC SDK is : Eclipse IDE for Embedded C/C++ Developers 2020-09.

This Eclipse IDE is a continuation of the GNU MCU Eclipse project (https://gnu-mcu-eclipse.github.io/) and is now an Eclipse Incubation Project, and is recommended over the older GNU MCU for new installations.

This guide assumes that all the pre-requisites for the QORC SDK applications are already installed and you are able to build the applications using :code:`make` from the command line.

.. contents::

It is recommended to keep a separate workspace for the QORC SDK applications, but it is not mandatory.

Convert Existing Makefile GCC_Project to an Eclipse Project
===========================================================

All QORC SDK applications have Makefile support, and the :code:`application_dir/GCC_Project/` contains the necessary Makefiles.

To convert this to an Eclipse Project, the following steps can be followed:

1. Create a directory :code:`eclipse_make_project` in the application's GCC_Project dir.

2. | Create a file :code:`eclipse_custom_build_command.sh` in the application's GCC_Project dir.
   |
   | Copy the below content into the file :

   ::

     #!/usr/bin/env bash

     cd $1
     echo $INSTALL_DIR
     export PATH="$INSTALL_DIR/quicklogic-arch-defs/bin:$INSTALL_DIR/quicklogic-arch-defs/bin/python:$PATH"
     source "$INSTALL_DIR/conda/etc/profile.d/conda.sh"
     conda activate
     make

   Save the file, and mark it executable with :code:`chmod +x eclipse_custom_build_command.sh`

3. Open Eclipse and Select Preferred Workspace.

4. | Select :code:`File -> New -> Project`
   |
   | From the :code:`New Project` dialog box, 
   |
   | Select :code:`Makefile project with existing code` under :code:`C++` and click Next.
   |

5. | Specify the Project Name ("Use Your Preferred Name") in the :code:`Project Name` text box.
   |
   | Specify the path to :code:`eclipse_make_project` that was created in Step [1] in the :code:`Existing Code Location` text box
   |
   | Keep both :code:`C` and :code:`C++` checked in :code:`Languages`
   |
   | Select :code:`ARM Cross GCC` in the :code:`Toolchain for Indexer Settings` options
   |
   | Click Finish
   |

6. | Add files and folder to the Eclipse Virtual Filesystem
   |
   | Select the following directories in the :code:`File Explorer` from the QORC SDK repo directory:
   
   - BSP
   - HAL
   - FreeRTOS
   - Libraries
   - Tasks

   | Drag the selected directories and drop them onto the newly created project in Eclipse IDE
   |
   | Once dropped, a :code:`File and Folder Operation` dialog pops up.
   |
   | Choose :code:`Link to files and recreate folder structure with virtual folders` option.
   | 
   | Ensure that the option :code:`Check the Create link locations relative to PROJECT_LOC` is checked.
   |
   | Click OK.
   |
   | Any other QORC SDK directories can be added to the eclipse project in the same way.
   |

7. | Create Virtual Folder named "App". 
   |
   | Ensure that the newly create project is selected in Eclipse IDE
   |
   | Select :code:`File -> New -> Folder`
   |
   | Specify folder name as :code:`App`
   |
   | Click the :code:`Advanced` tab and ensure to select the option :code:`Folder is not located in the file system (Virtual Folder)`
   |
   | Click :code:`Finish`
   |
   | Select the following directories in the :code:`File Explorer` from the application directory:

   - GCC_Project
   - src
   - inc

   | Drag the selected directories and drop them onto the newly :code:`App` Virtual Folder in Eclipse IDE

Now, the project structure is ready.

Setup Build Configuration For Project
=====================================

1. Ensure that the newly create project is selected in Eclipse IDE

2. | Select :code:`Project -> Properties`
   |
   | Select C/C++ Build from the left pane
   |
   | Select the :code:`Builder Settings` tab
   |
   | unselect :code:`Use default build command`
   |
   | Enter the following in the :code:`Build command` text box:

   ::

     ${workspace_loc:/${ProjName}}/../eclipse_custom_build_command.sh ${workspace_loc:/${ProjName}}/../

   | Select sub option :code:`Settings` under :code:`C/C++ Build`
   |
   | Select :code:`Toolchains` tab and pull-down :code:`Name` option and select :code:`GNU Tools for ARM Embedded Processors (arm-none-eabi-gcc)`
   |
   | Select sub option :code:`Environment` under :code:`C/C++ Build`
   |
   | Click option :code:`Add...`, input :code:`Name` as :code:`INSTALL_DIR` and :code:`Value` as :code:`"Path to the FPGA Toolchain Installation Directory"` (same as in regular setup)
   |
   | If Variable :code:`PATH` is not present, Click option :code:`Add...`, input :code:`Name` as :code:`PATH` and :code:`Value` as :code:`"Path to the ARM GCC Toolchain Directory":"$INSTALL_DIR/install/bin:$INSTALL_DIR/install/bin/python:$PATH`
   |
   | Click :code:`Apply and Close`
   |

3. Select :code:`Project -> Build Project`

You should be able to see the project build successfully.

The :code:`CDT Build Console` output should look to be the same as what you would see while executing :code:`make` from the command line.


Setup Debug Configuration For Project
=====================================

(Coming Soon)
