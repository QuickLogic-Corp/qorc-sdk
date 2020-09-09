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
                        status_i,
                        dataout_o,
                        datain_i,
                        oe_o,
                        active_on_p0_o,
                        active_on_p1_o,
                        active_on_p2_o,
                        active_on_p3_o,
                        databus_i,

                        interrupt_o,

                         //
                         // Misc
                         //
                         signature_o
                         );


//------Port Parameters----------------
//

parameter                ADDRWIDTH                  =   7  ;   // Allow for up to 128 registers in the FPGA
parameter                DATAWIDTH                  =  32  ;   // Allow for up to 128 registers in the FPGA

parameter                SIGNATURE_ADR              =  9'h00;  // Byte offset
parameter                FPGA_REVNUMBER_ADR         =  9'h04; 
parameter                FPGA_SCRATCH_REG_ADR       =  9'h08;

parameter                CONTROL_ADR                =  9'h10;   // Control register
parameter                STATUS_ADR                 =  9'h14;   // Status register
parameter                DATAOUT_ADR                =  9'h18;   // Data going to DUT
parameter                DATAIN_ADR                 =  9'h1C;   // Data from DUT (captured)

parameter                OE_ADR                     =  9'h20;   // Output enables 1 => output, 0 => input
parameter                ACTIVE_ON_P0_ADR           =  9'h24;   // Output is driven to dataout during phase 0
parameter                ACTIVE_ON_P1_ADR           =  9'h28;   // Output is driven to dataout during phase 1
parameter                ACTIVE_ON_P2_ADR           =  9'h2C;   // Output is driven to dataout during phase 2
parameter                ACTIVE_ON_P3_ADR           =  9'h30;   // Output is driven to dataout during phase 3

parameter                DATABUS_ADR                =  9'h34;   // Data from I/O (un-latched)

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
input                    WBs_CLK_i     ;  // FPGA Clock                 from FPGA
input                    WBs_RST_i     ;  // FPGA Reset                 to   FPGA
output  [DATAWIDTH-1:0]  WBs_DAT_o     ;  // Read Data Bus              from FPGA
output                   WBs_ACK_o     ;  // Transfer Cycle Acknowledge from FPGA
output                   interrupt_o   ;  // Interrupt to SoC

output      [31:0]  control_o;
input       [31:0]  status_i;
output      [31:0]  dataout_o;
input       [31:0]  datain_i;
output      [31:0]  oe_o;
output      [31:0]  active_on_p0_o;
output      [31:0]  active_on_p1_o;
output      [31:0]  active_on_p2_o;
output      [31:0]  active_on_p3_o;
input       [31:0]  databus_i;



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
wire    [31:0] signature_o;
wire    [31:0] revnumber_o;

reg     [15:0] scratch_reg;
reg     [31:0] control_reg;
reg     [31:0] dataout_reg;
reg     [31:0] oe_reg;
reg     [31:0] active_on_p0_reg;
reg     [31:0] active_on_p1_reg;
reg     [31:0] active_on_p2_reg;
reg     [31:0] active_on_p3_reg;

//------Define Parameters--------------
//

//
// None at this time
//

//------Internal Signals---------------
//
wire					 FB_SCRATCH_REG_Wr_Dcd;

wire					 WBs_ACK_o_nxt;

//------Assign Outputs-----------------

assign signature_o  = 32'h0000DEEB;
assign revnumber    = 32'h00000100;
assign control_o = control_reg;
assign dataout_o = dataout_reg;
assign oe_o = oe_reg;
assign active_on_p0_o = active_on_p0_reg;
assign active_on_p1_o = active_on_p1_reg;
assign active_on_p2_o = active_on_p2_reg;
assign active_on_p3_o = active_on_p3_reg;

//------Logic Operations---------------

// Define the FPGA's local register write enables
//
assign SIGNATURE_REG_Wr_Dcd     = ({WBs_ADR_i,2'h0} == SIGNATURE_ADR)           & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FPGA_REV_NUM_REG_Wr_Dcd  = ({WBs_ADR_i,2'h0} == FPGA_REV_NUM_ADR)        & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign FPGA_SCRATCH_REG_Wr_Dcd  = ({WBs_ADR_i,2'h0} == FPGA_SCRATCH_REG_ADR)    & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign CONTROL_REG_Wr_Dcd       = ({WBs_ADR_i,2'h0} == CONTROL_ADR)             & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign STATUS_REG_Wr_Dcd        = ({WBs_ADR_i,2'h0} == CONTROL_ADR)             & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign DATAOUT_REG_Wr_Dcd       = ({WBs_ADR_i,2'h0} == DATAOUT_ADR)             & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign DATAIN_REG_Wr_Dcd        = ({WBs_ADR_i,2'h0} == DATAIN_ADR)              & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign OE_REG_Wr_Dcd            = ({WBs_ADR_i,2'h0} == OE_ADR)                  & WBs_CYC_i & WBs_STB_i & WBs_WE_i  & (~WBs_ACK_o);
assign ACTIVE_ON_P0_REG_Wr_Dcd  = ({WBs_ADR_i,2'h0} == ACTIVE_ON_P0_ADR)        & WBs_CYC_i & WBs_STB_i & WBs_WE_i  & (~WBs_ACK_o);
assign ACTIVE_ON_P1_REG_Wr_Dcd  = ({WBs_ADR_i,2'h0} == ACTIVE_ON_P1_ADR)        & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign ACTIVE_ON_P2_REG_Wr_Dcd  = ({WBs_ADR_i,2'h0} == ACTIVE_ON_P2_ADR)        & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign ACTIVE_ON_P3_REG_Wr_Dcd  = ({WBs_ADR_i,2'h0} == ACTIVE_ON_P3_ADR)        & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);
assign DATABUS_REG_Wr_Dcd       = ({WBs_ADR_i,2'h0} == DATABUS_ADR)             & WBs_CYC_i & WBs_STB_i & WBs_WE_i   & (~WBs_ACK_o);



// Define the Acknowledge back to the host for registers
//
assign WBs_ACK_o_nxt          =   WBs_CYC_i & WBs_STB_i & (~WBs_ACK_o);


// Define the FPGA's Local Registers
//
always @( posedge WBs_CLK_i or posedge WBs_RST_i)
begin
    if (WBs_RST_i)
    begin
		Scratch_reg         <=  16'h0 ; 
        WBs_ACK_o           <=  1'b0  ;
        dataout_reg        <=  32'b0    ;
        datain_reg         <=  32'b0    ;
        oe_reg              <=  32'b0    ;
        active_on_p0_reg    <=  33'b0    ;
        active_on_p1_reg    <=  32'b0   ;
        active_on_p2_reg    <=  32'b0   ;
        active_on_p3_reg    <=  32'b0   ;
    end  
    else
    begin
	
		if(FB_SCRATCH_REG_Wr_Dcd && WBs_BYTE_STB_i[0]) begin
			scratch_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : scratch_reg[07:00];
            scratch_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : scratch_reg[15:08];
        end

        if (CONTROL_REG_Wr_Dcd) begin
            control_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : control_reg[07:00];
            control_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : control_reg[15:08];
            control_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : control_reg[23:16];
            control_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : control_reg[31:24];
        end
        
        if (DATAOUT_REG_Wr_Dcd) begin
            dataout_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : dataout_reg[07:00];
            dataout_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : dataout_reg[15:08];
            dataout_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : dataout_reg[23:16];
            dataout_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : dataout_reg[31:24];
        end

        if (OE_REG_Wr_Dcd) begin
            oe_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : oe_reg[07:00];
            oe_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : oe_reg[15:08];
            oe_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : oe_reg[23:16];
            oe_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : oe_reg[31:24];
        end
        
        if (ACTIVE_ON_P0_REG_Wr_Dcd) begin
            active_on_p0_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : active_on_p0_reg[07:00];
            active_on_p0_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : active_on_p0_reg[15:08];
            active_on_p0_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : active_on_p0_reg[23:16];
            active_on_p0_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : active_on_p0_reg[31:24];
        end
        
        if (ACTIVE_ON_P1_REG_Wr_Dcd) begin
            active_on_p1_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : active_on_p1_reg[07:00];
            active_on_p1_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : active_on_p1_reg[15:08];
            active_on_p1_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : active_on_p1_reg[23:16];
            active_on_p1_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : active_on_p1_reg[31:24];
        end
        
        if (ACTIVE_ON_P2_REG_Wr_Dcd) begin
            active_on_p2_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : active_on_p2_reg[07:00];
            active_on_p2_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : active_on_p2_reg[15:08];
            active_on_p2_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : active_on_p2_reg[23:16];
            active_on_p2_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : active_on_p2_reg[31:24];
        end
        
        if (ACTIVE_ON_P3_REG_Wr_Dcd) begin
            active_on_p3_reg[07:00] <= WBs_BYTE_STB_i[0] ? WBs_DAT_i[07:00]    : active_on_p3_reg[07:00];
            active_on_p3_reg[15:08] <= WBs_BYTE_STB_i[1] ? WBs_DAT_i[15:08]    : active_on_p3_reg[15:08];
            active_on_p3_reg[23:16] <= WBs_BYTE_STB_i[2] ? WBs_DAT_i[23:16]    : active_on_p3_reg[23:16];
            active_on_p3_reg[31:24] <= WBs_BYTE_STB_i[3] ? WBs_DAT_i[31:24]    : active_on_p3_reg[31:24];
        end

        WBs_ACK_o  <=  WBs_ACK_o_nxt  ;
    end  
end



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
    case({WBs_ADR_i[ADDRWIDTH-1:0],2'b0})
    SIGNATURE_ADR           : WBs_DAT_o <= signature_o;
    REV_NUM_ADR             : WBs_DAT_o <= revnumber;  
    SCRATCH_REG_ADR         : WBs_DAT_o <= { 16'h0, scratch_reg }; 
    CONTROL_ADR             : WBs_DAT_o <= control_reg;
    STATUS_ADR              : WBs_DAT_o <= status_i;
    OE_ADR                  : WBs_DAT_o <= oe_reg;
    DATAOUT_ADR             : WBs_DAT_o <= dataout_reg;
    DATAIN_ADR              : WBs_DAT_o <= datain_i;
    ACTIVE_ON_P0_ADR        : WBs_DAT_o <= active_on_p0_reg;
    ACTIVE_ON_P1_ADR        : WBs_DAT_o <= active_on_p1_reg;
    ACTIVE_ON_P2_ADR        : WBs_DAT_o <= active_on_p2_reg;
    ACTIVE_ON_P3_ADR        : WBs_DAT_o <= active_on_p3_reg;
    DATABUS_ADR             : WBs_DAT_o <= databus_i;
	default                 : WBs_DAT_o <= AL4S3B_DEF_REG_VALUE;
	endcase
end

assign interrupt_o = 1'b0;

endmodule
