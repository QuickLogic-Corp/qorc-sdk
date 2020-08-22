onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_WR_DAT
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_WE
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_STB
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_RD_DAT
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_RD
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_CYC
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_BYTE_STB
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_ADR
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WBs_ACK
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WB_RST_FPGA
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WB_RST
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/WB_CLK
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/SDMA0_INT
add wave -noupdate -color Gold /testbench_top/u_AL4S3B_Fabric_Top/CSn_o
add wave -noupdate -color Gold /testbench_top/u_AL4S3B_Fabric_Top/SCLK_o
add wave -noupdate -color Gold /testbench_top/u_AL4S3B_Fabric_Top/SDATA_i
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/u_af512x32_512x32/Push_Clk
add wave -noupdate -color Gold /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/u_af512x32_512x32/PUSH
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/u_af512x32_512x32/POP
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/u_af512x32_512x32/DOUT
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/u_af512x32_512x32/DIN
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/rx_fifo_cnt
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Serializer_Deserializer/Sensor_RD_Push_o
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Serializer_Deserializer/Sensor_RD_Data_o
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Serializer_Deserializer/read_fifo_receive_data_l
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Serializer_Deserializer/read_fifo_receive_data
add wave -noupdate /testbench_top/u_SPI_s_LTC1857/data_transmit
add wave -noupdate /testbench_top/u_SPI_s_LTC1857/mem_index
add wave -noupdate /testbench_top/u_SPI_s_LTC1857/mem_adc_sens_data
add wave -noupdate /testbench_top/u_SPI_s_LTC1857/s_data_state
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/dma_req
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/trig_i
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_AL4S3B_FPGA_Registers/DMA0_EN
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/dreq_reset
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/dma_active_i_2ff
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/dma_active_i_1ff
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/DMA_Active_i
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/DMA_Active_i
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_AL4S3B_FPGA_IP/u_Dma_Ctrl/ASSP_DMA_Done_i
add wave -noupdate /testbench_top/u_AL4S3B_Fabric_Top/u_qlal4s3b_cell_macro/Sys_Clk0
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {112500000 ps} 0} {{Cursor 2} {112300000 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 201
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
WaveRestoreZoom {112088174 ps} {113210251 ps}
