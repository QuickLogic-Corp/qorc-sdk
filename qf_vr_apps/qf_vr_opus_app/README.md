# Voice recognition with opus encoded audio streaming example application

The purpose of this application is to provide a software framework to easily
plug-in a 3rd party voice recognition engine for keyword detection applications.
The example application waits for an LPSD event indicating a valid speech signal
(WAIT_ON_LPSD event). The example application then enters Keyword detection
state. Audio data is processed by the VR engine to scan for the keyword.
When a keyword is detected, the application enters audio data streaming mode,
opus encoded audio data is sent over SPI to the host. The host quickfeather board in turn
sends the audio data over USB-serial to an attached terminal application program.

This application project should be used with qf_host_app project. 

## Hardware Setup 

Refer [VR opus streaming Hardware Setup] for details on hardware setup

## Building and running the project

1. Use the Makefile provided in qf\_vr\_apps/qf\_vr\_opus\_app/[GCC\_Project folder](GCC\_Project) and 
   an appropriate ARM GCC toolchain to build the project

2. Convert the resulting binary to "C" source file named firmware\_image\_vr\_opus\_app.h.
   Copy this firmware\_image\_vr\_opus\_app.h source file to the [host app include folder](../qf\_host\_app/inc)
   qf_vr_apps/qf_host_app/inc

3. Set the macro FEATURE\_OPUS\_ENCODER in qf_vr_apps/qf_host_app/inc/[Fw\_global\_config.h](../qf\_host\_app/inc/Fw\_global\_config.h) to 1

4. Use the Makefile provided in qf_vr_apps/qf_host_app/GCC_Project [host app GCC\_Project folder](../qf\_host\_app/GCC\_Project) and 
   an appropriate ARM GCC toolchain to build the project

5. Use the flash programming procedure to flash the qf\_host\_app binary to
   a host Quickfeather board.

6. Reset the host Quickfeather board to start running the VR opus encoded streaming 
   application.

## Building using a 3rd party VR engine

The default project does not provide any voice recognition engine, consequently
no keyword detection will be performed. To plug-in the desired 3rd party engine
follow these steps:

1. Implement the functions specified in the qf\_vr\_apps/qf\_vr\_opus\_app/inc/[vr\_engine\_api.h](inc/vr\_engine\_api.h)

2. Update the build system in qf\_vr\_apps/qf\_vr\_opus\_app/[GCC\_Project folder](GCC\_Project) to add the
   implemented source files.

3. Convert the resulting binary to "C" source file named firmware\_image\_vr\_opus_app.h.
   Copy this firmware\_image\_vr\_opus\_app.h source file to the [host app include folder](../qf\_host\_app/inc)
   qf_vr_apps/qf_host_app/inc

4. Set the macro FEATURE\_OPUS\_ENCODER in qf_vr_apps/qf_host_app/inc/[Fw\_global\_config.h](../qf\_host\_app/inc/Fw\_global\_config.h) to 1

5. Use the Makefile provided in qf_vr_apps/qf_host_app/GCC_Project [host app GCC\_Project folder](../qf\_host\_app/GCC\_Project) and 
   an appropriate ARM GCC toolchain to build the project

6. Use the flash programming procedure to flash the qf\_host\_app binary to
   a host Quickfeather board.

7. Reset the host Quickfeather board to start running the VR opus encoded streaming 
   application.

[VR opus streaming Hardware Setup]: ../readme.md#qf_vr_opus_app-coming-soon-companion-app-implementing-vr-host-communications-over-spi-compressing-and-packetizing-audio-with-opus-and-streaming-the-audio-packets-over-spi
