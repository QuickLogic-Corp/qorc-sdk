//////////////////////////////////////////////////////////////////////////////
// title          : Serial Transmit Bus Functional Model
// project        : Simulation BFM Development
//
// file           : Serial_Tx_monitor.v
// author         : Glen Gomes
// company        : QuickLogic Corp
// created        : 2016-03-05
// last update    : 2016-03-05
// platform       :
// standard       : Verilog
//
// description: Serial Receive Bus Functional Model
//
// copyright (c) 2016
//
// revisions  :
// date            version    author       description
// 2016-03-05      1.0        Glen Gomes   created
//
//////////////////////////////////////////////////////////////////////////////
`timescale 1ns/1ps

module Serial_Rx_monitor(

			    Rx                                ,
			    RTSn                              ,
			    CTSn                              ,

                Serial_Baud_Rate_parameter        ,
                Serial_Data_Bits_parameter        ,
                Serial_Parity_Bit_parameter       ,
                Serial_Stop_Bit_parameter         ,

                Rx_Baud_16x_Clk                       ,
                Rx_Capture_Trigger                ,

                Rx_Holding_Reg_Stop_Bit           ,
                Rx_Holding_Reg_Parity_Bit         ,
                Rx_Holding_Reg_Data_Bit           ,
                Rx_Holding_Reg_Start_Bit          ,
                Rx_Holding_Reg_Parity_Error_Flag  ,
                Rx_Holding_Reg_Framing_Error_Flag ,
                Rx_Holding_Reg_Break_Flag         ,
                Rx_Holding_Reg_False_Start_Flag

			    );
			

//------Port Parameters----------------
//

// Define Debug Message Controls
//
parameter       ENABLE_DEBUG_MSGS       =          1'b1;
parameter       ENABLE_DEBUG_ERROR_MSGS =          1'b1;

// Define the Standard Baud Rate Delays in nS
//
parameter       BAUD_RATE_PERIOD_110    =  9_090_909.09;
parameter       BAUD_RATE_PERIOD_300    =  3_333_333.33;
parameter       BAUD_RATE_PERIOD_600    =  1_666_666.67;
parameter       BAUD_RATE_PERIOD_1200   =    833_333.33;
parameter       BAUD_RATE_PERIOD_2400   =    416_666.67;
parameter       BAUD_RATE_PERIOD_4800   =    208_333.33;
parameter       BAUD_RATE_PERIOD_9600   =    104_166.67;
parameter       BAUD_RATE_PERIOD_14400  =     69_444.44;
parameter       BAUD_RATE_PERIOD_19200  =     52_083.33;
parameter       BAUD_RATE_PERIOD_38400  =     26_041.67;
parameter       BAUD_RATE_PERIOD_57600  =     17_361.11;
parameter       BAUD_RATE_PERIOD_115200 =      8_680.56;
parameter       BAUD_RATE_PERIOD_230400 =      4_340.28;
parameter       BAUD_RATE_PERIOD_921600 =      1_085.07;


parameter       SERIAL_DATA_5_BITS      =        2'b00 ;
parameter       SERIAL_DATA_6_BITS      =        2'b01 ;
parameter       SERIAL_DATA_7_BITS      =        2'b10 ;
parameter       SERIAL_DATA_8_BITS      =        2'b11 ;

parameter       SERIAL_PARITY_NONE      =        3'b000;
parameter       SERIAL_PARITY_ODD       =        3'b001;
parameter       SERIAL_PARITY_EVEN      =        3'b011;
parameter       SERIAL_PARITY_FORCE_1   =        3'b101;
parameter       SERIAL_PARITY_FORCE_0   =        3'b111;

parameter       SERIAL_STOP_1_BIT       =        1'b0  ;
parameter       SERIAL_STOP_1P5_BIT     =        1'b1  ;
parameter       SERIAL_STOP_2_BIT       =        1'b1  ;

parameter       SERIAL_BAUD_RATE_110    =        4'h0  ;
parameter       SERIAL_BAUD_RATE_300    =        4'h1  ;
parameter       SERIAL_BAUD_RATE_600    =        4'h2  ;
parameter       SERIAL_BAUD_RATE_1200   =        4'h3  ;
parameter       SERIAL_BAUD_RATE_2400   =        4'h4  ;
parameter       SERIAL_BAUD_RATE_4800   =        4'h5  ;
parameter       SERIAL_BAUD_RATE_9600   =        4'h6  ;
parameter       SERIAL_BAUD_RATE_14400  =        4'h7  ;
parameter       SERIAL_BAUD_RATE_19200  =        4'h8  ;
parameter       SERIAL_BAUD_RATE_38400  =        4'h9  ;
parameter       SERIAL_BAUD_RATE_57600  =        4'hA  ;
parameter       SERIAL_BAUD_RATE_115200 =        4'hB  ;
parameter       SERIAL_BAUD_RATE_230400 =        4'hC  ;
parameter       SERIAL_BAUD_RATE_921600 =        4'hD  ;


//------Port Signals-------------------
//
input           Rx                                ;
input           RTSn                              ;
output          CTSn                              ;

input   [3:0]   Serial_Baud_Rate_parameter        ;
input   [1:0]   Serial_Data_Bits_parameter        ;
input   [2:0]   Serial_Parity_Bit_parameter       ;
input           Serial_Stop_Bit_parameter         ;

output          Rx_Baud_16x_Clk                       ;
output          Rx_Capture_Trigger                ;

output  [1:0]   Rx_Holding_Reg_Stop_Bit           ;
output          Rx_Holding_Reg_Parity_Bit         ;
output  [7:0]   Rx_Holding_Reg_Data_Bit           ;
output          Rx_Holding_Reg_Start_Bit          ;
output          Rx_Holding_Reg_Parity_Error_Flag  ;
output          Rx_Holding_Reg_Framing_Error_Flag ;
output          Rx_Holding_Reg_Break_Flag         ;
output          Rx_Holding_Reg_False_Start_Flag   ;


wire		    Rx                                ;
wire		    RTSn                              ;
reg             CTSn                              ;

wire    [3:0]   Serial_Baud_Rate_parameter        ;
wire    [1:0]   Serial_Data_Bits_parameter        ;
wire    [2:0]   Serial_Parity_Bit_parameter       ;
wire            Serial_Stop_Bit_parameter         ;

reg             Rx_Baud_16x_Clk                       ;

reg             Rx_Capture_Trigger                ;
reg             Rx_Capture_Trigger_nxt            ;

wire    [1:0]   Rx_Holding_Reg_Stop_Bit           ;
wire            Rx_Holding_Reg_Parity_Bit         ;
wire    [7:0]   Rx_Holding_Reg_Data_Bit           ;
wire            Rx_Holding_Reg_Start_Bit          ;
wire            Rx_Holding_Reg_Parity_Error_Flag  ;
wire            Rx_Holding_Reg_Framing_Error_Flag ;
wire            Rx_Holding_Reg_Break_Flag         ;
wire            Rx_Holding_Reg_False_Start_Flag   ;


//------Define Parameters--------------
//

//
// Define the various operating modes
//

parameter       RX_BAUD_16X_CLK_RESET_LOOP  =    5     ;

parameter       RX_MON_IDLE             =        4'h0  ;
parameter       RX_MON_START            =        4'h1  ;
parameter       RX_MON_DATA_0           =        4'h2  ;
parameter       RX_MON_DATA_1           =        4'h3  ;
parameter       RX_MON_DATA_2           =        4'h4  ;
parameter       RX_MON_DATA_3           =        4'h5  ;
parameter       RX_MON_DATA_4           =        4'h6  ;
parameter       RX_MON_DATA_5           =        4'h7  ;
parameter       RX_MON_DATA_6           =        4'h8  ;
parameter       RX_MON_DATA_7           =        4'h9  ;
parameter       RX_MON_PARITY           =        4'ha  ;
parameter       RX_MON_STOP_1           =        4'hb  ;
parameter       RX_MON_STOP_2           =        4'hc  ;
parameter       RX_MON_SAVE             =        4'hd  ;


//------Internal Signals---------------
//

reg 		    Rx_1ff              ;
reg 		    Rx_2ff              ;
reg 		    Rx_3ff              ;

real            t_cycle_clk         ;

reg             Rx_Base_Rst         ;

integer         Rx_Baud_16x_Clk_Reset_Loop_Cnt;
integer         i, j, k, m, n       ;

reg             Rx_Start_Bit        ;
reg             Rx_Start_Bit_nxt    ;

reg     [7:0]   Rx_Data_Bit         ;
reg     [7:0]   Rx_Data_Bit_nxt     ;

reg             Rx_Parity_Bit       ;
reg             Rx_Parity_Bit_nxt   ;

reg     [1:0]   Rx_Stop_Bit         ;
reg     [1:0]   Rx_Stop_Bit_nxt     ;

reg     [3:0]   Rx_Bit_Count        ;
reg     [3:0]   Rx_Bit_Count_nxt    ;

reg     [3:0]   Rx_monitor_state    ;
reg     [3:0]   Rx_monitor_state_nxt;

reg    [15:0]   Rx_Holding_Reg      ;
reg    [15:0]   Rx_Holding_Reg_nxt  ;

real            Rx_Baud_Rate_msg    ;
integer         Rx_Data_Bit_msg     ;
reg     [2:0]   Rx_Parity_Bit_msg   ;
real            Rx_Stop_Bit_msg     ;

//	Define internal signals
//
reg	  [511:0]   Serial_Rx_monitor_msg1;  // Bus used for depositing test messages in ASCI
reg	  [511:0]   Serial_Rx_monitor_msg2;  // Bus used for depositing test messages in ASCI
reg	  [511:0]   Serial_Rx_monitor_msg3;  // Bus used for depositing test messages in ASCI
reg	  [511:0]   Serial_Rx_monitor_msg4;  // Bus used for depositing test messages in ASCI
reg   [511:0]   Serial_Rx_monitor_msg5;  // Bus used for depositing test messages in ASCI



//------Logic Operations---------------
//

// Simplify the Reading of the Rx Holding Register
//
assign  Rx_Holding_Reg_False_Start_Flag   =  Rx_Holding_Reg[15]   ;
assign  Rx_Holding_Reg_Break_Flag         =  Rx_Holding_Reg[14]   ;
assign  Rx_Holding_Reg_Framing_Error_Flag =  Rx_Holding_Reg[13]   ;
assign  Rx_Holding_Reg_Parity_Error_Flag  =  Rx_Holding_Reg[12]   ;
assign  Rx_Holding_Reg_Stop_Bit           =  Rx_Holding_Reg[11:10];
assign  Rx_Holding_Reg_Parity_Bit         =  Rx_Holding_Reg[9]    ;
assign  Rx_Holding_Reg_Data_Bit           =  Rx_Holding_Reg[8:1]  ;
assign  Rx_Holding_Reg_Start_Bit          =  Rx_Holding_Reg[0]    ;


// Define the intial state of key signals
//
initial
begin

    CTSn <= 1'b1;


	Serial_Rx_monitor_msg1 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
	Serial_Rx_monitor_msg2 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
	Serial_Rx_monitor_msg3 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
	Serial_Rx_monitor_msg4 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
    Serial_Rx_monitor_msg5 <= "NO ACTIVITY";        // Bus used for depositiog test messages in ASCI
end

  
// Define the clock oscillator section
//
initial
begin
	Rx_Baud_16x_Clk	= 0;	// Intialize the clock at time 0ns.

	forever				// Generate a clock with an expected frequency.
	begin
        case(Serial_Baud_Rate_parameter)
        SERIAL_BAUD_RATE_110    :  t_cycle_clk = (BAUD_RATE_PERIOD_110   /(2 * 16));
        SERIAL_BAUD_RATE_300    :  t_cycle_clk = (BAUD_RATE_PERIOD_300   /(2 * 16));
        SERIAL_BAUD_RATE_600    :  t_cycle_clk = (BAUD_RATE_PERIOD_600   /(2 * 16));
        SERIAL_BAUD_RATE_1200   :  t_cycle_clk = (BAUD_RATE_PERIOD_1200  /(2 * 16));
        SERIAL_BAUD_RATE_2400   :  t_cycle_clk = (BAUD_RATE_PERIOD_2400  /(2 * 16));
        SERIAL_BAUD_RATE_4800   :  t_cycle_clk = (BAUD_RATE_PERIOD_4800  /(2 * 16));
        SERIAL_BAUD_RATE_9600   :  t_cycle_clk = (BAUD_RATE_PERIOD_9600  /(2 * 16));
        SERIAL_BAUD_RATE_14400  :  t_cycle_clk = (BAUD_RATE_PERIOD_14400 /(2 * 16));
        SERIAL_BAUD_RATE_19200  :  t_cycle_clk = (BAUD_RATE_PERIOD_19200 /(2 * 16));
        SERIAL_BAUD_RATE_38400  :  t_cycle_clk = (BAUD_RATE_PERIOD_38400 /(2 * 16));
        SERIAL_BAUD_RATE_57600  :  t_cycle_clk = (BAUD_RATE_PERIOD_57600 /(2 * 16));
        SERIAL_BAUD_RATE_115200 :  t_cycle_clk = (BAUD_RATE_PERIOD_115200/(2 * 16));
        SERIAL_BAUD_RATE_230400 :  t_cycle_clk = (BAUD_RATE_PERIOD_230400/(2 * 16));
        SERIAL_BAUD_RATE_921600 :  t_cycle_clk = (BAUD_RATE_PERIOD_921600/(2 * 16));
        endcase

		#(t_cycle_clk) Rx_Baud_16x_Clk = 1;
		#(t_cycle_clk) Rx_Baud_16x_Clk = 0;
	end 
end


// Define the Rx monitor's reset
//
// Note: The monitor runs asynchronous to other IP's
//
initial
begin

    Rx_Base_Rst  <= 1'b1;

    for (Rx_Baud_16x_Clk_Reset_Loop_Cnt = 0; 
	     Rx_Baud_16x_Clk_Reset_Loop_Cnt < RX_BAUD_16X_CLK_RESET_LOOP; 
         Rx_Baud_16x_Clk_Reset_Loop_Cnt = Rx_Baud_16x_Clk_Reset_Loop_Cnt + 1)
    begin
        wait (Rx_Baud_16x_Clk == 1'b1) #1;
        wait (Rx_Baud_16x_Clk == 1'b0) #1;
    end

    wait (Rx_Baud_16x_Clk == 1'b1) #1;

    Rx_Base_Rst  <= 1'b0;

end

always @(posedge Rx_Baud_16x_Clk or posedge Rx_Base_Rst)
begin
    if (Rx_Base_Rst)
    begin
        Rx_1ff             <=  1'b1;
        Rx_2ff             <=  1'b1;
        Rx_3ff             <=  1'b1;

        Rx_monitor_state   <= RX_MON_IDLE;

        Rx_Start_Bit       <=  1'b1;
        Rx_Data_Bit        <=  8'h0;
        Rx_Parity_Bit      <=  1'b0;
        Rx_Stop_Bit        <=  2'h0;

        Rx_Holding_Reg     <= 12'h0;

		Rx_Bit_Count       <=  4'h8;

        Rx_Capture_Trigger <= 1'b0 ;
    end
    else
    begin
        Rx_1ff             <= Rx                    ;
        Rx_2ff             <= Rx_1ff                ;
        Rx_3ff             <= Rx_2ff                ;

        Rx_monitor_state   <= Rx_monitor_state_nxt  ;

        Rx_Start_Bit       <= Rx_Start_Bit_nxt      ;
        Rx_Data_Bit        <= Rx_Data_Bit_nxt       ;
        Rx_Parity_Bit      <= Rx_Parity_Bit_nxt     ;
        Rx_Stop_Bit        <= Rx_Stop_Bit_nxt       ;

        Rx_Holding_Reg     <= Rx_Holding_Reg_nxt    ;

		Rx_Bit_Count       <= Rx_Bit_Count_nxt      ;

        Rx_Capture_Trigger <= Rx_Capture_Trigger_nxt;
    end
end


// Print the monitor's main message(s)
//
always @(posedge Rx_Capture_Trigger)
begin

    if (ENABLE_DEBUG_MSGS)
    begin
            
        if (Rx_Holding_Reg_False_Start_Flag)
        begin
                                      $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, No Parity, and %0.1f Stop bits detected a False Start Condition at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                                     $realtime );
        end
        else if (Rx_Holding_Reg_Break_Flag)
        begin
                                      $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, No Parity, and %0.1f Stop bits detected a Break Condition at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                                     $realtime );
        end
        else
        begin
            case(Rx_Parity_Bit_msg)
            SERIAL_PARITY_NONE      : $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, No Parity, and %0.1f Stop bits detected a value of 0x%0x at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                       Rx_Holding_Reg_Data_Bit,
                                                                                                                                                                                     $realtime );
            SERIAL_PARITY_ODD       : $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, Odd Parity, and %0.1f Stop bits detected a value of 0x%0x at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                       Rx_Holding_Reg_Data_Bit,
                                                                                                                                                                                     $realtime );
            SERIAL_PARITY_EVEN      : $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, Even Parity, and %0.1f Stop bits detected a value of 0x%0x at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                       Rx_Holding_Reg_Data_Bit,
                                                                                                                                                                                     $realtime );
            SERIAL_PARITY_FORCE_1   : $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, Force 1 Parity, and %0.1f Stop bits detected a value of 0x%0x at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                       Rx_Holding_Reg_Data_Bit,
                                                                                                                                                                                     $realtime );
            SERIAL_PARITY_FORCE_0   : $display("  Serial Rx Bus Monitor:  Using Comm. settings of %0d Baud, %0d Data Bits, Force 0 Parity, and %0.1f Stop bits detected a value of 0x%0x at time %0t ", 
                                                                                                                                                                              Rx_Baud_Rate_msg, 
                                                                                                                                                                               Rx_Data_Bit_msg, 
                                                                                                                                                                               Rx_Stop_Bit_msg, 
                                                                                                                                                                       Rx_Holding_Reg_Data_Bit,
                                                                                                                                                                                     $realtime );
            endcase
        end

        if (ENABLE_DEBUG_ERROR_MSGS)
        begin
            case({ Rx_Holding_Reg_Framing_Error_Flag,  Rx_Holding_Reg_Parity_Error_Flag })
            2'b00                   : $display("  Serial Rx Bus Monitor:  No  Framing Error and No  Parity Error detected at time %0t ", $realtime );
            2'b01                   : $display("  Serial Rx Bus Monitor:  No  Framing Error and Yes Parity Error detected at time %0t ", $realtime );
            2'b10                   : $display("  Serial Rx Bus Monitor:  Yes Framing Error and No  Parity Error detected at time %0t ", $realtime );
            2'b11                   : $display("  Serial Rx Bus Monitor:  Yes Framing Error and Yes Parity Error detected at time %0t ", $realtime );
            endcase
        end
    end
end


// Monitor the Serial Data Stream and capture it when available.
//
// Note: The statemachine does not expect a false start condition
//
always @(Rx_monitor_state            or
         Rx_2ff                      or
         Rx_3ff                      or
         Rx                          or
         Rx_Start_Bit                or
         Rx_Data_Bit                 or
         Rx_Parity_Bit               or
         Rx_Stop_Bit                 or
         Rx_Holding_Reg              or
         Serial_Data_Bits_parameter  or
         Serial_Parity_Bit_parameter or
         Serial_Stop_Bit_parameter   or
         Rx_Bit_Count
         )
begin
    case(Rx_monitor_state)
    RX_MON_IDLE    : // Sense for the "Start" of the serial stream
    begin
	    Serial_Rx_monitor_msg1   <= "Waiting for Serial Data" ;
	    Serial_Rx_monitor_msg2   <= "NO ACTIVITY"             ;
	    Serial_Rx_monitor_msg3   <= "In IDLE State"           ;

        Rx_Start_Bit_nxt         <= 1'b0               ;
        Rx_Data_Bit_nxt          <= 8'h0               ;
        Rx_Parity_Bit_nxt        <= 1'b0               ;
        Rx_Stop_Bit_nxt          <= 2'b0               ;
		Rx_Bit_Count_nxt         <= 4'h7               ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case({Rx_2ff, Rx_3ff})
        2'b00: Rx_monitor_state_nxt <= RX_MON_IDLE     ;
        2'b01: Rx_monitor_state_nxt <= RX_MON_START    ;
        2'b10: Rx_monitor_state_nxt <= RX_MON_IDLE     ;
        2'b11: Rx_monitor_state_nxt <= RX_MON_IDLE     ;
        endcase
    end
	RX_MON_START   : // This state samples the "Start" bit at the beginning of all serial stream formats
    begin
	    Serial_Rx_monitor_msg1   <= "Sampling  Serial Stream"  ;
	    Serial_Rx_monitor_msg2   <= "Capturing Start Bit"      ;
	    Serial_Rx_monitor_msg3   <= "In Start State"           ;

        Rx_Data_Bit_nxt          <= Rx_Data_Bit          ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit        ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit          ;

        Rx_Holding_Reg_nxt[14:0] <= Rx_Holding_Reg[14:0] ;

        Rx_Capture_Trigger_nxt   <= 1'b0                 ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_0        ;

			case(Rx_2ff)
            1'b0:
            begin
                Rx_monitor_state_nxt   <= RX_MON_DATA_0  ;
                Rx_Start_Bit_nxt       <= Rx_2ff         ;
		        Rx_Bit_Count_nxt       <= 4'hf           ;
                Rx_Holding_Reg_nxt[15] <= 1'b0           ;
                Rx_Capture_Trigger_nxt <= 1'b0           ;
            end
            1'b1:
            begin
                Rx_monitor_state_nxt   <= RX_MON_IDLE    ; // False "Start" bit on the bus
                Rx_Start_Bit_nxt       <= 1'b0           ;
		        Rx_Bit_Count_nxt       <= 4'h7           ;
                Rx_Holding_Reg_nxt[15] <= 1'b1           ;
                Rx_Capture_Trigger_nxt <= 1'b1           ;
            end
            endcase
        end
        default:
        begin
            Rx_monitor_state_nxt     <= RX_MON_START     ;

            Rx_Start_Bit_nxt         <= Rx_Start_Bit     ;
            Rx_Holding_Reg_nxt[15]   <= 1'b1             ;
            Rx_Capture_Trigger_nxt   <= 1'b0             ;

		    Rx_Bit_Count_nxt         <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_0  : // This state samples bit [0] of all serial stream formats
    begin
	    Serial_Rx_monitor_msg2   <= "Capturing Data Bits";
	    Serial_Rx_monitor_msg3   <= "In Data 0 State"    ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_1      ;

            Rx_Data_Bit_nxt[0]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7:1] <= Rx_Data_Bit[7:1]   ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_0      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_1  : // This state samples bit [1] of all serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 1 State"  ;
        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_2      ;

            Rx_Data_Bit_nxt[0]   <= Rx_Data_Bit[0]     ;
            Rx_Data_Bit_nxt[1]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7:2] <= Rx_Data_Bit[7:2]   ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_1      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_2  : // This state samples bit [2] of all serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 2 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_3      ;

            Rx_Data_Bit_nxt[1:0] <= Rx_Data_Bit[1:0]   ;
            Rx_Data_Bit_nxt[2]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7:3] <= Rx_Data_Bit[7:3]   ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_2      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_3  : // This state samples bit [3] of all serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 3 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_4      ;

            Rx_Data_Bit_nxt[2:0] <= Rx_Data_Bit[2:0]   ;
            Rx_Data_Bit_nxt[3]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7:4] <= Rx_Data_Bit[7:4]   ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_3      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_4  : // This state provides the sample for 5, 6, 7, and 8-bit serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 4 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            case(Serial_Data_Bits_parameter)
            SERIAL_DATA_5_BITS :
            begin
                if (Serial_Parity_Bit_parameter == SERIAL_PARITY_NONE)
                     Rx_monitor_state_nxt <= RX_MON_STOP_1 ;
                else
                     Rx_monitor_state_nxt <= RX_MON_PARITY ;
            end
            default: Rx_monitor_state_nxt <= RX_MON_DATA_5 ;
            endcase

            Rx_Data_Bit_nxt[3:0] <= Rx_Data_Bit[3:0]   ;
            Rx_Data_Bit_nxt[4]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7:5] <= Rx_Data_Bit[7:5]   ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_4      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_5  : // This state provides the sample for 6, 7, and 8-bit serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 5 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            case(Serial_Data_Bits_parameter)
            SERIAL_DATA_6_BITS :
            begin
                if (Serial_Parity_Bit_parameter == SERIAL_PARITY_NONE)
                     Rx_monitor_state_nxt <= RX_MON_STOP_1 ;
                else
                     Rx_monitor_state_nxt <= RX_MON_PARITY ;
            end
            default: Rx_monitor_state_nxt <= RX_MON_DATA_6 ;
            endcase

            Rx_Data_Bit_nxt[4:0] <= Rx_Data_Bit[4:0]   ;
            Rx_Data_Bit_nxt[5]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7:6] <= Rx_Data_Bit[7:6]   ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_5      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_6  : // This state provides the sample for 7 and 8-bit serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 6 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            case(Serial_Data_Bits_parameter)
            SERIAL_DATA_7_BITS :
            begin
                if (Serial_Parity_Bit_parameter == SERIAL_PARITY_NONE)
                     Rx_monitor_state_nxt <= RX_MON_STOP_1 ;
                else
                     Rx_monitor_state_nxt <= RX_MON_PARITY ;
            end
            default: Rx_monitor_state_nxt <= RX_MON_DATA_7 ;
            endcase

            Rx_Data_Bit_nxt[5:0] <= Rx_Data_Bit[5:0]   ;
            Rx_Data_Bit_nxt[6]   <= Rx_2ff             ;
            Rx_Data_Bit_nxt[7]   <= Rx_Data_Bit[7]     ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_6      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_DATA_7  : // This state provides the sample for 8-bit serial serial stream formats
    begin
	    Serial_Rx_monitor_msg3   <= "In Data 7 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            if (Serial_Parity_Bit_parameter == SERIAL_PARITY_NONE)
                Rx_monitor_state_nxt <= RX_MON_STOP_1  ;
            else
                Rx_monitor_state_nxt <= RX_MON_PARITY  ;

            Rx_Data_Bit_nxt[6:0] <= Rx_Data_Bit[6:0]   ;
            Rx_Data_Bit_nxt[7]   <= Rx_2ff             ;

		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_DATA_7      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_PARITY  : // This state provides the sampling of the Parity bit.
    begin
	    Serial_Rx_monitor_msg2   <= "Capturing Parity Bit" ;
	    Serial_Rx_monitor_msg3   <= "In Parity State"      ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Data_Bit_nxt          <= Rx_Data_Bit        ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            Rx_monitor_state_nxt <= RX_MON_STOP_1      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;
            Rx_Parity_Bit_nxt    <= Rx_2ff             ;
            Rx_Stop_Bit_nxt      <= 2'h0               ;
      
		    Rx_Bit_Count_nxt     <= 4'hf               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_PARITY      ;

            Rx_Data_Bit_nxt      <= Rx_Data_Bit        ;
            Rx_Parity_Bit_nxt    <= Rx_Parity_Bit      ;
            Rx_Stop_Bit_nxt      <= Rx_Stop_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_STOP_1  : // This state provides the sampling for the first stop bit; all serial streams have at least one.
    begin
	    Serial_Rx_monitor_msg2   <= "Capturing Stop Bits"  ;
	    Serial_Rx_monitor_msg3   <= "In Stop 1 State"      ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Data_Bit_nxt          <= Rx_Data_Bit        ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h0:
        begin
            if (Serial_Stop_Bit_parameter == SERIAL_STOP_1_BIT)
                Rx_monitor_state_nxt <= RX_MON_SAVE    ;
            else
                Rx_monitor_state_nxt <= RX_MON_STOP_2  ;

            Rx_Stop_Bit_nxt[0]   <= Rx_2ff             ;
            Rx_Stop_Bit_nxt[1]   <= Rx_Stop_Bit[1]     ;

            if (Serial_Stop_Bit_parameter === SERIAL_STOP_1_BIT)
            begin
		        Rx_Bit_Count_nxt     <= 4'h7           ;
            end
            else
            begin
                if (Serial_Data_Bits_parameter === SERIAL_DATA_5_BITS)
		            Rx_Bit_Count_nxt <= 4'hb           ; // 1.5 Stop Bits
                else
		            Rx_Bit_Count_nxt <= 4'hf           ; // 2.0 Stop Bits
            end
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_STOP_1      ;

            Rx_Stop_Bit_nxt      <= Rx_Stop_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count - 1'b1;
        end
        endcase
    end
    RX_MON_STOP_2  : // This state provides the sampling for both 1.5 and 2.0 stop bits.
    begin
	    Serial_Rx_monitor_msg3   <= "In Stop 2 State"  ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Data_Bit_nxt          <= Rx_Data_Bit        ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;

        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;

        Rx_Capture_Trigger_nxt   <= 1'b0               ;

        case(Rx_Bit_Count)
        4'h7:
        begin
            Rx_monitor_state_nxt <= RX_MON_SAVE        ;

            Rx_Stop_Bit_nxt[0]   <= Rx_Stop_Bit[0]     ;
            Rx_Stop_Bit_nxt[1]   <= Rx_2ff             ;

		    Rx_Bit_Count_nxt     <= 4'h7               ;
        end
        default:
        begin
            Rx_monitor_state_nxt <= RX_MON_STOP_2      ;
     
            Rx_Stop_Bit_nxt      <= Rx_Stop_Bit        ;

		    Rx_Bit_Count_nxt     <= Rx_Bit_Count + 1'b1;
        end
        endcase
    end
    RX_MON_SAVE    :  // This state provides a convient way to save the results of the serial stream
    begin
	    Serial_Rx_monitor_msg2   <= "Saving Rx Data"   ;
	    Serial_Rx_monitor_msg3   <= "In Save State"    ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Data_Bit_nxt          <= Rx_Data_Bit        ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;
		Rx_Bit_Count_nxt         <= 4'h7               ;

        // Capture the current transfer settings with the captured data.
		//
        case(Serial_Baud_Rate_parameter)
        SERIAL_BAUD_RATE_110    :  Rx_Baud_Rate_msg <=    110;
        SERIAL_BAUD_RATE_300    :  Rx_Baud_Rate_msg <=    300;
        SERIAL_BAUD_RATE_600    :  Rx_Baud_Rate_msg <=    600;
        SERIAL_BAUD_RATE_1200   :  Rx_Baud_Rate_msg <=   1200;
        SERIAL_BAUD_RATE_2400   :  Rx_Baud_Rate_msg <=   2400;
        SERIAL_BAUD_RATE_4800   :  Rx_Baud_Rate_msg <=   4800;
        SERIAL_BAUD_RATE_9600   :  Rx_Baud_Rate_msg <=   9600;
        SERIAL_BAUD_RATE_14400  :  Rx_Baud_Rate_msg <=  14400;
        SERIAL_BAUD_RATE_19200  :  Rx_Baud_Rate_msg <=  19200;
        SERIAL_BAUD_RATE_38400  :  Rx_Baud_Rate_msg <=  38400;
        SERIAL_BAUD_RATE_57600  :  Rx_Baud_Rate_msg <=  57600;
        SERIAL_BAUD_RATE_115200 :  Rx_Baud_Rate_msg <= 115200;
        SERIAL_BAUD_RATE_230400 :  Rx_Baud_Rate_msg <= 230400;
        SERIAL_BAUD_RATE_921600 :  Rx_Baud_Rate_msg <= 921600;
        endcase

        case(Serial_Data_Bits_parameter)
        SERIAL_DATA_5_BITS      :  
        begin
                                   Rx_Data_Bit_msg  <= 5     ;
		    case(Serial_Stop_Bit_parameter)
            SERIAL_STOP_1_BIT   :  Rx_Stop_Bit_msg  <= 1.0   ;
            SERIAL_STOP_1P5_BIT :  Rx_Stop_Bit_msg  <= 1.5   ;
            endcase
        end
        SERIAL_DATA_6_BITS      :  
        begin
                                   Rx_Data_Bit_msg  <= 6     ;
		    case(Serial_Stop_Bit_parameter)
            SERIAL_STOP_1_BIT   :  Rx_Stop_Bit_msg  <= 1.0   ;
            SERIAL_STOP_2_BIT   :  Rx_Stop_Bit_msg  <= 2.0   ;
            endcase
        end
        SERIAL_DATA_7_BITS      :  
        begin
                                   Rx_Data_Bit_msg  <= 7     ;
		    case(Serial_Stop_Bit_parameter)
            SERIAL_STOP_1_BIT   :  Rx_Stop_Bit_msg  <= 1.0   ;
            SERIAL_STOP_2_BIT   :  Rx_Stop_Bit_msg  <= 2.0   ;
            endcase
        end
        SERIAL_DATA_8_BITS      :  
        begin
                                   Rx_Data_Bit_msg  <= 8     ;
		    case(Serial_Stop_Bit_parameter)
            SERIAL_STOP_1_BIT   :  Rx_Stop_Bit_msg  <= 1.0   ;
            SERIAL_STOP_2_BIT   :  Rx_Stop_Bit_msg  <= 2.0   ;
            endcase
        end
        endcase

        Rx_Parity_Bit_msg        <= Serial_Parity_Bit_parameter;

        Rx_Capture_Trigger_nxt   <= 1'b1                     ;

        // Check for Break Condition before declaring framing or parity errors
        //
        // Note: The unused bits in each format are already set to "0" at the
        //       beginning of the transfer monitoring; therefore, checking for
		//       all zeros is acceptable.
        //
        if (!((Rx_Stop_Bit   === 2'h0) &&
              (Rx_Parity_Bit === 1'b0) &&
              (Rx_Data_Bit   === 8'h0) &&
              (Rx_Start_Bit  === 1'b0)   ))
        begin
            Rx_Holding_Reg_nxt[14] <= 1'b0; // No "Break" condition

		    case(Serial_Stop_Bit_parameter)
            SERIAL_STOP_1_BIT       : Rx_Holding_Reg_nxt[13] <=    Rx_Stop_Bit[0]                      ? 1'b0 : 1'b1;
            SERIAL_STOP_1P5_BIT     : Rx_Holding_Reg_nxt[13] <= (  Rx_Stop_Bit[1:0] == 2'b11)          ? 1'b0 : 1'b1;
            SERIAL_STOP_2_BIT       : Rx_Holding_Reg_nxt[13] <= (  Rx_Stop_Bit[1:0] == 2'b11)          ? 1'b0 : 1'b1;
            endcase

		    case(Serial_Parity_Bit_parameter)
            SERIAL_PARITY_NONE      : Rx_Holding_Reg_nxt[12] <= 1'b0                                                ;
            SERIAL_PARITY_ODD       :  
            begin
                case(Serial_Data_Bits_parameter)
                SERIAL_DATA_5_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[4:0]) == Rx_Parity_Bit) ? 1'b0 : 1'b1;
                SERIAL_DATA_6_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[5:0]) == Rx_Parity_Bit) ? 1'b0 : 1'b1;
                SERIAL_DATA_7_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[6:0]) == Rx_Parity_Bit) ? 1'b0 : 1'b1;
                SERIAL_DATA_8_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[7:0]) == Rx_Parity_Bit) ? 1'b0 : 1'b1;
                endcase
            end
            SERIAL_PARITY_EVEN      :  
            begin
                case(Serial_Data_Bits_parameter)
                SERIAL_DATA_5_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[4:0]) == Rx_Parity_Bit) ? 1'b1 : 1'b0;
                SERIAL_DATA_6_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[5:0]) == Rx_Parity_Bit) ? 1'b1 : 1'b0;
                SERIAL_DATA_7_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[6:0]) == Rx_Parity_Bit) ? 1'b1 : 1'b0;
                SERIAL_DATA_8_BITS  : Rx_Holding_Reg_nxt[12] <= ((^Rx_Data_Bit[7:0]) == Rx_Parity_Bit) ? 1'b1 : 1'b0;
                endcase
            end
            SERIAL_PARITY_FORCE_1   : Rx_Holding_Reg_nxt[12] <=    Rx_Parity_Bit                       ? 1'b0 : 1'b1;
            SERIAL_PARITY_FORCE_0   : Rx_Holding_Reg_nxt[12] <=    Rx_Parity_Bit                       ? 1'b1 : 1'b0;
            endcase
        end
        else
        begin
            Rx_Holding_Reg_nxt[14] <= 1'b1             ;
            Rx_Holding_Reg_nxt[13] <= 1'b0             ;
            Rx_Holding_Reg_nxt[12] <= 1'b0             ;
        end

        Rx_Holding_Reg_nxt[11:0] <= {Rx_Stop_Bit       ,
                                     Rx_Parity_Bit     ,
                                     Rx_Data_Bit       ,
                                     Rx_Start_Bit     };

        Rx_monitor_state_nxt     <= RX_MON_IDLE        ;
    end
	default:
    begin
	    Serial_Rx_monitor_msg1   <= "NO ACTIVITY"      ;
	    Serial_Rx_monitor_msg2   <= "NO ACTIVITY"      ;
	    Serial_Rx_monitor_msg3   <= "NO ACTIVITY"      ;

        Rx_Start_Bit_nxt         <= Rx_Start_Bit       ;
        Rx_Data_Bit_nxt          <= Rx_Data_Bit        ;
        Rx_Parity_Bit_nxt        <= Rx_Parity_Bit      ;
        Rx_Stop_Bit_nxt          <= Rx_Stop_Bit        ;
		Rx_Bit_Count_nxt         <= 4'h7               ;
        Rx_Holding_Reg_nxt       <= Rx_Holding_Reg     ;
        Rx_Capture_Trigger_nxt   <= 1'b0               ;
        Rx_monitor_state_nxt     <= RX_MON_IDLE        ;
    end
    endcase
end

endmodule
