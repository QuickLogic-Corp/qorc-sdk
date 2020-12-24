Voice recognition with audio streaming example application
==========================================================

The purpose of this application is to provide a software framework to
easily plug-in a 3rd party voice recognition engine for keyword
detection applications. The example application waits for an LPSD event
indicating a valid speech signal (WAIT_ON_LPSD event). The example
application then enters Keyword detection state. Audio data is processed
by the VR engine to scan for the keyword. When a keyword is detected,
the application enters audio data streaming mode, audio data is sent
over SPI to the host. The host quickfeather board in turn sends the
audio data over USB-serial to an attached terminal application program.

This application project should be used with qf_host_app project.

Hardware Setup
--------------

Refer `VR raw streaming Hardware
Setup <../README.rst#qf_vr_raw_app-companion-app-implementing-vr-host-communications-over-spi-packetizing-raw-audio-with-and-streaming-the-audio-packets-over-spir>`__
for details on hardware setup

Building and running the project
--------------------------------

1. Use the Makefile provided in qf_vr_apps/qf_vr_raw_app/GCC_Project
   folder and an appropriate ARM GCC toolchain to build the project

2. Convert the resulting binary to "C" source file named
   firmware_raw_image.h. Copy this firmware_raw_image.h source file to
   the qf_apps/qf_host_app/inc folder

3. Use the Makefile provided in qf_vr_apps/qf_host_app/GCC_Project
   folder and an appropriate ARM GCC toolchain to build the project

4. Use the flash programming procedure to flash the qf_host_app binary
   to a host Quickfeather board.

5. Reset the host Quickfeather board to start running the VR raw
   streaming application.

Building using a 3rd party VR engine
------------------------------------

The default project does not provide any voice recognition engine,
consequently no keyword detection will be performed. To plug-in the
desired 3rd party engine follow these steps:

1. Implement the functions specified in the
   qf_vr_apps/qf_vr_raw_app/inc/vr_engine_api.h

2. Update the build system in qf_vr_apps/qf_vr_raw_app/GCC_Project to
   add the implemented source files.

3. Convert the resulting binary to "C" source file named
   firmware_raw_image.h. Copy this firmware_raw_image.h source file to
   the qf_apps/qf_host_app/inc folder

4. Use the Makefile provided in qf_vr_apps/qf_host_app/GCC_Project
   folder and an appropriate ARM GCC toolchain to build the project

5. Use the flash programming procedure to flash the qf_host_app binary
   to a host Quickfeather board.

6. Reset the host Quickfeather board to start running the VR raw
   streaming application.
