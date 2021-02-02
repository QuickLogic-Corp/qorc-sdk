This is Readme.txt file for creating and building QuickAI Projects using
open-source GCC Tools and the provided Makefiles.

Requirements:

The following open-source tools are needed in order to build using Makefiles.

- ARM GCC toolchain available for download from
    - https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads/
- GNU Make program Version 3.81 available for download from
    - https://sourceforge.net/projects/gnuwin32/files/make/3.81

Overview:

The following is a brief description of Makefile Architecture used for QuickAI SDK.

    There are 2 configuration setup files and a main Makefile under GCC_Project directory.

    1. config.mk - selects the operating system environment to use, "Windows" or "Linux"
      -set BUILD_SYS=WINCMD - for Windows or comment it out for Linux
      Note:
        On Windows 10, there may be a problem to "auto-detect" the ARM Gnu Embedded Tool
        Chain path. Then explicit set the QORC_TC_PATH to where the ARM GCC tools are installed.
      -
    2. config-GCC.mk - sets the GCC compiler options, source directories and "output file"

    3. Makefile - This is the main Makefile which calls the sub makefiles to build

       makefiles/ folder - There are separate makefiles for each directory that needs to be
            included in the given project. Each of the makefile starts with Makefile_ and ends
            with the Directory name (for example Makefile_HAL)
       All the makefiles are exactly the same except for *_SRCS and *_DIR.
        (The Makefile_Libraries is one of the exceptions and builds all the Libraries.)

       Makefile_common - is common to all the makefiles and "should not be changed".

    4. output/ folder - all the output is placed in this directory
             depend/ - all the depdencies created by the Make are placed in this folder
             bin/ - the final output is copied to this folder if the build is succesful


Adding New makefiles:
    - in config-GCC.mk
         -add the "export YOURTEST_DIR" to point to your directory,for example, "YourTest"
         -append the include path to INCLUDE_DIRS if there are include folders
    - make a copy of one of the makefiles in the makefiles/ folder and rename it to
      reflect the name of the folder you want to include (for example Makefile_YourTest)
    - change the all *_DIR references to YOURTEST_DIR and *_SRCS to YOURTEST_SRCS
    - in the main Makefile
       - add a new Target to the line starting with " $(OUTPUT_FILE).o: "
       - create build commands for the new Target as .PHONY objects point to your new makefile

Building:
     Make sure "C:\GnuWin32\bin" is the first in the Path and QORC_TC_PATH is properly set.
     - At the command prompt just type "make" to build all
     - Type "make clean" to delete all the object files and output built
     - type "make target" to selectively build a Target (for example, "make HAL")
     - type "make clean_target" to selectilve clean a Target (for example, "make clean_HAL")

   On a successfull build, output/bin will contain
     - *.bin, *.out, *.map files

