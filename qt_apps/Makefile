BINS = qt_helloworldsw/GCC_Project/output/bin/qt_helloworldsw.bin\
	qt_bootloader/GCC_Project/output/bin/qt_bootloader.bin\
	qt_loadflash/GCC_Project/output/bin/qt_loadflash.bin\
	qt_ssi_ai_app/GCC_Project/output/bin/qt_ssi_ai_app.bin

MFGBINS = qt_bootloader/GCC_Project/output/bin/qt_bootloader.bin\
		qt_loadflash/GCC_Project/output/bin/qt_loadflash.bin\
		qt_helloworldsw/GCC_Project/output/bin/qt_helloworldsw.bin
all:	$(BINS)

clean:
	cd qt_helloworldsw/GCC_Project/ && make clean
	cd qt_bootloader/GCC_Project/ && make clean
	cd qt_loadflash/GCC_Project/ && make clean
	cd qt_ssi_ai_app/GCC_Project/ && make clean



qt_mfgpkg:	$(MFGBINS)
	mkdir -p qt-initial-binaries
	cp $(MFGBINS) qt-initial-binaries
	cp ../s3-gateware/usb2serial/usb2serial.bin qt-initial-binaries/qt_bootfpga.bin

qt_helloworldsw/GCC_Project/output/bin/qt_helloworldsw.bin:
	cd qt_helloworldsw/GCC_Project/ && make

qt_bootloader/GCC_Project/output/bin/qt_bootloader.bin:
	cd qt_bootloader/GCC_Project/ && make

qt_loadflash/GCC_Project/output/bin/qt_loadflash.bin:
	cd qt_loadflash/GCC_Project/ && make

qt_ssi_ai_app/GCC_Project/output/bin/qt_ssi_ai_app.bin:
	cd qt_ssi_ai_app/GCC_Project/ && make