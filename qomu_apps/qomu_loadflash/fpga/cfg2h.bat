REM Convert bitstream into .h file
perl convert_fbcfg_to_cpuwrites.pl -h ShiftRegPattern.qlal4s3b.AL4S3B_FPGA_top.Qomu > fpga.h
perl convert_fbcfg_to_cpuwrites.pl -h ShiftRegPattern.qlal4s3b.AL4S3B_FPGA_top.Qomu > fpga.c
cmd -k