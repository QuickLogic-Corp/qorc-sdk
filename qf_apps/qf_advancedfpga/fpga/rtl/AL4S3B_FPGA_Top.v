// -----------------------------------------------------------------------------
// title          : AL4S3B Example FPGA IP Top Module
// project        : Tamar2 Device
// -----------------------------------------------------------------------------
// file           : AL4S3B_FPGA_top.v
// author         : SSG
// company        : QuickLogic Corp
// created        : 2016/02/01	
// last update    : 2016/02/01
// platform       : ArcticLink 4 S3B
// standard       : Verilog 2001
// -----------------------------------------------------------------------------
// description: The FPGA example IP design contains the essential logic for
//              interfacing the ASSP of the AL4S3B to IP located in the 
//              programmable FPGA.
// -----------------------------------------------------------------------------
// copyright (c) 2015
// -----------------------------------------------------------------------------
// revisions  :
// date            version        author              description
// 2016/02/01      1.0        Rakesh Moolacheri     Initial Release
//
// -----------------------------------------------------------------------------
// Comments: This solution is specifically for use with the QuickLogic
//           AL4S3B device. 
// -----------------------------------------------------------------------------
//

`timescale 1ns / 10ps

module AL4S3B_FPGA_top ( 
			led_r_o,
			led_b_o,
			led_g_o
			
            );


//------Port Parameters----------------
//

parameter       APERWIDTH                   = 17            ;
parameter       APERSIZE                    =  9            ;

parameter       FPGA_REG_BASE_ADDRESS     = 17'h00000     ; // Assumes 128K Byte FPGA Memory Aperture
parameter       QL_RESERVED_BASE_ADDRESS    = 17'h00800     ; // Assumes 128K Byte FPGA Memory Aperture

parameter       ADDRWIDTH_FAB_REG           =  7            ;
parameter       DATAWIDTH_FAB_REG           = 32            ;

parameter       FPGA_REG_ID_VALUE_ADR     =  7'h0; 
parameter       FPGA_REV_NUM_ADR          =  7'h1; 
parameter       FPGA_SCRATCH_REG_ADR      =  7'h2; 

parameter       AL4S3B_DEF_REG_VALUE        = 32'hFAB_DEF_AC; // Distinguish access to undefined area

parameter       DEFAULT_READ_VALUE          = 32'hBAD_FAB_AC;
parameter       DEFAULT_CNTR_WIDTH          =  3            ;
parameter       DEFAULT_CNTR_TIMEOUT        =  7            ;

parameter       ADDRWIDTH_QL_RESERVED       =  7            ;
parameter       DATAWIDTH_QL_RESERVED       = 32            ;

parameter       QL_RESERVED_CUST_PROD_ADR   =  7'h7E        ;  // <<-- Very Top of the FPGA's Memory Aperture
parameter       QL_RESERVED_REVISIONS_ADR   =  7'h7F        ;  // <<-- Very Top of the FPGA's Memory Aperture

parameter       QL_RESERVED_CUSTOMER_ID     =  8'h01        ;  // <<-- Update for each Customer
parameter       QL_RESERVED_PRODUCT_ID      =  8'h00        ;  // <<-- Update for each Customer Product
parameter       QL_RESERVED_MAJOR_REV       = 16'h0000      ;  // <<-- Update for each Major Revision (i.e. Rev 1,    Rev 2,    etc.)
parameter       QL_RESERVED_MINOR_REV       = 16'h0001      ;  // <<-- Update for each Minor Revision (i.e. Rev 1.01, Rev 1.02, etc.)

parameter       QL_RESERVED_DEF_REG_VALUE   = 32'hDEF_FAB_AC; // Distinguish access to undefined area


//------Port Signals-------------------

output 			led_r_o;
output 			led_b_o;
output 			led_g_o;

//
// None at this time
//

//------Internal Signals---------------
//

// FPGA Global Signals
//
wire            WB_CLK         ; // Selected FPGA Clock

wire            Sys_Clk0       ; // Selected FPGA Clock
wire            Sys_Clk0_Rst   ; // Selected FPGA Reset

wire            Sys_Clk1       ; // Selected FPGA Clock
wire            Sys_Clk1_Rst   ; // Selected FPGA Reset

// Wishbone Bus Signals
//
wire    [16:0]  WBs_ADR        ; // Wishbone Address Bus
wire            WBs_CYC        ; // Wishbone Client Cycle  Strobe (i.e. Chip Select)
wire     [3:0]  WBs_BYTE_STB   ; // Wishbone Byte   Enables
wire            WBs_WE         ; // Wishbone Write  Enable Strobe
wire            WBs_RD         ; // Wishbone Read   Enable Strobe
wire            WBs_STB        ; // Wishbone Transfer      Strobe
wire    [31:0]  WBs_RD_DAT     ; // Wishbone Read   Data Bus
wire    [31:0]  WBs_WR_DAT     ; // Wishbone Write  Data Bus
wire            WBs_ACK        ; // Wishbone Client Acknowledge
wire            WB_RST         ; // Wishbone FPGA Reset
wire            WB_RST_FPGA  ; // Wishbone FPGA Reset

// Misc
//
wire    [31:0]  Device_ID      ;

wire   			boot;
wire   			clk_48mhz;
wire   			reset;

wire   			usb_pu_cntrl_o;

wire            Interrupt_o;


//------Logic Operations---------------
//
//assign usb_pu_cntrl_o = 1'b1;
assign usb_pu_cntrl_o = 1'b0;

// Determine the FPGA reset
//
// Note: Reset the FPGA IP on either the AHB or clock domain reset signals.
//
gclkbuff u_gclkbuff_reset ( .A(Sys_Clk0_Rst | WB_RST) , .Z(WB_RST_FPGA) );
gclkbuff u_gclkbuff_clock ( .A(Sys_Clk0             ) , .Z(WB_CLK     ));

gclkbuff u_gclkbuff_reset1 ( .A(Sys_Clk1_Rst) , .Z(reset) );
gclkbuff u_gclkbuff_clock1  ( .A(Sys_Clk1   ) , .Z(clk_48mhz ) );


// Example FPGA Design
//
AL4S3B_FPGA_IP              #(
	
    .APERWIDTH                 ( APERWIDTH                   ),
    .APERSIZE                  ( APERSIZE                    ),

    .FPGA_REG_BASE_ADDRESS     ( FPGA_REG_BASE_ADDRESS     ),
    .QL_RESERVED_BASE_ADDRESS  ( QL_RESERVED_BASE_ADDRESS    ),

	.ADDRWIDTH_FAB_REG         ( ADDRWIDTH_FAB_REG           ),
    .DATAWIDTH_FAB_REG         ( DATAWIDTH_FAB_REG           ),
	
    .FPGA_REG_ID_VALUE_ADR     ( FPGA_REG_ID_VALUE_ADR       ),
    .FPGA_REV_NUM_ADR          ( FPGA_REV_NUM_ADR            ),     
    .FPGA_SCRATCH_REG_ADR      ( FPGA_SCRATCH_REG_ADR        ),

	.AL4S3B_DEF_REG_VALUE      ( AL4S3B_DEF_REG_VALUE        ),

    .DEFAULT_READ_VALUE        ( DEFAULT_READ_VALUE          ),
    .DEFAULT_CNTR_WIDTH        ( DEFAULT_CNTR_WIDTH          ),
    .DEFAULT_CNTR_TIMEOUT      ( DEFAULT_CNTR_TIMEOUT        ),

    .ADDRWIDTH_QL_RESERVED     ( ADDRWIDTH_QL_RESERVED       ),
    .DATAWIDTH_QL_RESERVED     ( DATAWIDTH_QL_RESERVED       ),

    .QL_RESERVED_CUST_PROD_ADR ( QL_RESERVED_CUST_PROD_ADR   ),
    .QL_RESERVED_REVISIONS_ADR ( QL_RESERVED_REVISIONS_ADR   ),

    .QL_RESERVED_CUSTOMER_ID   ( QL_RESERVED_CUSTOMER_ID     ),
    .QL_RESERVED_PRODUCT_ID    ( QL_RESERVED_PRODUCT_ID      ),
    .QL_RESERVED_MAJOR_REV     ( QL_RESERVED_MAJOR_REV       ),
    .QL_RESERVED_MINOR_REV     ( QL_RESERVED_MINOR_REV       ),

    .QL_RESERVED_DEF_REG_VALUE ( QL_RESERVED_DEF_REG_VALUE   )
                                                             )

     u_AL4S3B_FPGA_IP        (

	// AHB-To_FPGA Bridge I/F
    .WBs_ADR                   ( WBs_ADR                     ), // input  [16:0] | Address Bus                to   FPGA
    .WBs_CYC                   ( WBs_CYC                     ), // input         | Cycle Chip Select          to   FPGA
    .WBs_BYTE_STB              ( WBs_BYTE_STB                ), // input   [3:0] | Byte Select                to   FPGA
    .WBs_WE                    ( WBs_WE                      ), // input         | Write Enable               to   FPGA
    .WBs_RD                    ( WBs_RD                      ), // input         | Read  Enable               to   FPGA
    .WBs_STB                   ( WBs_STB                     ), // input         | Strobe Signal              to   FPGA
    .WBs_WR_DAT                ( WBs_WR_DAT                  ), // input  [31:0] | Write Data Bus             to   FPGA
    .WB_CLK                    ( WB_CLK                      ), // output        | FPGA Clock               from FPGA
    .WB_RST                    ( WB_RST_FPGA                 ), // input         | FPGA Reset               to   FPGA
    .WBs_RD_DAT                ( WBs_RD_DAT                  ), // output [31:0] | Read Data Bus              from FPGA
    .WBs_ACK                   ( WBs_ACK                     ), // output        | Transfer Cycle Acknowledge from FPGA

    //FPGA IP
	.usbp_io		            ( usbp_io            		),
	.usbn_io             		( usbn_io        			),
	
	.clk_48mhz_i             	( clk_48mhz        			),
	.reset_i             		( reset         			),
	
	.spi_cs_o             		( spi_cs_o         			),
	.spi_sck_o             		( spi_sck_o        			),
	.spi_mosi_o             	( spi_mosi_o       			),
	.spi_miso_i             	( spi_miso_i       			),
	
	.led_r_o             		( led_r_o       			),
	.led_b_o             		( led_b_o       			),
	.led_g_o             		( led_g_o       			),
	.boot_o             		( boot       				),
    .Interrupt_o	            ( Interrupt_o	            ),

	//MISC
    .Device_ID_o                ( Device_ID                 )
	                                                        );
															 

															 

// Empty Verilog model of QLAL4S3B
//
qlal4s3b_cell_macro              u_qlal4s3b_cell_macro
                               (
    // AHB-To-FPGA Bridge
	//
    .WBs_ADR                   ( WBs_ADR                     ), // output [16:0] | Address Bus                to   FPGA
    .WBs_CYC                   ( WBs_CYC                     ), // output        | Cycle Chip Select          to   FPGA
    .WBs_BYTE_STB              ( WBs_BYTE_STB                ), // output  [3:0] | Byte Select                to   FPGA
    .WBs_WE                    ( WBs_WE                      ), // output        | Write Enable               to   FPGA
    .WBs_RD                    ( WBs_RD                      ), // output        | Read  Enable               to   FPGA
    .WBs_STB                   ( WBs_STB                     ), // output        | Strobe Signal              to   FPGA
    .WBs_WR_DAT                ( WBs_WR_DAT                  ), // output [31:0] | Write Data Bus             to   FPGA
    .WB_CLK                    ( WB_CLK                      ), // input         | FPGA Clock               from FPGA
    .WB_RST                    ( WB_RST                      ), // output        | FPGA Reset               to   FPGA
    .WBs_RD_DAT                ( WBs_RD_DAT                  ), // input  [31:0] | Read Data Bus              from FPGA
    .WBs_ACK                   ( WBs_ACK                     ), // input         | Transfer Cycle Acknowledge from FPGA
    //
    // SDMA Signals
    //
    .SDMA_Req                  (4'b0000						 ), // input   [3:0]
    .SDMA_Sreq                 (4'b0000		                 ), // input   [3:0]
    .SDMA_Done                 (  							 ), // output  [3:0]
    .SDMA_Active               ( 							 ), // output  [3:0]
    //
    // FB Interrupts
    //
    //.FB_msg_out                ( {  3'b000,  boot           }), // input   [3:0]
    .FB_msg_out                ( {  3'b000,  Interrupt_o    }), // input   [3:0]
    .FB_Int_Clr                (  8'h0                       ), // input   [7:0]
    .FB_Start                  (                             ), // output
    .FB_Busy                   (  1'b0                       ), // input
    //
    // FB Clocks
    //
    .Sys_Clk0                  ( Sys_Clk0                    ), // output
    .Sys_Clk0_Rst              ( Sys_Clk0_Rst                ), // output
    .Sys_Clk1                  ( Sys_Clk1                    ), // output
    .Sys_Clk1_Rst              ( Sys_Clk1_Rst                ), // output
    //
    // Packet FIFO
    //
    .Sys_PKfb_Clk              (  1'b0                       ), // input
    .Sys_PKfb_Rst              (                             ), // output
    .FB_PKfbData               ( 32'h0                       ), // input  [31:0]
    .FB_PKfbPush               (  4'h0                       ), // input   [3:0]
    .FB_PKfbSOF                (  1'b0                       ), // input
    .FB_PKfbEOF                (  1'b0                       ), // input
    .FB_PKfbOverflow           (                             ), // output
	//
	// Sensor Interface
	//
    .Sensor_Int                (                             ), // output  [7:0]
    .TimeStamp                 (                             ), // output [23:0]
    //
    // SPI Master APB Bus
    //
    .Sys_Pclk                  (                             ), // output
    .Sys_Pclk_Rst              (                             ), // output      <-- Fixed to add "_Rst"
    .Sys_PSel                  (  1'b0                       ), // input
    .SPIm_Paddr                ( 16'h0                       ), // input  [15:0]
    .SPIm_PEnable              (  1'b0                       ), // input
    .SPIm_PWrite               (  1'b0                       ), // input
    .SPIm_PWdata               ( 32'h0                       ), // input  [31:0]
    .SPIm_Prdata               (                             ), // output [31:0]
    .SPIm_PReady               (                             ), // output
    .SPIm_PSlvErr              (                             ), // output
    //
    // Misc
    //
    .Device_ID                 ( Device_ID[15:0]             ), // input  [15:0]
    //
    // FBIO Signals
    //
    .FBIO_In                   (                             ), // output [13:0] <-- Do Not make any connections; Use Constraint manager in SpDE to sFBIO
    .FBIO_In_En                (                             ), // input  [13:0] <-- Do Not make any connections; Use Constraint manager in SpDE to sFBIO
    .FBIO_Out                  (                             ), // input  [13:0] <-- Do Not make any connections; Use Constraint manager in SpDE to sFBIO
    .FBIO_Out_En               (                             ), // input  [13:0] <-- Do Not make any connections; Use Constraint manager in SpDE to sFBIO
	//
	// ???
	//
    .SFBIO                     (                             ), // inout  [13:0]
    .Device_ID_6S              ( 1'b0                        ), // input
    .Device_ID_4S              ( 1'b0                        ), // input
    .SPIm_PWdata_26S           ( 1'b0                        ), // input
    .SPIm_PWdata_24S           ( 1'b0                        ), // input
    .SPIm_PWdata_14S           ( 1'b0                        ), // input
    .SPIm_PWdata_11S           ( 1'b0                        ), // input
    .SPIm_PWdata_0S            ( 1'b0                        ), // input
    .SPIm_Paddr_8S             ( 1'b0                        ), // input
    .SPIm_Paddr_6S             ( 1'b0                        ), // input
    .FB_PKfbPush_1S            ( 1'b0                        ), // input
    .FB_PKfbData_31S           ( 1'b0                        ), // input
    .FB_PKfbData_21S           ( 1'b0                        ), // input
    .FB_PKfbData_19S           ( 1'b0                        ), // input
    .FB_PKfbData_9S            ( 1'b0                        ), // input
    .FB_PKfbData_6S            ( 1'b0                        ), // input
    .Sys_PKfb_ClkS             ( 1'b0                        ), // input
    .FB_BusyS                  ( 1'b0                        ), // input
    .WB_CLKS                   ( 1'b0                        )  // input
                                                             );

//pragma attribute u_qlal4s3b_cell_macro         preserve_cell true
//pragma attribute u_AL4S3B_FPGA_IP            preserve_cell true

endmodule
