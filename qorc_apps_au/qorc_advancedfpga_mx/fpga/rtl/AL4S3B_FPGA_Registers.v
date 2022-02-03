// -----------------------------------------------------------------------------
// title          : AL4S3B Example FPGA Register Module
// project        : Tamar2 Device
// -----------------------------------------------------------------------------
// file           : AL4S3B_FPGA_Registers.v
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
// date            version       author              description
// 2016/02/03      1.0        Rakesh Moolacheri     Initial Release
//
// -----------------------------------------------------------------------------
// Comments: This solution is specifically for use with the QuickLogic
//           AL4S3B device. 
// -----------------------------------------------------------------------------
//

`timescale 1ns / 10ps


module AL4S3B_FPGA_Registers ( 

                         // AHB-To_FPGA Bridge I/F
                         //
                         WBs_ADR_i,
                         WBs_CYC_i,
                         WBs_BYTE_STB_i,
                         WBs_WE_i,
                         WBs_STB_i,
                         WBs_DAT_i,
                         WBs_CLK_i,
                         WBs_RST_i,
                         WBs_DAT_o,
                         WBs_ACK_o,

                        color0,
                        color1,
                        color2,
                        color3,
                        duration0,
                        duration1,
                        duration2,
                        duration3,

                        Interrupt_o,

                         //
                         // Misc
                         //
                         Device_ID_o
                         );


//------Port Parameters----------------
//

parameter                ADDRWIDTH                   =   7  ;   // Allow for up to 128 registers in the FPGA
parameter                DATAWIDTH                   =  32  ;   // Allow for up to 128 registers in the FPGA

parameter                FPGA_REG_ID_VALUE_ADR       =  7'h0; 
parameter                FPGA_REV_NUM_ADR            =  7'h1; 
parameter                FPGA_SCRATCH_REG_ADR        =  7'h2;

parameter                FPGA_COLORS_ADR            =  7'h04;   // byte offset 0x10
parameter                FPGA_DURATION0_ADR         =  7'h08;   // byte offset 0x20
parameter                FPGA_DURATION1_ADR         =  7'h09;   // byte offset 0x24
parameter                FPGA_DURATION2_ADR         =  7'h0A;   // byte offset 0x28
parameter                FPGA_DURATION3_ADR         =  7'h0B;   // byte offset 0x2C

parameter                AL4S3B_DEF_REG_VALUE        = 32'hFAB_DEF_AC;


//------Port Signals-------------------
//

// AHB-To_FPGA Bridge I/F
//
input   [ADDRWIDTH-1:0]  WBs_ADR_i     ;  // Address Bus                to   FPGA
input                    WBs_CYC_i     ;  // Cycle Chip Select          to   FPGA
input             [3:0]  WBs_BYTE_STB_i;  // Byte Select                to   FPGA
input                    WBs_WE_i      ;  // Write Enable               to   FPGA
input                    WBs_STB_i     ;  // Strobe Signal              to   FPGA
input   [DATAWIDTH-1:0]  WBs_DAT_i     ;  // Write Data Bus             to   FPGA
input                    WBs_CLK_i     ;  // FPGA Clock               from FPGA
input                    WBs_RST_i     ;  // FPGA Reset               to   FPGA
output  [DATAWIDTH-1:0]  WBs_DAT_o     ;  // Read Data Bus              from FPGA
output                   WBs_ACK_o     ;  // Transfer Cycle Acknowledge from FPGA

output reg  [2:0]   color0;
output reg  [2:0]   color1;
output reg  [2:0]   color2;
output reg  [2:0]   color3;
output reg  [11:0]  duration0;
output reg  [11:0]  duration1;
output reg  [11:0]  duration2;
output reg  [11:0]  duration3;

output          Interrupt_o;

//
// Misc
//
output           [31:0]  Device_ID_o   ;



// FPGA Global Signals
//
wire                     WBs_CLK_i     ;  // Wishbone FPGA Clock
wire                     WBs_RST_i     ;  // Wishbone FPGA Reset

// Wishbone Bus Signals
//
wire    [ADDRWIDTH-1:0]  WBs_ADR_i     ;  // Wishbone Address Bus
wire                     WBs_CYC_i     ;  // Wishbone Client Cycle  Strobe (i.e. Chip Select)
wire              [3:0]  WBs_BYTE_STB_i;  // Wishbone Byte   Enables
wire                     WBs_WE_i      ;  // Wishbone Write  Enable Strobe
wire                     WBs_STB_i     ;  // Wishbone Transfer      Strobe
wire    [DATAWIDTH-1:0]  WBs_DAT_i     ;  // Wishbone Write  Data Bus
 
reg     [DATAWIDTH-1:0]  WBs_DAT_o     ;  // Wishbone Read   Data Bus

reg                      WBs_ACK_o     ;  // Wishbone Client Acknowledge

// Misc
//
wire              [31:0]  Device_ID_o;
wire              [31:0]  Rev_Num;

reg 			  [15:0] Scratch_reg;

//------Define Parameters--------------
//

//
// None at this time
//

//------Internal Signals---------------
//
wire					 FB_SCRATCH_REG_Wr_Dcd;

wire					 WBs_ACK_o_nxt;


//------Logic Operations---------------

// Define the FPGA's local register write enables
//
assign FB_SCRATCH_REG_Wr_Dcd  = (WBs_ADR_i == FPGA_SCRATCH_REG_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_COLORS_REG_Wr_Dcd   = (WBs_ADR_i == FPGA_COLORS_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_DURATION0_REG_Wr_Dcd   = (WBs_ADR_i == FPGA_DURATION0_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_DURATION1_REG_Wr_Dcd   = (WBs_ADR_i == FPGA_DURATION1_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_DURATION2_REG_Wr_Dcd   = (WBs_ADR_i == FPGA_DURATION2_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_DURATION3_REG_Wr_Dcd   = (WBs_ADR_i == FPGA_DURATION3_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);

// Define the Acknowledge back to the host for registers
//
assign WBs_ACK_o_nxt          =   WBs_CYC_i & WBs_STB_i & (~WBs_ACK_o);


// Define the FPGA's Local Registers
//
always @( posedge WBs_CLK_i or posedge WBs_RST_i)
begin
    if (WBs_RST_i)
    begin
		Scratch_reg <=  16'h0 ; 
        WBs_ACK_o   <=  1'b0  ;
        color0      <=  3'b0    ;
        color1      <=  3'b0    ;
        color2      <=  3'b0    ;
        color3      <=  3'b0    ;
        duration0   <=  12'b0   ;
        duration1   <=  12'b0   ;
        duration2   <=  12'b0   ;
        duration3   <=  12'b0   ;
    end  
    else
    begin
	
		if(FB_SCRATCH_REG_Wr_Dcd && WBs_BYTE_STB_i[0])
			Scratch_reg[7:0]   <= WBs_DAT_i[7:0]  ;

        if(FB_SCRATCH_REG_Wr_Dcd && WBs_BYTE_STB_i[1])
			Scratch_reg[15:8]  <= WBs_DAT_i[15:8] ;

        if (FB_COLORS_REG_Wr_Dcd) begin
            color0 <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[2:0]    : color0;
            color1 <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[10:8]   : color1;
            color2 <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[18:16]  : color2;
            color3 <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[26:24]  : color3;
        end

        if (FB_DURATION0_REG_Wr_Dcd) begin
            duration0[7:0]   <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[7:0]  : duration0[7:0];
            duration0[11:8]  <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[11:8] : duration0[11:8];
        end

        if (FB_DURATION1_REG_Wr_Dcd) begin
            duration1[7:0]   <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[7:0]  : duration1[7:0];
            duration1[11:8]  <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[11:8] : duration1[11:8];
        end

        if (FB_DURATION2_REG_Wr_Dcd) begin
            duration2[7:0]   <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[7:0]  : duration2[7:0];
            duration2[11:8]  <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[11:8] : duration2[11:8];
        end

        if (FB_DURATION3_REG_Wr_Dcd) begin
            duration3[7:0]   <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[7:0]  : duration3[7:0];
            duration3[11:8]  <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[11:8] : duration3[11:8];
        end

        WBs_ACK_o  <=  WBs_ACK_o_nxt  ;
    end  
end


assign Device_ID_o = 32'h0000A5BD;
assign Rev_Num     = 32'h00000100;

// Define the how to read the local registers and memory
//
always @(
         //WBs_ADR_i         or
         //Device_ID_o       or
         //Rev_Num		   or
		 //Scratch_reg 
		 *
 )
 begin
    case(WBs_ADR_i[ADDRWIDTH-1:0])
    FPGA_REG_ID_VALUE_ADR    : WBs_DAT_o <= Device_ID_o;
    FPGA_REV_NUM_ADR         : WBs_DAT_o <= Rev_Num;  
    FPGA_SCRATCH_REG_ADR     : WBs_DAT_o <= { 16'h0, Scratch_reg }; 
    FPGA_COLORS_ADR          : WBs_DAT_o <= { 5'b0, color3, 5'b0, color2, 5'b0, color1, 5'b0, color0};
    FPGA_DURATION0_ADR       : WBs_DAT_o <= { 20'b0, duration0};
    FPGA_DURATION1_ADR       : WBs_DAT_o <= { 20'b0, duration1};
    FPGA_DURATION2_ADR       : WBs_DAT_o <= { 20'b0, duration2};
    FPGA_DURATION3_ADR       : WBs_DAT_o <= { 20'b0, duration3};
	default                  : WBs_DAT_o <= AL4S3B_DEF_REG_VALUE;
	endcase
end

assign Interrupt_o = 1'b0;

endmodule
