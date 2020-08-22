onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/SCLK_o
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/SDATA_i
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/SIN_i
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/SOUT_o
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/CSn_o
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WB_CLK
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WB_CLKS
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WB_RST
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_ACK
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_ADR
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_BYTE_STB
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_CYC
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_RD
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_RD_DAT
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_STB
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_WE
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/WBs_WR_DAT
add wave -noupdate /testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/Sys_Clk0
add wave -noupdate {/testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/FB_msg_out[0]}
add wave -noupdate {/testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/SDMA_Req[0]}
add wave -noupdate {/testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/SDMA_Done[0]}
add wave -noupdate {/testbench_top/u_AL4S3B_FPGA_Top/ASSP_u_qlal4s3b_cell_macro/SDMA_Active[0]}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ps} 0}
quietly wave cursor active 0
configure wave -namecolwidth 150
configure wave -valuecolwidth 74
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {24864 ps}
