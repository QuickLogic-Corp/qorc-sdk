This is EclipseReadme.txt file for creating and building qorc-sdk Projects using
GCC Tools and the provided Makefiles using Eclipse IDE. 

This document describes creating an Eclipse project for builiding "only" using
the provided makefiles and debugging using J-Link.

See the "Docs/QuickAI-Eclipse-IDE.docx" for more detailed description of setting
up and using Eclipse Tools with qorc-sdk projects as well as creating new projects
without any makefiles.


Overview:
========

This document details the installation procedure to setup an Eclipse IDE based 
development environment for qorc-sdk projects and provides instructions for migrating
existing Makefile projects to Eclipse IDE environment.

Requirements:
=============

The following software tools and plug-ins are needed in order to setup the 
Eclipse IDE environment

- ARM GCC toolchain available for download from
    - https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads/ 
- GNU Make program Version 3.81 available for download from
    - https://sourceforge.net/projects/gnuwin32/files/make/3.81
- Eclipse IDE available for download from
    - http://www.eclipse.org/downloads/ 
- ARM Cross Compiler plug-in and J-Link debugger plug-in for Eclipse available from
     - http://gnu-mcu-eclipse.netlify.com/v4-neon-updates

Tools installation:
===================

1. ARM GCC toolchain installation
   Download and install GNU ARM embedded toolchain from 
   https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads/

2. GNU make program installation
    Download the GNU make utility from
    https://sourceforge.net/projects/gnuwin32/files/make/3.81
    Install to the folder C:\GnuWin32\
    
    Note: 
      Please make sure the PATH contains "C:\GnuWin32\bin" as the first "make utility".
      Other versions of Make may "not work" with the Makefiles provided.

3. Install Eclipse IDE for C/C++ Developers
    Download Eclipse Photon and install Eclipse IDE for C/C++ Developers
    -Install to  the folder C:\Eclipse\cpp-photon 
    Download Eclipse Photon from http://www.eclipse.org/downloads/ 
    For 64-bit Windows use the link: 
    (https://www.eclipse.org/downloads/download.php?file=/oomph/epp/photon/R/eclipse-inst-win64.exe)

4. Install J-Link debugger and ARM Cross Compiler plug-ins
    Start the Eclipse IDE, from the Eclipse main menu, select Help -> Install New Software
    Add GNU ARM Eclipse plug-in site:
    http://gnu-mcu-eclipse.netlify.com/v4-neon-updates

    Select the following plug-ins to install
    GNU MCU C/C++ ARM Cross Compiler
    GNU MCU C/C++ J-Link Debugging

    Follow the instructions to install the plug-ins.
    The installer warns about the unsigned content installation, accept and continue.

Setup Eclipse for Cross ARM GCC and GNU Make build tools:
=========================================================

    Open Eclipse IDE, on the main menu select Window -> Preferences
    On the "Preferences" dialog box, navigate to MCU -> "Global Build Tools Path"
       - Enter Build tools folder location as "C:\GnuWin32\bin" as setup above in 
         the installation procedures.

Convert an existing Makefile GCC_Project to an Eclipse project:
===============================================================

To convert an existing Makefile based project to the Eclipse IDE environment 
and reuse these existing Makefiles for build, follow the below procedure.

1. Open Eclipse IDE.
   You can choose to create workspace as the C:\Eclipse\cpp-photon\workspace 
2. Select File -> New -> Project
   (If this is fresh installation, select "Import a project with a working Makefile")
3. From the New Project Wizard dialog box, 
   Select "Makefile project with existing code" and click Next.
   On the next screen, 
      - Select "ARM Cross GCC" in the Toolchain for Indexer Settings window pane, 
      - Specify the Project Name ("YourProject") as the "Project Name" text box 
        of the "New Project" dialog box
      - Create a new folder named "Eclipse_Make_Project" in "App/YourProject/GCC_Project" folder
        and specify this folder location to save eclipse project settings in the
        "Existing Code Location" text box of the "New Project" diaglog box, and
      - Click Finish
4. Add files and folder to the Eclipse Virtual filesystem
      Drag and drop the folder QL-EOSS3-SW/BSP to the newly created project
           On the "File and Folder Operation" dialog box, choose
              - Link to files and recreate folder structure with virtual folders and
              - Check the Create link locations relative to PROJECT_LOC
      Similarly drag and drop other folders:
           FReeRTOSv8_2_2, HAL, Libraries, and Tasks
      Create Virtual Folder named "App". 
           On the Eclipse IDE, select <YourProject>, Click on File -> New -> Folder
           On the "New Folder" dialog box, specify the folder name as "App", 
           click the "Advanced" tab and select the radio button "Folder is not
           located in the file system (Virtual Folder)" and click Finish
      Create Virtual folder named "YourProject"
      Drag and drop from your "App/YourProject" the following folders: 
          GCC_Project/, src/, and inc/ folders to the newly created 
          "App/YourProject" virtual folder.

Build setup:
============

    In the Eclipse IDE, select the project to build
    In the Eclipse IDE's main menu:
    - Select Project -> Properties 
    - Select C/C++ Build from the left pane of the "Properties for <YourProject>"
      dialog box.
      On the "Builder Settings" tab
         - unselect "Use default build command"
         - Enter the following in the Build command box:
           make -C ${workspace_loc:/${ProjName}}/../
      Click on sub -option "Settings" under "C/C++ Build"
          Select "Toolchains" Tab and pull-down Name option and select
            "GNU Tools for ARM Embedded Processors (arm-none-eabi-gcc)"
    - Select the Environment from the C/C++ Build 
         - Double-click on the PATH environment variable
         - Add the folder C:\GnuWin32\bin to the PATH environment variable
         - Add the folder where the ARM GCC toolchain is installed to the PATH 
           environment variable (for example, C:\Program Files (x86)\GNU Tools ARM Embedded\7 2017-q4-major\bin)
    - Select Project -> "Build Project"
    - Add the 3 output files from GCC_project/output/bin folder to the virtual folder GCC_project/output/bin
      (else the you would not be able select the project in the next step)

Setup the debug environment for SEGGER JLink debugging:
=======================================================

    On the main menu, Click Run -> Debug Configurations 
    Select "GDB SEGGER J-Link Debugging", and then click on "New launch configuration" 
    (the top left most icon with only a "+" in the corner)
    On the "Main" tab
       - Select the project name "YourProject"
       - Specify the C/C++ Application 
         Click on Search Project and select "YourProject.out"
         (Note: "YourProject.out" will not show up in search unless you Build the project successfuly)
         
    On the "Debugger" tab set the "Device Name" to "Cortext-M4"
    On the "Startup" tab, 
       - set "Initial Reset and Halt"  type as 0, 
       - Check the "RAM application (reload after reach reset/restart)"
       - Uncheck "Pre-run/Restart reset"
          
