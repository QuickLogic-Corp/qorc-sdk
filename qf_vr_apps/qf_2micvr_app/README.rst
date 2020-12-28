2-Mic Voice recognition keyword detection example application
=============================================================

The purpose of this application is to provide a software framework to
easily plug-in a 3rd party voice recognition engine for keyword
detection applications and plug-in a 3rd party noise suppression
algorithm (for example beamforming) using stereo input. The example
application waits for an LPSD event indicating a valid speech signal
(WAIT_ON_LPSD event). The example application then enters Keyword
detection state. Audio data is preprocess first using simple mix of left
and right channels and the resulting mono output is processed by the VR
engine to scan for the keyword.

Building and running the project
--------------------------------

1. Use the Makefile provided in qf_vr_apps/qf_2micvr_app/GCC_Project
   folder and an appropriate ARM GCC toolchain to build the project

2. Use the flash programming procedure to flash the binary to
   Quickfeather board.

3. Reset the board to start running the 2-Mic VR application.

Building using a 3rd party VR engine
------------------------------------

The default project does not provide any voice recognition engine,
consequently no keyword detection will be performed. To plug-in the
desired 3rd party engine follow these steps:

1. Implement the functions specified in the
   qf_vr_apps/qf_2micvr_app/inc/vr_engine_api.h

2. Update the build system in qf_vr_apps/qf_2micvr_app/GCC_Project to
   add the implemented source files.

3. Build the projectand flash the generated application to the
   Quickfeather board.

4. Use the flash programming procedure to flash the binary to
   Quickfeather board.

5. Reset the board to start running the 2-Mic VR application.

Adding a 3rd party noise suppression
------------------------------------

The default project provides a simple mix of left and right channels as
the noise suppression in the Library file ql_audio_preproc.c. To replace
this, implement datablk_pe_config_ql_pre_process and
datablk_pe_process_ql_pre_process functions and exclude
Libraries/Audio/src/ql_audio_preproc.c from the build system (See
comments in GCC_Project/makefiles/Makefile_Audio)
