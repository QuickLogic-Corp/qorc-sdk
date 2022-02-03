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
				
                //FPGA IP
				usbp_io,
				usbn_io,
				
				clk_48mhz_i,
				reset_i,
				
				spi_cs_o,
				spi_sck_o,
				spi_mosi_o,
				spi_miso_i,
				
				led_r_o,
				led_b_o,
				led_g_o,
				boot_o,
			    Interrupt_o,

                // Misc
                Device_ID_o
                );


//------Port Parameters----------------
//

parameter       APERWIDTH                   = 17            ;
parameter       APERSIZE                    =  9            ;

parameter       FPGA_REG_BASE_ADDRESS       = 17'h00000     ; // Assumes 128K Byte FPGA Memory Aperture
parameter       QL_RESERVED_BASE_ADDRESS    = 17'h00800     ; // Assumes 128K Byte FPGA Memory Aperture

parameter       ADDRWIDTH_FAB_REG           =  7            ;
parameter       DATAWIDTH_FAB_REG           = 32            ;
	
parameter       FPGA_REG_ID_VALUE_ADR     	=  7'h0; 
parameter       FPGA_REV_NUM_ADR          	=  7'h1; 
parameter       FPGA_SCRATCH_REG_ADR      	=  7'h2; 



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

output    [31:0]  Device_ID_o    ;

inout 			usbp_io;
inout 			usbn_io;
	
input 			clk_48mhz_i;
input 			reset_i;
	
output 			spi_cs_o;
output 			spi_sck_o;
output 			spi_mosi_o;
input 			spi_miso_i;
	
output 			led_r_o;
output 			led_b_o;
output 			led_g_o;
output 			boot_o;
output          Interrupt_o;


assign boot_o = 0;
assign spi_cs_o = 0;
assign spi_sck_o = 0;
assign spi_mosi_o = 0;


// FPGA IP Internal Signals

wire usb_p_rx_int, usb_n_rx_int ;


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

// Wishbone Bus Signals
//
wire            WBs_CYC_FPGA_Reg   ;
wire            WBs_CYC_QL_Reserved  ;

wire            WBs_ACK_FPGA_Reg   ;
wire            WBs_ACK_QL_Reserved  ;

wire    [31:0]  WBs_DAT_o_FPGA_Reg ;
wire    [31:0]  WBs_DAT_o_QL_Reserved;

//
wire    [31:0]  Device_ID_o    ;

wire 			clk_12mhz;


wire    [11:0]  duration0;
wire    [11:0]  duration1;
wire    [11:0]  duration2;
wire    [11:0]  duration3;

wire    [2:0]   color0;
wire    [2:0]   color1;
wire    [2:0]   color2;
wire    [2:0]   color3;

wire            Interrupt_o           ;



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


//------Instantiate Modules------------
//

// Define the FPGA I/O Pad Signals
//
// Note: Use the Constraint manager in SpDE to assign these buffers to FBIO pads.
//

bipad u_bipad_I0  ( .A( 1'b0 ), .EN( 1'b0 ), .Q( usb_p_rx_int ), .P( usbp_io ) );
bipad u_bipad_I1  ( .A( 1'b0 ), .EN( 1'b0 ), .Q( usb_n_rx_int ), .P( usbn_io ) );


assign clk_12mhz = WB_CLK;

// User IP
LED_controller u_LED_controller (
    .clk        ( clk_12mhz     ),
    .rst        ( reset_i       ),

    .duration0  ( duration0     ),
    .duration1  ( duration1     ),
    .duration2  ( duration2     ),
    .duration3  ( duration3     ),

    .color0     ( color0        ),
    .color1     ( color1        ),
    .color2     ( color2        ),
    .color3     ( color3        ),

    .led_r      ( led_r_o       ),
    .led_g      ( led_g_o       ),
    .led_b      ( led_b_o       )
);



// General FPGA Resources 
//
AL4S3B_FPGA_Registers #(

    .ADDRWIDTH                  ( ADDRWIDTH_FAB_REG             ),
    .DATAWIDTH                  ( DATAWIDTH_FAB_REG             ),

    .FPGA_REG_ID_VALUE_ADR    	( FPGA_REG_ID_VALUE_ADR       	),
    .FPGA_REV_NUM_ADR         	( FPGA_REV_NUM_ADR            	),     
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
    .WBs_DAT_o                 ( WBs_DAT_o_FPGA_Reg           	),
    .WBs_ACK_o                 ( WBs_ACK_FPGA_Reg             	),

    .color0                    ( color0                         ),
    .color1                    ( color1                         ),
    .color2                    ( color2                         ),
    .color3                    ( color3                         ),
    .duration0                 ( duration0                      ),
    .duration1                 ( duration1                      ),
    .duration2                 ( duration2                      ),
    .duration3                 ( duration3                      ),

    .Interrupt_o               ( Interrupt_o           ),

    //
    // Misc
    //
    .Device_ID_o               ( Device_ID_o                     )
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
