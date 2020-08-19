quit -sim

vlib work

vlog -work work +define+GSIM top_post_synthesis.v
vlog -work work +define+GSIM cells_sim.v
vlog -work work +define+GSIM ram_sim.v
vlog -work work +define+GSIM ../../testbench/Serial_Rx_monitor.v
vlog -work work +define+GSIM ../../testbench/Serial_Tx_gen_bfm.v
vlog -work work +define+GSIM ../../testbench/SPI_S_AD7476A.v
vlog -work work +define+GSIM ../../testbench/Generic_tb_AL4S3B_FPGA_IP.v

#vsim +define+SIMULATION -t ps -novopt work.testbench_top -l testbench_top.log
vsim +define+SIMULATION -t ps -sdfmax /testbench_top/u_AL4S3B_FPGA_Top=top_post_synthesis.sdf -novopt work.testbench_top -l testbench_top.log

do wave_tmp.do
log -r *
run -all