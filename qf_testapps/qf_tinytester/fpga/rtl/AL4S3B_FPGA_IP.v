// -----------------------------------------------------------------------------
// title          : AL4S3B Example FPGA IP Module
// project        : Tamar2 Device
// -----------------------------------------------------------------------------
// file           : AL4S3B_FPGA_IP.v
// author         : SSG
// company        : QuickLogic Corp
// created        : 2016/02/03	
// last update    : 2016/02/03
// platform       : ArcticLink 4 S3B
// standard       : Verilog 2001
// -----------------------------------------------------------------------------
// description: The FPGA example IP design contains the essential logic for
//              interfacing the ASSP of the AL4S3B to registers and memory 
//              located in the programmable FPGA.
// -----------------------------------------------------------------------------
// copyright (c) 2016
// -----------------------------------------------------------------------------
// revisions  :
// date            version       author             description
// 2016/02/03      1.0       Rakesh Moolacheri     Initial Release
// -----------------------------------------------------------------------------
// Comments: This solution is specifically for use with the QuickLogic
//           AL4S3B device. 
// -----------------------------------------------------------------------------
//

`timescale 1ns / 10ps
module AL4S3B_FPGA_IP ( 

                // AHB-To_FPGA Bridge I/F
                //
                WBs_ADR,
                WBs_CYC,
                WBs_BYTE_STB,
                WBs_WE,
                WBs_RD,
                WBs_STB,
                WBs_WR_DAT,
                WB_CLK,
                WB_RST,
                WBs_RD_DAT,
                WBs_ACK,
                interrupt_o,
				
                // Custom gateware signals
				clk_48mhz_i,
				reset_i,
                
                // I/O signals
                IO_23_b,
                IO_31_b,
                IO_12_b,
                IO_5_b,
                IO_7_b,
                IO_10_b,
                IO_29_b,
                IO_24_b,
                IO_11_b,
                IO_4_b,
                IO_30_b,
                IO_8_b,
                IO_25_b,
                IO_13_b,
                IO_17_b,
                IO_16_b,
                IO_3_b,
                IO_0_b,
                IO_1_b,

                // Misc
                IO_2_b,         // Test signal (monitors state[5])
                signature_o
                );


//------Port Parameters----------------
//

parameter       APERWIDTH                   = 17            ;
parameter       APERSIZE                    =  9            ;

parameter       FPGA_REG_BASE_ADDRESS       = 17'h00000     ; // Assumes 128K Byte FPGA Memory Aperture
parameter       QL_RESERVED_BASE_ADDRESS    = 17'h00800     ; // Assumes 128K Byte FPGA Memory Aperture

parameter       ADDRWIDTH_FAB_REG           =  7            ;
parameter       DATAWIDTH_FAB_REG           = 32            ;
	
parameter       SIGNATURE_ADR              =  7'h00;  // Byte offset
parameter       FPGA_REVNUMBER_ADR         =  7'h04; 
parameter       FPGA_SCRATCH_REG_ADR       =  7'h08;



parameter       AL4S3B_DEF_REG_VALUE        = 32'hFAB_DEF_AC; // Distinguish access to undefined area

parameter       DEFAULT_READ_VALUE          = 32'hBAD_FAB_AC; // Bad FPGA Access
parameter       DEFAULT_CNTR_WIDTH          =  3            ;
parameter       DEFAULT_CNTR_TIMEOUT        =  7            ;

parameter       ADDRWIDTH_QL_RESERVED       =  7            ;
parameter       DATAWIDTH_QL_RESERVED       = 32            ;

parameter       QL_RESERVED_CUST_PROD_ADR   =  7'h7E        ;
parameter       QL_RESERVED_REVISIONS_ADR   =  7'h7F        ;

parameter       QL_RESERVED_CUSTOMER_ID     =  8'h01        ;
parameter       QL_RESERVED_PRODUCT_ID      =  8'h00        ;
parameter       QL_RESERVED_MAJOR_REV       = 16'h0001      ; 
parameter       QL_RESERVED_MINOR_REV       = 16'h0000      ;

parameter       QL_RESERVED_DEF_REG_VALUE   = 32'hDEF_FAB_AC; // Distinguish access to undefined area


//------Port Signals-------------------
//

// AHB-To_FPGA Bridge I/F
//
input   [16:0]  WBs_ADR          ;  // Address Bus                to   FPGA
input           WBs_CYC          ;  // Cycle Chip Select          to   FPGA
input    [3:0]  WBs_BYTE_STB     ;  // Byte Select                to   FPGA
input           WBs_WE           ;  // Write Enable               to   FPGA
input           WBs_RD           ;  // Read  Enable               to   FPGA
input           WBs_STB          ;  // Strobe Signal              to   FPGA
input   [31:0]  WBs_WR_DAT       ;  // Write Data Bus             to   FPGA
input           WB_CLK           ;  // FPGA Clock               from FPGA
input           WB_RST           ;  // FPGA Reset               to   FPGA
output  [31:0]  WBs_RD_DAT       ;  // Read Data Bus              from FPGA
output          WBs_ACK          ;  // Transfer Cycle Acknowledge from FPGA
output          interrupt_o;

// Gateware signals
input 			clk_48mhz_i;
input 			reset_i;
	
inout 			IO_23_b;
inout 			IO_31_b;
inout 			IO_12_b;
inout 			IO_5_b;
inout 			IO_7_b;
inout 			IO_10_b;
inout 			IO_29_b;
inout 			IO_24_b;
inout 			IO_11_b;
inout 			IO_4_b;
inout 			IO_30_b;
inout 			IO_8_b;
inout 			IO_25_b;
inout 			IO_13_b;
inout 			IO_17_b;
inout 			IO_16_b;
inout 			IO_3_b;
inout 			IO_0_b;
inout 			IO_1_b;

inout           IO_2_b;     // Test signal (monitors state[5])

// Misc signals
output    [31:0]  signature_o    ;


// FPGA IP Internal Signals

wire [6:0] sequencer_state;


// FPGA Global Signals
//
wire            WB_CLK           ;  // Wishbone FPGA Clock
wire            WB_RST           ;  // Wishbone FPGA Reset

// Wishbone Bus Signals
//
wire    [16:0]  WBs_ADR          ;  // Wishbone Address Bus
wire            WBs_CYC          ;  // Wishbone Client Cycle  Strobe (i.e. Chip Select)
wire     [3:0]  WBs_BYTE_STB     ;  // Wishbone Byte   Enables
wire            WBs_WE           ;  // Wishbone Write  Enable Strobe
wire            WBs_RD           ;  // Wishbone Read   Enable Strobe
wire            WBs_STB          ;  // Wishbone Transfer      Strobe

reg     [31:0]  WBs_RD_DAT       ;  // Wishbone Read   Data Bus
wire    [31:0]  WBs_WR_DAT       ;  // Wishbone Write  Data Bus
wire            WBs_ACK          ;  // Wishbone Client Acknowledge


//------Internal Signals---------------
//
wire    [31:0]  signature_o;      // Gateware signature
wire    [31:0]  dataout;        // Data to be applied to the DUT
wire    [31:0]  datain;         // Data coming from the DUT
wire    [31:0]  oe;             // Signals indicating whether this bit is an 1 => output, 0 => input
wire    [31:0]  active_on_p0;   // Signals indicating whether this bit is driven on P0: 1 => drive to dataout[i], 0 => drive to '0'
wire    [31:0]  active_on_p1;   // Signals indicating whether this bit is driven on P1: 1 => drive to dataout[i], 0 => drive to '0'
wire    [31:0]  active_on_p2;   // Signals indicating whether this bit is driven on P2: 1 => drive to dataout[i], 0 => drive to '0'
wire    [31:0]  active_on_p3;   // Signals indicating whether this bit is driven on P3: 1 => drive to dataout[i], 0 => drive to '0'

wire    [31:0]  padout;         // Output signals to pads
wire    [31:0]  padoe;          // Output enable signals to pads
wire    [31:0]  padin;          // Input singals from pads

// Wishbone Bus Signals
//
wire            WBs_CYC_FPGA_Reg   ;
wire            WBs_CYC_QL_Reserved  ;

wire            WBs_ACK_FPGA_Reg   ;
wire            WBs_ACK_QL_Reserved  ;

wire    [31:0]  WBs_DAT_o_FPGA_Reg ;
wire    [31:0]  WBs_DAT_o_QL_Reserved;

wire 			clk_12mhz;


wire            interrupt_o;



//------Logic Operations---------------
//


// Define the Chip Select for each interface
//
assign WBs_CYC_FPGA_Reg   = (  WBs_ADR[APERWIDTH-1:APERSIZE+2] == FPGA_REG_BASE_ADDRESS    [APERWIDTH-1:APERSIZE+2] ) 
                            & (  WBs_CYC                                                                                );

assign WBs_CYC_QL_Reserved  = (  WBs_ADR[APERWIDTH-1:APERSIZE+2] == QL_RESERVED_BASE_ADDRESS   [APERWIDTH-1:APERSIZE+2] ) 
                            & (  WBs_CYC  																				);


// Define the Acknowledge back to the host for everything
//
assign WBs_ACK              =    WBs_ACK_FPGA_Reg
                            |    WBs_ACK_QL_Reserved;


// Define the how to read from each IP
//
always @(
         WBs_ADR               or
         WBs_DAT_o_FPGA_Reg  or
         WBs_DAT_o_QL_Reserved or
         WBs_RD_DAT    
        )
 begin
    case(WBs_ADR[APERWIDTH-1:APERSIZE+2])
    FPGA_REG_BASE_ADDRESS    [APERWIDTH-1:APERSIZE+2]: WBs_RD_DAT    <=          WBs_DAT_o_FPGA_Reg     ;
    QL_RESERVED_BASE_ADDRESS   [APERWIDTH-1:APERSIZE+2]: WBs_RD_DAT  <=          WBs_DAT_o_QL_Reserved  ;
	default:                                             WBs_RD_DAT  <=          DEFAULT_READ_VALUE     ;
	endcase
end

// Define the FPGA I/O Pad Signals
//
// Note: Use the pcf file to place these on the correct I/O
//

assign IO_23_b = (padoe[0]) ? padout[0] : 1'bZ; assign padin[0] = IO_23_b;
assign IO_31_b = (padoe[1]) ? padout[1] : 1'bZ; assign padin[1] = IO_31_b;
assign IO_12_b = (padoe[2]) ? padout[2] : 1'bZ; assign padin[2] = IO_12_b;
assign IO_5_b  = (padoe[3]) ? padout[3] : 1'bZ; assign padin[3] = IO_5_b;
assign IO_7_b  = (padoe[4]) ? padout[4] : 1'bZ; assign padin[4] = IO_7_b;
assign IO_10_b = (padoe[5]) ? padout[5] : 1'bZ; assign padin[5] = IO_10_b;
assign IO_29_b = (padoe[6]) ? padout[6] : 1'bZ; assign padin[6] = IO_29_b;
assign IO_24_b = (padoe[7]) ? padout[7] : 1'bZ; assign padin[7] = IO_24_b;
assign IO_11_b = (padoe[8]) ? padout[8] : 1'bZ; assign padin[8] = IO_11_b;
assign IO_4_b  = (padoe[9]) ? padout[9] : 1'bZ; assign padin[9] = IO_4_b;
assign IO_30_b = (padoe[10]) ? padout[10] : 1'bZ; assign padin[10] = IO_30_b;
assign IO_8_b  = (padoe[11]) ? padout[11] : 1'bZ; assign padin[11] = IO_8_b;
assign IO_25_b = (padoe[12]) ? padout[12] : 1'bZ; assign padin[12] = IO_25_b;
assign IO_13_b = (padoe[13]) ? padout[13] : 1'bZ; assign padin[13] = IO_13_b;
assign IO_17_b = (padoe[14]) ? padout[14] : 1'bZ; assign padin[14] = IO_17_b;
assign IO_16_b = (padoe[15]) ? padout[15] : 1'bZ; assign padin[15] = IO_16_b;
assign IO_3_b  = (padoe[16]) ? padout[16] : 1'bZ; assign padin[16] = IO_3_b;
assign IO_0_b  = (padoe[17]) ? padout[17] : 1'bZ; assign padin[17] = IO_0_b;
assign IO_1_b  = (padoe[18]) ? padout[18] : 1'bZ; assign padin[18] = IO_1_b;

assign IO_2_b  = sequencer_state[0];    // Monitor Phase 0


//------Instantiate Modules------------
//


assign clk_12mhz = WB_CLK;

// User IP
tinytester u_tinytester (
    .clk        ( clk_12mhz     ),
    .rst        ( reset_i       ),

    // Register interface
    .control_i      ( control           ),
    .dataout_i      ( dataout           ),
    .datain_o       ( datain            ),
    .oe_i           ( oe                ),
    .active_on_p0_i ( active_on_p0      ),
    .active_on_p1_i ( active_on_p1      ),
    .active_on_p2_i ( active_on_p2      ),
    .active_on_p3_i ( active_on_p3       ),

    // External IO I/F
    .padout_o       ( padout        ),
    .padoe_o        ( padoe         ),
    .padin_i        ( padin         ),
    
    .sequencer_state( sequencer_state   )
);



// General FPGA Resources 
//
AL4S3B_FPGA_Registers #(

    .ADDRWIDTH                  ( ADDRWIDTH_FAB_REG             ),
    .DATAWIDTH                  ( DATAWIDTH_FAB_REG             ),

    .SIGNATURE_ADR              ( SIGNATURE_ADR       	),
    .FPGA_REVNUMBER_ADR         ( FPGA_REVNUMBER_ADR            	),     
    .FPGA_SCRATCH_REG_ADR     	( FPGA_SCRATCH_REG_ADR        	),

    .AL4S3B_DEF_REG_VALUE       ( AL4S3B_DEF_REG_VALUE          )
                                                                )

     u_AL4S3B_FPGA_Registers 
	                           ( 
    // AHB-To_FPGA Bridge I/F
    //
    .WBs_ADR_i                 ( WBs_ADR[ADDRWIDTH_FAB_REG+1:2] ),
    .WBs_CYC_i                 ( WBs_CYC_FPGA_Reg             	),
    .WBs_BYTE_STB_i            ( WBs_BYTE_STB                   ),
    .WBs_WE_i                  ( WBs_WE                         ),
    .WBs_STB_i                 ( WBs_STB                        ),
    .WBs_DAT_i                 ( WBs_WR_DAT                     ),
    .WBs_CLK_i                 ( WB_CLK                         ),
    .WBs_RST_i                 ( WB_RST                         ),
    .WBs_DAT_o                 ( WBs_DAT_o_FPGA_Reg             ),
    .WBs_ACK_o                 ( WBs_ACK_FPGA_Reg               ),
    .interrupt_o               ( interrupt_o                    ),

    // Register and data
    .control_o                 ( control                        ),
    .status_i                  ( sequencer_state                ),
    .dataout_o                 ( dataout                        ),
    .datain_i                  ( datain                         ),
    .oe_o                      ( oe                             ),
    .active_on_p0_o            ( active_on_p0                   ),
    .active_on_p1_o            ( active_on_p1                   ),
    .active_on_p2_o            ( active_on_p2                   ),
    .active_on_p3_o            ( active_on_p3                   ),
    .databus_i                 ( padin                          ),

    //
    // Misc
    //
    .signature_o               ( signature_o                     )
   );


// Reserved Resources Block
//
// Note: This block should be used in each QL FPGA design
//
AL4S3B_FPGA_QL_Reserved     #(

    .ADDRWIDTH                 ( ADDRWIDTH_QL_RESERVED          ),
    .DATAWIDTH                 ( DATAWIDTH_QL_RESERVED          ),

    .QL_RESERVED_CUST_PROD_ADR ( QL_RESERVED_CUST_PROD_ADR      ),
    .QL_RESERVED_REVISIONS_ADR ( QL_RESERVED_REVISIONS_ADR      ),

    .QL_RESERVED_CUSTOMER_ID   ( QL_RESERVED_CUSTOMER_ID        ),
    .QL_RESERVED_PRODUCT_ID    ( QL_RESERVED_PRODUCT_ID         ),
    .QL_RESERVED_MAJOR_REV     ( QL_RESERVED_MAJOR_REV          ),
    .QL_RESERVED_MINOR_REV     ( QL_RESERVED_MINOR_REV          ),
    .QL_RESERVED_DEF_REG_VALUE ( QL_RESERVED_DEF_REG_VALUE      ),

    .DEFAULT_CNTR_WIDTH        ( DEFAULT_CNTR_WIDTH             ),
    .DEFAULT_CNTR_TIMEOUT      ( DEFAULT_CNTR_TIMEOUT           )
                                                                )	
                                 u_AL4S3B_FPGA_QL_Reserved
							   (
     // AHB-To_FPGA Bridge I/F
     //
    .WBs_CLK_i                 ( WB_CLK                         ),
    .WBs_RST_i                 ( WB_RST                         ),

    .WBs_ADR_i                 ( WBs_ADR[ADDRWIDTH_FAB_REG+1:2] ),
    .WBs_CYC_QL_Reserved_i     ( WBs_CYC_QL_Reserved            ),
    .WBs_CYC_i                 ( WBs_CYC                        ),
    .WBs_STB_i                 ( WBs_STB                        ),
    .WBs_DAT_o                 ( WBs_DAT_o_QL_Reserved          ),
    .WBs_ACK_i                 ( WBs_ACK                        ),
    .WBs_ACK_o                 ( WBs_ACK_QL_Reserved            )

                                                                );



//pragma attribute u_AL4S3B_FPGA_Registers   preserve_cell true
//pragma attribute u_AL4S3B_FPGA_QL_Reserved preserve_cell true 

//pragma attribute u_bipad_I0                  preserve_cell true
//pragma attribute u_bipad_I1                  preserve_cell true 

endmodule
