`timescale 1ns/10ps
//`define ENAB_GPIO_INT
//`define ENAB_UART_16550_inst
module testbench_top;

// =======================================
//
//		SIGNAL / PARAMETER DEFINITIONS
//
// =======================================

parameter       ADDRWIDTH                   = 32            ;
parameter       DATAWIDTH                   = 32            ;

parameter       APERWIDTH                   = 17            ;
parameter       APERSIZE                    =  9            ;

parameter       FPGA_REG_BASE_ADDRESS	        = 32'h40020000; // Assumes 128K Byte Aperture
parameter       UART_BASE_ADDRESS    			= 32'h40021000; // Assumes 128K Byte Aperture
parameter       BASE_AHB_ADDRESS_DMA_REG    	= 32'h40030000;
parameter       BASE_AHB_ADDRESS_DMA_DPORT0REG  = 32'h40031000;
parameter       QL_RESERVED_BASE_ADDRESS    	= 32'h40032000;

parameter       AL4S3B_DEVICE_ID              = 32'h000ADC1F;   
parameter       AL4S3B_REV_LEVEL              = 32'h00000100;

parameter       ID_REG_ADR          	       =  10'h0         ;
parameter       REV_REG_ADR          	   	   =  10'h1         ;
parameter       FIFO_RST_REG_ADR          	   =  10'h2         ;
parameter       SENS_EN_REG          	       =  10'h3         ;
parameter       SENS_1_REG          	       =  10'h4         ;
parameter       SENS_2_REG          	       =  10'h5         ;
parameter       SENS_3_REG          	       =  10'h6         ;
parameter       SENS_4_REG          	       =  10'h7         ;
parameter       TIMER_REG          	           =  10'h8         ;
parameter       TIMER_EN          	           =  10'h9         ;

parameter       DMA_EN_REG_ADR                =  10'h0         ;
parameter       DMA_STS_REG_ADR               =  10'h1         ;
parameter       DMA_INTR_EN_REG_ADR           =  10'h2         ;
parameter       RESERVED_3                    =  10'hB         ;
parameter       DMA_DEF_REG_VALUE             =  32'hDAD_DEF_AC ; 
parameter       DMA_CH0_DATA_REG_ADR          =  10'h0;

parameter       AL4S3B_DEF_REG_VALUE        = 32'hFAB_DEF_AC; // Distinguish access to undefined area

parameter       DEFAULT_READ_VALUE          = 32'hBAD_FAB_AC;
parameter       DEFAULT_CNTR_WIDTH          =  3            ;
parameter       DEFAULT_CNTR_TIMEOUT        =  7            ;

parameter       STD_CLK_DLY                 =  2            ;

parameter       TEST_MSG_ARRAY_SIZE         = (64 * 8)      ;     // 64 ASCII character array size

reg 			rst_spi_slave;

reg         	conv;
reg         	MOSI;
wire        	RDbar;
wire        	BUSYbar ;

//	Define the reset signal
//
reg             sys_rst_N   ;

reg             enable_fab_reg_test_1;
reg             enable_fab_reg_test_2;
reg             enable_fab_reg_test_3;

integer 		pass_count_fab_tests;
integer 		fail_count_fab_tests;

integer			fail_count;				// Count the number of failing tests
integer			pass_count;				// Count the number of passing tests

`ifdef ENAB_UART_16550_inst	
reg             SIN         ;
wire            SOUT        ;
wire            Null_Modem_Tx_to_Rx;
wire            Null_Modem_Rx_to_Tx;
`endif

`ifdef GSIM
	defparam testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.T_CYCLE_CLK_SYS_CLK0 = 100;
`else
    defparam testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.T_CYCLE_CLK_SYS_CLK0 = 100;
`endif

//  Application Specific signals
//
// 	Note:   These signals may be controlled by an external pull-up/down,
//          switches, jumpers, or GPIO port of a controller.
//
//          These should be controlled via reg to allow emulation of this
//          control.
//

// =======================================
//
//		INITIALIZE INTERFACE SIGNALS
//
// =======================================
//
//	Note:   These are signals that are normally controlled via a pull-up/down,
//          switches, jumpers, or GPIO of a controller.
//
//          The following always blocks sets the initial state of the signal.
//

initial
begin

    // Initialize the Serial UART Bus monitor

    sys_rst_N        <=  1'b0;    // Start the simulation with the device reset
	#100;
    sys_rst_N        <=  1'b1;    // Start the simulation with the device reset

end


// =======================================
//
//		MODULE INSTANTIATIONS
//
// =======================================
// Fabric top level
//
// Note: This represents the board level connection to the fabric IP.
//
top      u_AL4S3B_FPGA_Top 
	                           (
`ifdef ENAB_GPIO_INT							   
    .GPIO_PIN                  	( GPIO_PIN ),
`endif	
`ifdef ENAB_UART_16550_inst	
	.SIN_i                     	( Null_Modem_Tx_to_Rx ),
	.SOUT_o                    	( Null_Modem_Rx_to_Tx ),
`endif
    .SDATA_i					(	MISO		), 
    .SCLK_o						(	SCK			), 
 	.CSn_o        			    (	RDbar		)
	);
	
SPI_s_LTC1857 u_SPI_s_LTC1857 
   (.rst_i			(rst_spi_slave),
	.sck_i			(SCK),
	.mosi_i			(MOSI),
	.miso_o			(MISO),
	.conv_i         (conv),
	.RDbar_i        (RDbar),
	.BUSYbar_o      (BUSYbar)
);	

//`define Modelsim

// Save waveforms for analysis
//
/*
initial
begin
    $timeformat(-9,1,"ns",20);
//    $dumpfile("AL4S3B_Fabric_sim.vcd");
//    $dumpvars;

`ifdef Modelsim
	$wlfdumpvars;
`endif

end
*/
`ifdef ENAB_UART_16550_inst	
// Intantiate the Tx BFM
//
Serial_Tx_gen_bfm             #(

    .ENABLE_REG_WR_DEBUG_MSG   ( 1'b0                                    ),
    .ENABLE_REG_RD_DEBUG_MSG   ( 1'b0                                    ),

    .SERIAL_DATA_5_BITS        ( SERIAL_DATA_5_BITS                      ),
    .SERIAL_DATA_6_BITS        ( SERIAL_DATA_6_BITS                      ),
    .SERIAL_DATA_7_BITS        ( SERIAL_DATA_7_BITS                      ),
    .SERIAL_DATA_8_BITS        ( SERIAL_DATA_8_BITS                      ),

    .SERIAL_PARITY_NONE        ( SERIAL_PARITY_NONE                      ),
    .SERIAL_PARITY_ODD         ( SERIAL_PARITY_ODD                       ),
    .SERIAL_PARITY_EVEN        ( SERIAL_PARITY_EVEN                      ),
    .SERIAL_PARITY_FORCE_1     ( SERIAL_PARITY_FORCE_1                   ),
    .SERIAL_PARITY_FORCE_0     ( SERIAL_PARITY_FORCE_0                   ),

    .SERIAL_STOP_1_BIT         ( SERIAL_STOP_1_BIT                       ),
    .SERIAL_STOP_1P5_BIT       ( SERIAL_STOP_1P5_BIT                     ),
    .SERIAL_STOP_2_BIT         ( SERIAL_STOP_2_BIT                       ),

    .SERIAL_BAUD_RATE_110      ( SERIAL_BAUD_RATE_110                    ),
    .SERIAL_BAUD_RATE_300      ( SERIAL_BAUD_RATE_300                    ),
    .SERIAL_BAUD_RATE_600      ( SERIAL_BAUD_RATE_600                    ),
    .SERIAL_BAUD_RATE_1200     ( SERIAL_BAUD_RATE_1200                   ),
    .SERIAL_BAUD_RATE_2400     ( SERIAL_BAUD_RATE_2400                   ),
    .SERIAL_BAUD_RATE_4800     ( SERIAL_BAUD_RATE_4800                   ),
    .SERIAL_BAUD_RATE_9600     ( SERIAL_BAUD_RATE_9600                   ),
    .SERIAL_BAUD_RATE_14400    ( SERIAL_BAUD_RATE_14400                  ),
    .SERIAL_BAUD_RATE_19200    ( SERIAL_BAUD_RATE_19200                  ),
    .SERIAL_BAUD_RATE_38400    ( SERIAL_BAUD_RATE_38400                  ),
    .SERIAL_BAUD_RATE_57600    ( SERIAL_BAUD_RATE_57600                  ),
    .SERIAL_BAUD_RATE_115200   ( SERIAL_BAUD_RATE_115200                 ),
    .SERIAL_BAUD_RATE_230400   ( SERIAL_BAUD_RATE_230400                 ),
    .SERIAL_BAUD_RATE_921600   ( SERIAL_BAUD_RATE_921600                 )

    )

    u_Serial_Tx_gen_bfm        (

    .Tx                        ( Null_Modem_Tx_to_Rx                     ),
    .RTSn                      (                                         ),
    .CTSn                      (                                         )

    );
			
// Intantiate the Rx BFM
//
Serial_Rx_monitor             #(

    .ENABLE_DEBUG_MSGS         ( 1'b1                                    ),
    .ENABLE_DEBUG_ERROR_MSGS   ( 1'b1                                    ),

    .SERIAL_DATA_5_BITS        ( SERIAL_DATA_5_BITS                      ),
    .SERIAL_DATA_6_BITS        ( SERIAL_DATA_6_BITS                      ),
    .SERIAL_DATA_7_BITS        ( SERIAL_DATA_7_BITS                      ),
    .SERIAL_DATA_8_BITS        ( SERIAL_DATA_8_BITS                      ),

    .SERIAL_PARITY_NONE        ( SERIAL_PARITY_NONE                      ),
    .SERIAL_PARITY_ODD         ( SERIAL_PARITY_ODD                       ),
    .SERIAL_PARITY_EVEN        ( SERIAL_PARITY_EVEN                      ),
    .SERIAL_PARITY_FORCE_1     ( SERIAL_PARITY_FORCE_1                   ),
    .SERIAL_PARITY_FORCE_0     ( SERIAL_PARITY_FORCE_0                   ),

    .SERIAL_STOP_1_BIT         ( SERIAL_STOP_1_BIT                       ),
    .SERIAL_STOP_1P5_BIT       ( SERIAL_STOP_1P5_BIT                     ),
    .SERIAL_STOP_2_BIT         ( SERIAL_STOP_2_BIT                       ),

    .SERIAL_BAUD_RATE_110      ( SERIAL_BAUD_RATE_110                    ),
    .SERIAL_BAUD_RATE_300      ( SERIAL_BAUD_RATE_300                    ),
    .SERIAL_BAUD_RATE_600      ( SERIAL_BAUD_RATE_600                    ),
    .SERIAL_BAUD_RATE_1200     ( SERIAL_BAUD_RATE_1200                   ),
    .SERIAL_BAUD_RATE_2400     ( SERIAL_BAUD_RATE_2400                   ),
    .SERIAL_BAUD_RATE_4800     ( SERIAL_BAUD_RATE_4800                   ),
    .SERIAL_BAUD_RATE_9600     ( SERIAL_BAUD_RATE_9600                   ),
    .SERIAL_BAUD_RATE_14400    ( SERIAL_BAUD_RATE_14400                  ),
    .SERIAL_BAUD_RATE_19200    ( SERIAL_BAUD_RATE_19200                  ),
    .SERIAL_BAUD_RATE_38400    ( SERIAL_BAUD_RATE_38400                  ),
    .SERIAL_BAUD_RATE_57600    ( SERIAL_BAUD_RATE_57600                  ),
    .SERIAL_BAUD_RATE_115200   ( SERIAL_BAUD_RATE_115200                 ),
    .SERIAL_BAUD_RATE_230400   ( SERIAL_BAUD_RATE_230400                 ),
    .SERIAL_BAUD_RATE_921600   ( SERIAL_BAUD_RATE_921600                 )

    )

    u_Serial_Rx_monitor                (

    .Rx                                ( Null_Modem_Rx_to_Tx               ),
    .RTSn                              (                                   ),
    .CTSn                              (                                   ),

    .Serial_Baud_Rate_parameter        ( Serial_Baud_Rate_parameter        ),
    .Serial_Data_Bits_parameter        ( Serial_Data_Bits_parameter        ),
    .Serial_Parity_Bit_parameter       ( Serial_Parity_Bit_parameter       ),
    .Serial_Stop_Bit_parameter         ( Serial_Stop_Bit_parameter         ),

    .Rx_Baud_16x_Clk                   ( Rx_Baud_16x_Clk                   ),
    .Rx_Capture_Trigger                ( Rx_Capture_Trigger                ),

    .Rx_Holding_Reg_Stop_Bit           ( Rx_Holding_Reg_Stop_Bit           ),
    .Rx_Holding_Reg_Parity_Bit         ( Rx_Holding_Reg_Parity_Bit         ),
    .Rx_Holding_Reg_Data_Bit           ( Rx_Holding_Reg_Data_Bit           ),
    .Rx_Holding_Reg_Start_Bit          ( Rx_Holding_Reg_Start_Bit          ),
    .Rx_Holding_Reg_Parity_Error_Flag  ( Rx_Holding_Reg_Parity_Error_Flag  ),
    .Rx_Holding_Reg_Framing_Error_Flag ( Rx_Holding_Reg_Framing_Error_Flag ),
    .Rx_Holding_Reg_Break_Flag         ( Rx_Holding_Reg_Break_Flag         ),
    .Rx_Holding_Reg_False_Start_Flag   ( Rx_Holding_Reg_False_Start_Flag   )

    );
`endif

task testCases_AL4S3B_Fabric_IP_ADC_LTC_testRoutines;

input  [31:0] TEST_ENABLES;
output [31:0] TOTAL_FAIL_COUNT;            // Count the number of failing tests
output [31:0] TOTAL_PASS_COUNT;            // Count the number of passing tests

integer initial_mem_contents_file;
integer std_mem_addrs_file;
integer final_mem_contents_file;

begin

	// Define a call to each test
	//
											
	if (TEST_ENABLES[0])
        Generic_AL4S3B_Fabric_IP_fabreg_Test ;
		
	if (TEST_ENABLES[1])
        Generic_AL4S3B_Fabric_IP_sens4_run_dma_read_test ;
		
	TOTAL_FAIL_COUNT = fail_count;
	TOTAL_PASS_COUNT = pass_count;

end       
endtask


task Generic_AL4S3B_Fabric_IP_fabreg_Test;
reg      [DATAWIDTH-1:0]  read_data;
reg	     [DATAWIDTH-1:0]  expected_data;

reg	     [ADDRWIDTH-1:0]  target_address;
reg	     [DATAWIDTH-1:0]  target_data;

reg                [7:0]  write_data_1;
reg                [7:0]  write_data_2;

reg                [2:0]  ahb_xfr_size;
reg                [9:0]  transfer_size;

reg      [DATAWIDTH-1:0]  target_read_data;
reg      [DATAWIDTH-1:0]  target_ref_data;

integer 				 d_buff_index;

integer        i, j,k,int_loop_shift;
integer        sens_prg_index;


reg 					  disable_read_chk;
reg 					  read_only_test;

begin
     read_only_test = 0;
    // Select the AHB transfer size of 32-Bits
	//
	//wait (testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.A2F_HRESET == 0);
	
	
    ahb_xfr_size = 3'h2;

	
	$display("-------------------------------------------------------------------------------------------------");
	$display("-------------------------------------------------------------------------------------------------");
	$display("Running ADC SPI - DMA Register Write/Read Tests ");
	$display("-------------------------------------------------------------------------------------------------");
	$display("-------------------------------------------------------------------------------------------------");	
	
	
    sens_prg_index = 0;
	//$stop();

    // Wait for a few clocks
	//
	for (i = 0; i < 4; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);
`endif
	ahb_xfr_size = 3'h2;
	target_data = 32'hADC1F;
    expected_data = target_data;
	disable_read_chk   = 0;	
	
	for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK) #STD_CLK_DLY;
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;
`endif
		
	target_address = 0;	
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), read_data);
`else
    testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), read_data);
`endif
	
	if (~disable_read_chk)
	begin
		for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK); 
`endif         
			
			if (read_data !== expected_data)
            begin
                $display("[Error] FPGA_Reg_Test_1: ID REG Register Address=0x%x , read=0x%x , expected=0x%x at time %0t", 
                                                                                                     (FPGA_REG_BASE_ADDRESS + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                expected_data, 
                                                                                                                                    $realtime);
                fail_count = fail_count + 1;
	            $stop();
            end	
            else
            begin
                $display("[Pass]  FPGA_Reg_Test_1: ID REG  Register Address=0x%x , read=0x%x , expected=0x%x at time %0t",  
                                                                                                     (FPGA_REG_BASE_ADDRESS + target_address[7:0]),
                                                                                                                                    read_data,
                                                                                                                                expected_data,
                                                                                                                                    $realtime);
                pass_count = pass_count + 1;
	        end
			
	end
	else
	begin
            $display("FPGA_Reg_Test_1: ID REG  Register Address=0x%x , read=0x%x at time %0t", 
                                                                                                     (FPGA_REG_BASE_ADDRESS + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                    $realtime);
	end
	
	target_address = target_address + 4;
	for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK) #STD_CLK_DLY;
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;
`endif
	
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), read_data);
`else		
    testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), read_data);
`endif	
	
	target_data = 32'h1;	
	expected_data = 32'h0;
    disable_read_chk   = 0;	
	target_address = target_address + 4;
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`else
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`endif

`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), read_data);
`else	
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), read_data);
`endif
	if (~disable_read_chk)
	begin
		for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK); 
`endif           
			
			if (read_data !== expected_data)
            begin
                $display("[Error] FPGA_Reg_Test_1: ID REG Register Address=0x%x , read=0x%x , expected=0x%x at time %0t", 
                                                                                                     (FPGA_REG_BASE_ADDRESS + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                expected_data, 
                                                                                                                                    $realtime);
                fail_count = fail_count + 1;
	            $stop();
            end	
            else
            begin
                $display("[Pass]  FPGA_Reg_Test_1: ID REG  Register Address=0x%x , read=0x%x , expected=0x%x at time %0t",  
                                                                                                     (FPGA_REG_BASE_ADDRESS + target_address[7:0]),
                                                                                                                                    read_data,
                                                                                                                                expected_data,
                                                                                                                                    $realtime);
                pass_count = pass_count + 1;
	        end
			
	end
	else
	begin
            $display("FPGA_Reg_Test_1: ID REG  Register Address=0x%x , read=0x%x at time %0t", 
                                                                                                     (FPGA_REG_BASE_ADDRESS + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                    $realtime);
	end
	
    // Wait for a few clocks
	//
	for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK) #STD_CLK_DLY;
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;
`endif

end
endtask

task Generic_AL4S3B_Fabric_IP_sens4_run_dma_read_test;
reg      [DATAWIDTH-1:0]  read_data;
reg	     [DATAWIDTH-1:0]  expected_data;

reg	     [ADDRWIDTH-1:0]  target_address;
reg	     [DATAWIDTH-1:0]  target_data;

reg                [2:0]  ahb_xfr_size;
reg                [9:0]  transfer_size;

reg      [DATAWIDTH-1:0]  target_read_data;
reg      [DATAWIDTH-1:0]  target_ref_data;

integer 				 d_buff_index;

integer        			i, j,k;
integer        			adr_index,loop_idx;
integer        			dma_idx;
integer        			top_dma_idx;

integer                sens_dmarddata_f_ptr;

reg 					disable_read_chk;
reg 					read_only_test;
reg      [31:0]         mem_sens_data [0:3];
reg      [7:0]          sens_value;
reg 	 [1:0]   		mem_index;


begin
    read_only_test 	= 0;
    ahb_xfr_size = 3'h2;
    sens_value = 8'hF1;
    mem_sens_data[0] = 32'h0aa60aa5;
	mem_sens_data[1] = 32'h0aa80aa7;
	mem_sens_data[2] = 32'h0aaa0aa9;
	mem_sens_data[3] = 32'h0aac0aab;
	mem_index 	= 0;
	
	$display("-------------------------------------------------------------------------------------------------");
	$display("-------------------------------------------------------------------------------------------------");
	$display("Running Acc Neuron - DMA ch0 read Tests ");
	$display("-------------------------------------------------------------------------------------------------");
	$display("-------------------------------------------------------------------------------------------------");	
	
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_active_sig[0] <= 1'b0;
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_done_sig[0] <= 1'b0;	
`else
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_active_sig[0] <= 1'b0;
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_done_sig[0] <= 1'b0;	
`endif

    // Wait for a few clocks
	//
	for (i = 0; i < 4; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);
`endif

	target_data = 32'h1;	
	expected_data = 32'h1;
    disable_read_chk   = 0;	
	target_address = 8'h8;
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), target_data);
`else
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), target_data);
`endif

`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);
`else	
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);
`endif	
	if (~disable_read_chk)
	begin
		for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);   
`endif         
			
			if (read_data !== expected_data)
            begin
                $display("[Error] DMA_INT_Reg: DMA INT Enable Register Address=0x%x , read=0x%x , expected=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                expected_data, 
                                                                                                                                    $realtime);
                fail_count = fail_count + 1;
	            $stop();
            end	
            else
            begin
                $display("[Pass]  DMA_INT_Reg: DMA INT Enable Register Address=0x%x , read=0x%x , expected=0x%x at time %0t",  
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]),
                                                                                                                                    read_data,
                                                                                                                                expected_data,
                                                                                                                                    $realtime);
                pass_count = pass_count + 1;
	        end
			
	end
	else
	begin
            $display("DMA_INT_Reg: DMA INT Enable Register Address=0x%x , read=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                    $realtime);
	end

	target_data = 32'h1;	
	target_address = 8'hC;
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`else
	testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`endif
	// Wait for a few clocks
	//
	for (i = 0; i < 14; i = i + 1)
`ifdef GSIM
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
`else
		@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;	
`endif

    for (top_dma_idx = 0; top_dma_idx < 8; top_dma_idx = top_dma_idx + 1)	
	begin		
			target_data = 32'h1;	
			target_address = 8'h0;
		`ifdef GSIM
			testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), target_data);
		`else
			testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), target_data);
		`endif

			$display("Waiting SDMA Read Dreq");		
		`ifdef GSIM
			wait (testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_req_i[0] == 1'b1 );
		`else 
			wait (testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_req_i[0] == 1'b1 );
		`endif 

            for (i = 0; i < 2; i = i + 1)
			`ifdef GSIM
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
			`else
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;
			`endif

		`ifdef GSIM
			testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_active_sig[0] <= 1'b1;
		`else
			testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_active_sig[0] <= 1'b1;
        `endif
					
			for (i = 0; i < 14; i = i + 1)
			`ifdef GSIM
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK) #STD_CLK_DLY;
			`else
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;	
			`endif
			
			mem_index 	= 0;
			for (dma_idx = 0; dma_idx < 256; dma_idx = dma_idx + 1)	
			begin
                expected_data = mem_sens_data[mem_index];
			`ifdef GSIM
				testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_DPORT0REG), read_data);
			`else
				testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_DPORT0REG), read_data);
			`endif
				if (read_data !== expected_data)
				begin
					$display("[Error] DMA_INT_Reg: DMA INT Enable Register Address=0x%x , read=0x%x , expected=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                expected_data, 
                                                                                                                                    $realtime);
					fail_count = fail_count + 1;
					$stop();
				end	
				else
				begin
					$display("-----------------------------------------\n Sensor Received Data=0x%h \n-----------------------------------------\n",read_data); 
                end
				mem_index = mem_index + 1;
            end	
             		
				//Disable the DMA Active signal
			`ifdef GSIM
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.A2F_HCLK);
				testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_active_sig[0] <= 1'b0;
				repeat (1) @(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.A2F_HCLK);
			`else
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.A2F_HCLK);
				testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_active_sig[0] <= 1'b0;
				repeat (1) @(posedge testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.A2F_HCLK);
			`endif

			`ifdef GSIM
				testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_done_sig[0] <= 1'b1;
				repeat (1) @(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.A2F_HCLK);
				testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_done_sig[0] <= 1'b0;
			`else				
				testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_done_sig[0] <= 1'b1;
				repeat (1) @(posedge testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.A2F_HCLK);
				testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.sdma_bfm_inst0.sdma_done_sig[0] <= 1'b0;
			`endif

			for (i = 0; i < 14; i = i + 1)
			`ifdef GSIM
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK) #STD_CLK_DLY;
			`else
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK) #STD_CLK_DLY;
			`endif
		`ifdef GSIM
			wait (testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.FB_msg_out[0] == 1'b1);//DMA interrupt
		`else
			wait (testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.FB_msg_out[0] == 1'b1);//DMA interrupt
		`endif

			for (i = 0; i < 2; i = i + 1)
		`ifdef GSIM
			@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
		`else
			@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);
		`endif

            target_address = 8'h0;
		`ifdef GSIM
			testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);	
		`else
			testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);	
		`endif	
			$display("DMA_EN_Reg: DMA Enable Register Address=0x%x , read=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                    $realtime);
			for (i = 0; i < 2; i = i + 1)
		`ifdef GSIM
			@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
		`else
			@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);
		`endif

		    expected_data = 32'h01;
			disable_read_chk   = 0;	
			target_address = 8'h4;
		`ifdef GSIM
			testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);
		`else
			testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);
		`endif
			if (~disable_read_chk)
			begin
				for (i = 0; i < 2; i = i + 1)
			`ifdef GSIM
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
			`else
				@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);   
			`endif
			
				if (read_data !== expected_data)
				begin
					$display("[Error] DMA_INT_Reg: DMA Done INT  Register Address=0x%x , read=0x%x , expected=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                expected_data, 
                                                                                                                                    $realtime);
					fail_count = fail_count + 1;
					$stop();
				end	
				else
				begin
					$display("[Pass]  DMA_INT_Reg: DMA Done INT  Register Address=0x%x , read=0x%x , expected=0x%x at time %0t",  
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]),
                                                                                                                                    read_data,
                                                                                                                                expected_data,
                                                                                                                                    $realtime);
					pass_count = pass_count + 1;
				end
			end
			else
			begin
				$display("DMA_INT_Reg: DMA Done INT  Register Address=0x%x , read=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                    $realtime);
			end   
			target_data = 32'h0;
		`ifdef GSIM
			testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), target_data);
		`else	
			testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), target_data);
		`endif
			for (i = 0; i < 4; i = i + 1)
		`ifdef GSIM
			@(posedge testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.WB_CLK);
            testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);	
		`else
			@(posedge testbench_top.u_AL4S3B_FPGA_Top.WB_CLK);
            testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_read_word_al4s3b_fabric((BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), read_data);	
		`endif
				$display("DMA_INT_Reg: DMA Done INT  Register Address=0x%x , read=0x%x at time %0t", 
                                                                                                     (BASE_AHB_ADDRESS_DMA_REG + target_address[7:0]), 
						                                                                                                            read_data, 
                                                                                                                                    $realtime);
	end	
    target_data = 32'h0;	
    target_address = 8'hC;
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`else
    testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`endif

    target_data = 32'h1;	
    target_address = 8'h8;
`ifdef GSIM
	testbench_top.u_AL4S3B_FPGA_Top.ASSP_u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`else
    testbench_top.u_AL4S3B_FPGA_Top.u_qlal4s3b_cell_macro.u_ASSP_bfm_inst.u_ahb_gen_bfm.ahb_write_word_al4s3b_fabric((FPGA_REG_BASE_ADDRESS + target_address[7:0]), target_data);
`endif	    
end
endtask

initial
begin
	 MOSI = 0;
	 conv = 0;
	 pass_count	= 0;
     fail_count = 0;
     rst_spi_slave = 0;
	 #10;
	 rst_spi_slave = 1;
	 #100;
	 rst_spi_slave = 0;
end

initial
begin
    pass_count									= 0;
    fail_count									= 0;

    pass_count_fab_tests                        = 0;

    fail_count_fab_tests                        = 0;

    enable_fab_reg_test_1                       = 0;
    enable_fab_reg_test_2                       = 0;
    enable_fab_reg_test_3                       = 0;
	
	#100
	enable_fab_reg_test_1                 = 1'b1;
	enable_fab_reg_test_2                 = 1'b1;
	enable_fab_reg_test_3                 = 1'b0;
	
	testCases_AL4S3B_Fabric_IP_ADC_LTC_testRoutines(    
        {28'h0,
		 enable_fab_reg_test_3,
		 enable_fab_reg_test_2,
		 enable_fab_reg_test_1},   									// TEST_ENABLES[0]                                         
         fail_count_fab_tests, 
         pass_count_fab_tests);		 

		pass_count = pass_count_fab_tests;
		fail_count = fail_count_fab_tests;

		// Display test execution statistics
		$display(" Passing Tests: %0d \n", pass_count);
		$display(" Failing Tests: %0d \n", fail_count);		
	
	#5000
	$finish;

end

endmodule // testbench_top
