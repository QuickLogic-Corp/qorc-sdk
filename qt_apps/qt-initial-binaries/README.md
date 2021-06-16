# qt-initial-bins
Binary files used to initialize a Thing+ board

This repo contains the three binaries that are used to initialize a Thing+ board during manufacturing:
* qt_bootloader.bin -- the bootloader program; this binaries uses the USB2Serial port for programming the Thing+ Flash device, via EOS S3
* qf_bootloader_dual.bin -- the bootloader program; this binaries uses the USB2Serial port or the EOS S3 UART port for programming the Thing+ Flash device; selection is done by enterning the correct COM port value; this should be the default bootloader for the THing+ board
* qt_bootfpga.bin -- the fpga image that the bootloader will use during progamming
* qt_helloworldsw.bin -- the initial m4 application
* qt_ssi_ai_app_imu.bin -- application binaries to stream onboard accel sensor data to Host (running SensiML Data Capture Lab application)
* qt_ssi_ai_app_audio32.bin -- application binaries to stream onboard microphone data (PCM) to Host (running SensiML Data Capture Lab application)

This repo also contains a utility program, qf_loadflash.bin, that can be loaded by J-LINK and used to program the Thing+.

# Recovering a 'bricked' Thing+
Use J-LINK to load and run qt_loadflash.bin and then use the TinyFPGA-programmer-application to restore the factory programming:
```sh
python3 TinyFPGA-Programmer-Application/tinyfpga-programmer-gui --port COMXXX --mfgpkg qt-initial-binaries
```
