`timescale 1ns / 10ps
module AL4S3B_FPGA_IP ( 

                // AHB-To_FPGA Bridge I/F
                //
                CLK_IP_i,
                RST_IP_i,

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

                // UART 0
                UART0_SIN_i,
                UART0_SOUT_o,

                UART0_Intr_o,

                Device_ID_o
                );


//------Port Parameters----------------
//

parameter       APERWIDTH                   = 17            ;
parameter       APERSIZE                    = 10            ;

                                                                // these are byte offsets.
parameter       FPGA_REG_BASE_ADDRESS       = 17'h00000     ; // Assumes 128K Byte FPGA Memory Aperture
parameter       UART0_BASE_ADDRESS          = 17'h01000     ;
//parameter       QL_RESERVED_BASE_ADDRESS    = 17'h12000     ; // Assumes 128K Byte FPGA Memory Aperture
parameter       QL_RESERVED_BASE_ADDRESS    = 17'h03000     ; // Assumes 128K Byte FPGA Memory Aperture

parameter       ADDRWIDTH_FAB_REG           =  10           ;
parameter       DATAWIDTH_FAB_REG           =  32           ;

                                                                // these are byte offsets
                                                                //  the 2 LSB's (on the right) should be 0's.
parameter       FPGA_REG_ID_VALUE_ADR       = 10'h000       ; 
parameter       FPGA_REV_NUM_ADR            = 10'h004       ; 

parameter       AL4S3B_DEVICE_ID            = 16'h0         ;
parameter       AL4S3B_REV_LEVEL            = 32'h0         ;
parameter       AL4S3B_SCRATCH_REG          = 32'h12345678  ;

parameter       AL4S3B_DEF_REG_VALUE        = 32'hFAB_DEF_AC; // Distinguish access to undefined area

parameter       DEFAULT_READ_VALUE          = 32'hBAD_FAB_AC; // Bad FPGA Access
parameter       DEFAULT_CNTR_WIDTH          =  3            ;
parameter       DEFAULT_CNTR_TIMEOUT        =  7            ;

parameter       ADDRWIDTH_QL_RESERVED       = 10            ;
parameter       DATAWIDTH_QL_RESERVED       = 32            ;

parameter       QL_RESERVED_CUST_PROD_ADR   = 10'h1F8       ;
parameter       QL_RESERVED_REVISIONS_ADR   = 10'h1FC       ;

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

// Misc
//
input                   CLK_IP_i;  
input                   RST_IP_i; 


output    [31:0]        Device_ID_o;

// UART 0
input                   UART0_SIN_i; 
output                  UART0_SOUT_o;
output                  UART0_Intr_o;

// FPGA Global Signals
//
wire            WB_CLK          ;  // Wishbone FPGA Clock
wire            WB_RST          ;  // Wishbone FPGA Reset

// Wishbone Bus Signals
//
wire    [16:0]  WBs_ADR         ;  // Wishbone Address Bus
wire            WBs_CYC         ;  // Wishbone Client Cycle  Strobe (i.e. Chip Select)
wire     [3:0]  WBs_BYTE_STB    ;  // Wishbone Byte   Enables
wire            WBs_WE          ;  // Wishbone Write  Enable Strobe
wire            WBs_RD          ;  // Wishbone Read   Enable Strobe
wire            WBs_STB         ;  // Wishbone Transfer      Strobe

reg     [31:0]  WBs_RD_DAT      ;  // Wishbone Read   Data Bus
wire    [31:0]  WBs_WR_DAT      ;  // Wishbone Write  Data Bus
wire            WBs_ACK         ;  // Wishbone Client Acknowledge

wire            CLK_IP_i        ;
wire            RST_IP_i        ;

// UART 0
wire            UART0_SIN_i; 
wire            UART0_SOUT_o;
wire            UART0_Intr_o;


wire    [31:0]  Device_ID_o;


wire            dbg_reset;

//------Define Parameters--------------
//

// Default I/O timeout statemachine
//
parameter       DEFAULT_IDLE   =  0  ;
parameter       DEFAULT_COUNT  =  1  ;


//------Internal Signals---------------
//

// Wishbone Bus Signals
//
wire            WBs_CYC_FPGA_Reg    ; 
wire            WBs_CYC_UART0       ;
wire            WBs_CYC_QL_Reserved  ;

wire            WBs_ACK_FPGA_Reg   ;
wire            WBs_ACK_UART0      ;
wire            WBs_ACK_QL_Reserved  ;

wire    [31:0]  WBs_DAT_o_FPGA_Reg ; 
wire    [15:0]  WBs_DAT_o_UART0    ;
wire    [31:0]  WBs_DAT_o_QL_Reserved;

//------Logic Operations---------------
//
// Define the Chip Select for each interface
//
assign WBs_CYC_FPGA_Reg   = (  WBs_ADR[APERWIDTH-1:APERSIZE+2] == FPGA_REG_BASE_ADDRESS[APERWIDTH-1:APERSIZE+2] ) 
                            & (  WBs_CYC                                                                                );

assign WBs_CYC_UART0        = (  WBs_ADR[APERWIDTH-1:APERSIZE+2] == UART0_BASE_ADDRESS[APERWIDTH-1:APERSIZE+2] ) 
                            & (( WBs_CYC & WBs_WE  & WBs_BYTE_STB[0]                                                    ) 
                            |  ( WBs_CYC & WBs_RD                                                                       )); 

assign WBs_CYC_QL_Reserved  = (  WBs_ADR[APERWIDTH-1:APERSIZE+2] == QL_RESERVED_BASE_ADDRESS[APERWIDTH-1:APERSIZE+2] ) 
                            & (  WBs_CYC                                                                                );


// Define the Acknowledge back to the host for everything
//
assign WBs_ACK              =    WBs_ACK_FPGA_Reg | WBs_ACK_UART0 
                                 |    WBs_ACK_QL_Reserved;


// Define the how to read from each IP
//
always @(
        WBs_ADR               or
        WBs_DAT_o_FPGA_Reg    or
        WBs_DAT_o_UART0       or
        WBs_DAT_o_QL_Reserved or
        WBs_RD_DAT    
        )
 begin
    case(WBs_ADR[APERWIDTH-1:APERSIZE+2])
    FPGA_REG_BASE_ADDRESS    [APERWIDTH-1:APERSIZE+2]: WBs_RD_DAT  <=    WBs_DAT_o_FPGA_Reg     ;
    UART0_BASE_ADDRESS       [APERWIDTH-1:APERSIZE+2]: WBs_RD_DAT  <=    {16'h0, WBs_DAT_o_UART0};
    QL_RESERVED_BASE_ADDRESS [APERWIDTH-1:APERSIZE+2]: WBs_RD_DAT  <=    WBs_DAT_o_QL_Reserved  ;
    default:                                           WBs_RD_DAT  <=    DEFAULT_READ_VALUE     ;
    endcase
end
//------Instantiate Modules------------
//
// Define the FPGA I/O Pad Signals
//
// Note: Use the Constraint manager in SpDE to assign these buffers to FBIO pads.
//
// General FPGA Resources 
//
AL4S3B_FPGA_Registers #(

    .ADDRWIDTH                  ( ADDRWIDTH_FAB_REG             ),
    .DATAWIDTH                  ( DATAWIDTH_FAB_REG             ),

    .FPGA_REG_ID_VALUE_ADR      ( FPGA_REG_ID_VALUE_ADR         ),
    .FPGA_REV_NUM_ADR           ( FPGA_REV_NUM_ADR              ),

    .AL4S3B_DEVICE_ID           ( AL4S3B_DEVICE_ID              ),
    .AL4S3B_REV_LEVEL           ( AL4S3B_REV_LEVEL              ),
    .AL4S3B_SCRATCH_REG         ( AL4S3B_SCRATCH_REG            ),

    .AL4S3B_DEF_REG_VALUE       ( AL4S3B_DEF_REG_VALUE          )
                                                                )

     u_AL4S3B_FPGA_Registers 
                                ( 
    // AHB-To_FPGA Bridge I/F
    //
    .WBs_ADR_i                  ( WBs_ADR[ADDRWIDTH_FAB_REG+1:2] ),
    .WBs_CYC_i                  ( WBs_CYC_FPGA_Reg               ),
    .WBs_BYTE_STB_i             ( WBs_BYTE_STB                   ),
    .WBs_WE_i                   ( WBs_WE                         ),
    .WBs_STB_i                  ( WBs_STB                        ),
    .WBs_DAT_i                  ( WBs_WR_DAT                     ),
    .WBs_CLK_i                  ( WB_CLK                         ),
    .WBs_RST_i                  ( WB_RST                         ),
    .WBs_DAT_o                  ( WBs_DAT_o_FPGA_Reg             ),
    .WBs_ACK_o                  ( WBs_ACK_FPGA_Reg               ), 

    .dbg_reset_o                ( dbg_reset                      ), 

    .Device_ID_o                ( Device_ID_o                    )
    );


// UART 0
UART_16550 u_UART_16550_0      ( 
    // AHB-To_Fabric Bridge I/F
    .WBs_ADR_i                 ( WBs_ADR[5:2]                   ),
    .WBs_CYC_i                 ( WBs_CYC_UART0                  ),
    .WBs_WE_i                  ( WBs_WE                         ),
    .WBs_STB_i                 ( WBs_STB                        ),
    .WBs_DAT_i                 ( WBs_WR_DAT[7:0]                ),
    .WBs_CLK_i                 ( WB_CLK                         ),
    .WBs_RST_i                 ( WB_RST                         ),
    .WBs_DAT_o                 ( WBs_DAT_o_UART0                ),
    .WBs_ACK_o                 ( WBs_ACK_UART0                  ),

    .SIN_i                     ( UART0_SIN_i                    ),
    .SOUT_o                    ( UART0_SOUT_o                   ),

    .INTR_o                    ( UART0_Intr_o                   )
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

endmodule
