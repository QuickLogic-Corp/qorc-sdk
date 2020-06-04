BINS = qf_helloworldsw/GCC_Project/output/bin/qf_helloworldsw.bin\
	qf_helloworldhw/GCC_Project/output/bin/qf_helloworldhw.bin\
	qf_advancedfpga/GCC_Project/output/bin/qf_advancedfpga.bin\
	qf_bootloader/GCC_Project/output/bin/qf_bootloader.bin

MFGBINS = qf_bootloader/GCC_Project/output/bin/qf_bootloader.bin\
	  qf_helloworldsw/GCC_Project/output/bin/qf_helloworldsw.bin
all:	$(BINS)

clean:
	cd qf_helloworldsw/GCC_Project/ && make clean
	cd qf_helloworldhw/GCC_Project/ && make clean
	cd qf_advancedfpga/GCC_Project/ && make clean
	cd qf_bootloader/GCC_Project/ && make clean



qf_mfgpkg:	$(MFGBINS)
	mkdir -p ../../qf_mfgpkg
	cp $(MFGBINS) ../../qf_mfgpkg


qf_helloworldsw/GCC_Project/output/bin/qf_helloworldsw.bin:
	cd qf_helloworldsw/GCC_Project/ && make

qf_helloworldhw/GCC_Project/output/bin/qf_helloworldhw.bin:
	cd qf_helloworldhw/GCC_Project/ && make

qf_advancedfpga/GCC_Project/output/bin/qf_advancedfpga.bin:
	cd qf_advancedfpga/GCC_Project/ && make

qf_bootloader/GCC_Project/output/bin/qf_bootloader.bin:
	cd qf_bootloader/GCC_Project/ && make
