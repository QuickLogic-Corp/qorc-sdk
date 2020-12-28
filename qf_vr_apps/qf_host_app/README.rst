Host communication example application
======================================

The purpose of this host application is to provide a software framework
to illustrate the use of communicating with a device Quickfeather board
to run voice recognition applications, and stream audio data from the
device Quickfeather board over to the host Quickfeather board either in
raw 16-bit format or in an Opus encoded bit stream. This example
application sends the received audio data to the PC over USB-serial
using base64 encoding. This base64 encoded data can later be decoded
using the `Base64 Audio
Decoder <../../Tools/uartaudio/convb64towav.py>`__

Host - Device SPI communcation pin mapping
------------------------------------------

The table below gives pin mapping for SPI communication between the Host
Quickfeather board and the device Quickfeather board.

+-------------+-------------------------------+---------------------------------+
| Signal Name | S3 Host (Quickfeather header) | S3 Device (Quickfeather header) |
+=============+===============================+=================================+
| CLK         | IO34 (J3.6)                   | IO16 (J2.8)                     |
+-------------+-------------------------------+---------------------------------+
| MISO        | IO36 (J3.4)                   | IO17 (J2.7)                     |
+-------------+-------------------------------+---------------------------------+
| MOSI        | IO38 (J3.5)                   | IO19 (J2.6)                     |
+-------------+-------------------------------+---------------------------------+
| CSn         | IO27 (J2.2)                   | IO20 (J2.5)                     |
+-------------+-------------------------------+---------------------------------+
| H2D INT     | IO11 (J8.6)                   | IO12 (J8.7)                     |
+-------------+-------------------------------+---------------------------------+
| D2H INT     | IO12 (J8.7)                   | IO43 (J2.9)                     |
+-------------+-------------------------------+---------------------------------+
| D2H ACK     | IO13 (J3.8)                   | IO11 (J8.6)                     |
+-------------+-------------------------------+---------------------------------+
| H2D ACK     | IO30 (J3.10)                  | IO13 (J3.8)                     |
+-------------+-------------------------------+---------------------------------+
| RSTn        | IO6 (J8.10)                   | RSTn (J3.16)                    |
+-------------+-------------------------------+---------------------------------+

Hardware Setup
--------------

Refer `VR raw streaming Hardware
Setup <../readme.md#qf_vr_raw_app-companion-app-implementing-vr-host-communications-over-spi-packetizing-raw-audio-with-and-streaming-the-audio-packets-over-spir>`__
for details on hardware setup

Building and running the project
--------------------------------

1. Use the Makefile provided in qf_vr_apps/qf_host_app/GCC_Project
   folder and an appropriate ARM GCC toolchain to build the project

2. Use the flash programming procedure to flash the qf_host_app binary
   to a host Quickfeather board.

3. Reset the host Quickfeather board to start running the VR raw
   streaming application.

`Base64 Audio Decoder <../../Tools/uartaudio/convb64towav.py>`__
----------------------------------------------------------------

Base64 audio data received over the USB-serial terminal can be decoded
using this tool to convert to WAV file format. The resulting WAV file
can be opened using audio tools such as Audacity for further analysis.

::

   usage: convb64towav.py [-h] [-i 'file'] [-o 'file']

   Info - Converts Base64 encoded file into an Audio WAV file. An input file with
   Base64 encoded data must be supplied. Output file names is optional. An output
   Audio file is always generated and saved with .wav extension. If no output
   file is specified, same name as input file with .wav extension will be used.

   optional arguments:
     -h, --help            show this help message and exit
     -i 'file', --input 'file'
                           Input Base64 encoded file
     -o 'file', --output 'file'
                           Output WAV file
