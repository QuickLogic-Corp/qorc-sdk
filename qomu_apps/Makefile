BINS = qomu_helloworldsw/GCC_Project/output/bin/qomu_helloworldsw.bin\
	qomu_bootloader/GCC_Project/output/bin/qomu_bootloader.bin\
	qomu_loadflash/GCC_Project/output/bin/qomu_loadflash.bin

MFGBINS = qomu_bootloader/GCC_Project/output/bin/qomu_bootloader.bin\
		qomu_loadflash/GCC_Project/output/bin/qomu_loadflash.bin\
		qomu_helloworldsw/GCC_Project/output/bin/qomu_helloworldsw.bin
all:	$(BINS)

clean:
	cd qomu_helloworldsw/GCC_Project/ && make clean
	cd qomu_bootloader/GCC_Project/ && make clean
	cd qomu_loadflash/GCC_Project/ && make clean



qomu_mfgpkg:	$(MFGBINS)
	mkdir -p qomu-initial-binaries
	cp $(MFGBINS) qomu-initial-binaries
	cp ../s3-gateware/usb2serial/usb2serial.bin qomu-initial-binaries/qomu_bootfpga.bin

qomu_helloworldsw/GCC_Project/output/bin/qomu_helloworldsw.bin:
	cd qomu_helloworldsw/GCC_Project/ && make

qomu_bootloader/GCC_Project/output/bin/qomu_bootloader.bin:
	cd qomu_bootloader/GCC_Project/ && make

qomu_loadflash/GCC_Project/output/bin/qomu_loadflash.bin:
	cd qomu_loadflash/GCC_Project/ && make