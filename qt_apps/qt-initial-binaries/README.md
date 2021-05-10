# qt-initial-bins
Binary files used to initialize a Thing+ board

This repo contains the three binaries that are used to initialize a Thing+ board during manufacturing:
* qt_bootloader.bin -- the bootloader program
* qt_bootfpga.bin -- the fpga image that the bootloader will use during progamming
* qt_helloworldsw.bin -- the initial m4 application

This repo also contains a utility program, qf_loadflash.bin, that can be loaded by J-LINK and used to program the Thing+.

# Recovering a 'bricked' Thing+
Use J-LINK to load and run qt_loadflash.bin and then use the TinyFPGA-programmer-application to restore the factory programming:
```sh
python3 TinyFPGA-Programmer-Application/tinyfpga-programmer-gui --port COMXXX --mfgpkg qt-initial-binaries
```
