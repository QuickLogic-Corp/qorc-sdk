quit -sim

vlib work

vlog -work work ../../rtl/FIFO_16K_BLK.v
vlog -work work ../../rtl/af512x32_512x32.v
vlog -work work ../../rtl/f512x18_512x18_f512x9_512x9.v
vlog -work work ../../rtl/UART_16550_Tx_Rx_FIFOs.v
vlog -work work ../../rtl/UART_16550_Interrupt_Control.v
vlog -work work ../../rtl/UART_16550_Registers.v
vlog -work work ../../rtl/UART_16550_Rx_Logic.v
vlog -work work ../../rtl/UART_16550_Tx_Logic.v
vlog -work work ../../rtl/UART_16550.v
vlog -work work ../../rtl/Dma_Ctrl.v
vlog -work work ../../rtl/Fsm_Top.v
vlog -work work ../../rtl/Serializer_Deserializer_Test.v
vlog -work work ../../rtl/Serializer_Deserializer.v
vlog -work work ../../rtl/AL4S3B_FPGA_QL_Reserved.v
vlog -work work ../../rtl/AL4S3B_FPGA_Registers.v
vlog -work work ../../rtl/AL4S3B_FPGA_IP.v
vlog -work work ../../rtl/AL4S3B_FPGA_Top.v
vlog -work work ../../testbench/cells_sim.v
vlog -work work ../../testbench/Serial_Rx_monitor.v
vlog -work work ../../testbench/Serial_Tx_gen_bfm.v
vlog -work work ../../testbench/SPI_S_AD7476A.v
vlog -work work ../../testbench/Generic_tb_AL4S3B_Fabric_IP.v

vsim +define+SIMULATION -t ps -novopt work.testbench_top -l testbench_top.log

do wave.do
log -r *
run -all