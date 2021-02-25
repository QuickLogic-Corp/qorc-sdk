REM
REM  Batch to generate usb2serial.bin
REM 
copy AL4S3B_FPGA_top_2020.05.19a_serial.chp usb2serial.chp
spdecl.exe -load usb2serial.chp -save_config_bits
perl convert_fbcfg_to_cpuwrites.pl -h ShiftRegPattern.qlal4s3b.usb2serial > usb2serial_bit.h
copy qlal4s3b_usb2serial.bin usb2serial.bin
copy usb2serial.bin qf_bootfpga.bin
del usb2serial.clk usb2serial.spd temp.txt spdecl.log qlal4s3b_usb2serial.jlink qlal4s3b_usb2serial.bin ShiftRegPattern.qlal4s3b.usb2serial ShiftPattern.qlal4s3b.usb2serial.csv
