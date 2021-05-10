# qf-initial-bins
Binary files used to initialize a QuickFeather board

This repo contains the three binaries that are used to initialize a QuickFeather board during manufacturing:
* qf_bootloader.bin -- the bootloader program
* qf_bootfpga.bin -- the fpga image that the bootloader will use during progamming
* qf_helloworldsw.bin -- the initial m4 application

This repo also contains a utility program, qf_loadflash.bin, that can be loaded by J-LINK and used to program the QuickFeather.

# Recovering a 'bricked' QuickFeather
Use J-LINK to load and run qf_loadflash.bin and then use the TinyFPGA-programmer-application to restore the factory programming:
```sh
python3 TinyFPGA-Programmer-Application/tinyfpga-programmer-gui --port COMXXX --mfgpkg quickfeather-initial-binaries
```
