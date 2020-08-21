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

                        control_o,
                        clkdiv_o,
                        arnold_reset_i,

                        interrupt_o,

                         //
                         // Misc
                         //
                         signature_o
                         );


//------Port Parameters----------------
//

parameter                ADDRWIDTH                   =   7  ;   // Allow for up to 128 registers in the FPGA
parameter                DATAWIDTH                   =  32  ;   // Allow for up to 128 registers in the FPGA

parameter                FPGA_SIGNATURE_ADR          =  7'h0; 
parameter                FPGA_REV_NUM_ADR            =  7'h1; 
parameter                FPGA_SCRATCH_REG_ADR        =  7'h2;

parameter                FPGA_CONTROL_REG_ADR        =  7'h04;   // byte offset 0x10
parameter                FPGA_CLKDIV_REG_ADR         =  7'h05;   // byte offset 0x14

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

output  [31:0]    control_o;
output  [31:0]    clkdiv_o;
input             arnold_reset_i;

output              interrupt_o;

//
// Misc
//
output           [31:0]  signature_o   ;



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
wire              [31:0]  signature_o;
wire              [31:0]  Rev_Num;

reg         [15:0] Scratch_reg;

//------Define Parameters--------------
//

//
// None at this time
//

//------Internal Registers---------------
//
reg         [15:0] scratch_reg;
reg         [31:0] control_reg;
reg         [31:0] clkdiv_reg;

//------Internal Signals---------------
//
wire           FB_SCRATCH_REG_Wr_Dcd;
wire           WBs_ACK_o_nxt;


//------Logic Operations---------------

// Define the FPGA's local register write enables
//
assign FB_SCRATCH_REG_Wr_Dcd     = (WBs_ADR_i == FPGA_SCRATCH_REG_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_CONTROL_REG_Wr_Dcd     = (WBs_ADR_i == FPGA_CONTROL_REG_ADR) & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FB_CLKDIV_REG_Wr_Dcd      = (WBs_ADR_i == FPGA_CLKDIV_REG_ADR)  & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);


// Define the Acknowledge back to the host for registers
//
assign WBs_ACK_o_nxt          =   WBs_CYC_i & WBs_STB_i & (~WBs_ACK_o);


// Define the FPGA's Local Registers
//
always @( posedge WBs_CLK_i or posedge WBs_RST_i)
begin
    if (WBs_RST_i)
    begin
    scratch_reg <=  16'h0 ; 
        WBs_ACK_o   <=  1'b0;
        control_reg <=  32'b0;
        clkdiv_reg  <=  32'b0;
    end  
    else
    begin
  
    if(FB_SCRATCH_REG_Wr_Dcd && WBs_BYTE_STB_i[0])
      scratch_reg[7:0]   <= WBs_DAT_i[7:0]  ;

        if(FB_SCRATCH_REG_Wr_Dcd && WBs_BYTE_STB_i[1])
      scratch_reg[15:8]  <= WBs_DAT_i[15:8] ;

        if (FB_CONTROL_REG_Wr_Dcd) begin
            control_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]  : control_reg[07:00];
            control_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]  : control_reg[15:08];
            control_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]  : control_reg[23:16];
            control_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]  : control_reg[31:24];
        end
        
        if (FB_CLKDIV_REG_Wr_Dcd) begin
            clkdiv_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]  : clkdiv_reg[07:00];
            clkdiv_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]  : clkdiv_reg[15:08];
            clkdiv_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]  : clkdiv_reg[23:16];
            clkdiv_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]  : clkdiv_reg[31:24];
        end

        WBs_ACK_o  <=  WBs_ACK_o_nxt  ;
    end  
end


assign signature_o = 32'h0000FEED;
assign Rev_Num     = 32'h00000100;

// Define the how to read the local registers and memory
//
always @(
         //WBs_ADR_i         or
         //Signature_o       or
         //Rev_Num       or
     //Scratch_reg 
     *
 )
 begin
    case(WBs_ADR_i[ADDRWIDTH-1:0])
    FPGA_SIGNATURE_ADR        : WBs_DAT_o <= signature_o;
    FPGA_REV_NUM_ADR          : WBs_DAT_o <= Rev_Num;  
    FPGA_SCRATCH_REG_ADR      : WBs_DAT_o <= { 13'h0, scratch_reg, control_o[0], control_reg[0], arnold_reset_i }; 
    FPGA_CONTROL_REG_ADR      : WBs_DAT_o <= control_reg;
    FPGA_CLKDIV_REG_ADR       : WBs_DAT_o <= clkdiv_reg;
    default                   : WBs_DAT_o <= AL4S3B_DEF_REG_VALUE;
  endcase
end

assign interrupt_o = 1'b0;
assign control_o  = control_reg;
assign clkdiv_o   = clkdiv_reg;

endmodule
