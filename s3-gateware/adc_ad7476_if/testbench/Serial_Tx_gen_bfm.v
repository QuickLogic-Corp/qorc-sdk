//////////////////////////////////////////////////////////////////////////////
// title          : Serial Transmit Bus Functional Model
// project        : Simulation BFM Development
//
// file           : Serial_Tx_gen_bfm.v
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

module Serial_Tx_gen_bfm(

			Tx,
			RTSn,
			CTSn

			);
			

//------Port Parameters----------------
//

// Define Debug Message Controls
//
parameter   ENABLE_REG_WR_DEBUG_MSG =          1'b0;
parameter   ENABLE_REG_RD_DEBUG_MSG =          1'b0;

// Define the Standard Baud Rate Delays in nS
//
parameter   BAUD_RATE_PERIOD_110    =  9_090_909.09;
parameter   BAUD_RATE_PERIOD_300    =  3_333_333.33;
parameter   BAUD_RATE_PERIOD_600    =  1_666_666.67;
parameter   BAUD_RATE_PERIOD_1200   =    833_333.33;
parameter   BAUD_RATE_PERIOD_2400   =    416_666.67;
parameter   BAUD_RATE_PERIOD_4800   =    208_333.33;
parameter   BAUD_RATE_PERIOD_9600   =    104_166.67;
parameter   BAUD_RATE_PERIOD_14400  =     69_444.44;
parameter   BAUD_RATE_PERIOD_19200  =     52_083.33;
parameter   BAUD_RATE_PERIOD_38400  =     26_041.67;
parameter   BAUD_RATE_PERIOD_57600  =     17_361.11;
parameter   BAUD_RATE_PERIOD_115200 =      8_680.56;
parameter   BAUD_RATE_PERIOD_230400 =      4_340.28;
parameter   BAUD_RATE_PERIOD_921600 =      1_085.07;


parameter   SERIAL_DATA_5_BITS      =        2'b00 ;
parameter   SERIAL_DATA_6_BITS      =        2'b01 ;
parameter   SERIAL_DATA_7_BITS      =        2'b10 ;
parameter   SERIAL_DATA_8_BITS      =        2'b11 ;

parameter   SERIAL_PARITY_NONE      =        3'b000;
parameter   SERIAL_PARITY_ODD       =        3'b001;
parameter   SERIAL_PARITY_EVEN      =        3'b011;
parameter   SERIAL_PARITY_FORCE_1   =        3'b101;
parameter   SERIAL_PARITY_FORCE_0   =        3'b111;

parameter   SERIAL_STOP_1_BIT       =        1'b0  ;
parameter   SERIAL_STOP_1P5_BIT     =        1'b1  ;
parameter   SERIAL_STOP_2_BIT       =        1'b1  ;

parameter   SERIAL_BAUD_RATE_110    =        4'h0  ;
parameter   SERIAL_BAUD_RATE_300    =        4'h1  ;
parameter   SERIAL_BAUD_RATE_600    =        4'h2  ;
parameter   SERIAL_BAUD_RATE_1200   =        4'h3  ;
parameter   SERIAL_BAUD_RATE_2400   =        4'h4  ;
parameter   SERIAL_BAUD_RATE_4800   =        4'h5  ;
parameter   SERIAL_BAUD_RATE_9600   =        4'h6  ;
parameter   SERIAL_BAUD_RATE_14400  =        4'h7  ;
parameter   SERIAL_BAUD_RATE_19200  =        4'h8  ;
parameter   SERIAL_BAUD_RATE_38400  =        4'h9  ;
parameter   SERIAL_BAUD_RATE_57600  =        4'hA  ;
parameter   SERIAL_BAUD_RATE_115200 =        4'hB  ;
parameter   SERIAL_BAUD_RATE_230400 =        4'hC  ;
parameter   SERIAL_BAUD_RATE_921600 =        4'hD  ;


//------Port Signals-------------------
//
output      Tx;
input       RTSn;
output      CTSn;

reg 		Tx;
wire		RTSn;
reg         CTSn;


//------Define Parameters--------------
//

//
// Define the various operating modes
//



//------Internal Signals---------------
//

//	Define internal signals
//
reg	  [511:0]   Serial_Tx_gen_bfm_msg1;  // Bus used for depositing test messages in ASCI
reg	  [511:0]   Serial_Tx_gen_bfm_msg2;  // Bus used for depositing test messages in ASCI
reg	  [511:0]   Serial_Tx_gen_bfm_msg3;  // Bus used for depositing test messages in ASCI
reg	  [511:0]   Serial_Tx_gen_bfm_msg4;  // Bus used for depositing test messages in ASCI
reg   [511:0]   Serial_Tx_gen_bfm_msg5;  // Bus used for depositing test messages in ASCI



//------Logic Operations---------------
//

// Define the intial state of key signals
//
initial
begin

    Tx   <= 1'b1;
    CTSn <= 1'b1;


	Serial_Tx_gen_bfm_msg1 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
	Serial_Tx_gen_bfm_msg2 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
	Serial_Tx_gen_bfm_msg3 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
	Serial_Tx_gen_bfm_msg4 <= "NO ACTIVITY";		// Bus used for depositing test messages in ASCI
    Serial_Tx_gen_bfm_msg5 <= "NO ACTIVITY";        // Bus used for depositiog test messages in ASCI
end

  


// Define a task that will read from a Sensor Register
//
task serial_tx_write_data;

input   [1:0]	SERIAL_DATA   ;
input   [2:0]	SERIAL_PARITY ;
input        	SERIAL_STOP   ;
input   [3:0]   BAUD_RATE     ;

input   [7:0]   TARGET_DATA   ;


real            expected_xfr_bit_time;

integer         expected_number_of_data_bits;

reg             expected_parity_odd;
reg             expected_parity;

integer         i;

begin

	// Issue a diagnostic Message
	//

    case(BAUD_RATE)
    SERIAL_BAUD_RATE_110    : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 110 Baud"     ;
    SERIAL_BAUD_RATE_300    : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 300 Baud"     ;
    SERIAL_BAUD_RATE_600    : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 600 Baud"     ;
    SERIAL_BAUD_RATE_1200   : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 1200 Baud"    ;
    SERIAL_BAUD_RATE_2400   : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 2400 Baud"    ;
    SERIAL_BAUD_RATE_4800   : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 4800 Baud"    ;
    SERIAL_BAUD_RATE_9600   : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 9600 Baud"    ;
    SERIAL_BAUD_RATE_14400  : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 14,400 Baud"  ;
    SERIAL_BAUD_RATE_19200  : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 19,200 Baud"  ;
    SERIAL_BAUD_RATE_38400  : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 38,400 Baud"  ;
    SERIAL_BAUD_RATE_57600  : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 57,600 Baud"  ;
    SERIAL_BAUD_RATE_115200 : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 115,200 Baud" ;
    SERIAL_BAUD_RATE_230400 : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 230,400 Baud" ;
    SERIAL_BAUD_RATE_921600 : Serial_Tx_gen_bfm_msg1 = "Serial Tx Single Transfer at 921,600 Baud" ;
    endcase


	// Generate the diagnostic message for each transfer type
	//

    case({SERIAL_PARITY[0], SERIAL_STOP, SERIAL_DATA})
    4'b0000:                    Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, No Parity, 1   Stop bit"       ; // Start, 5 bits,         Stop bits -> 1
    4'b0001:                    Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, No Parity, 1   Stop bit"       ; // Start, 6 bits,         Stop bits -> 1
    4'b0010:                    Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, No Parity, 1   Stop bit"       ; // Start, 7 bits,         Stop bits -> 1
    4'b0011:                    Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, No Parity, 1   Stop bit"       ; // Start, 8 bits,         Stop bits -> 1

    4'b0100:                    Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, No Parity, 1.5 Stop bits"      ; // Start, 5 bits,         Stop bits -> 1.5
    4'b0101:                    Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, No Parity, 2   Stop bits"      ; // Start, 6 bits,         Stop bits -> 2
    4'b0110:                    Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, No Parity, 2   Stop bits"      ; // Start, 7 bits,         Stop bits -> 2
    4'b0111:                    Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, No Parity, 2   Stop bits"      ; // Start, 8 bits,         Stop bits -> 2

    4'b1000: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Odd     Parity, 1 Stop bit"    ; // Start, 5 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Even    Parity, 1 Stop bit"    ; // Start, 5 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Force 1 Parity, 1 Stop bit"    ; // Start, 5 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Force 0 Parity, 1 Stop bit"    ; // Start, 5 bits, Parity  Stop bits -> 1
        endcase
    end
    4'b1001: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Odd     Parity, 1 Stop bit"    ; // Start, 6 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Even    Parity, 1 Stop bit"    ; // Start, 6 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Force 1 Parity, 1 Stop bit"    ; // Start, 6 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Force 0 Parity, 1 Stop bit"    ; // Start, 6 bits, Parity  Stop bits -> 1
        endcase
    end
    4'b1010: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Odd     Parity, 1 Stop bit"    ; // Start, 7 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Even    Parity, 1 Stop bit"    ; // Start, 7 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Force 1 Parity, 1 Stop bit"    ; // Start, 7 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Force 0 Parity, 1 Stop bit"    ; // Start, 7 bits, Parity  Stop bits -> 1
        endcase
    end
    4'b1011: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Odd     Parity, 1 Stop bit"    ; // Start, 8 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Even    Parity, 1 Stop bit"    ; // Start, 8 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Force 1 Parity, 1 Stop bit"    ; // Start, 8 bits, Parity  Stop bits -> 1
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Force 0 Parity, 1 Stop bit"    ; // Start, 8 bits, Parity  Stop bits -> 1
        endcase
    end


    4'b1100: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Odd     Parity, 1.5 Stop bit"  ; // Start, 5 bits, Parity  Stop bits -> 1.5
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Even    Parity, 1.5 Stop bit"  ; // Start, 5 bits, Parity  Stop bits -> 1.5
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Force 1 Parity, 1.5 Stop bit"  ; // Start, 5 bits, Parity  Stop bits -> 1.5
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 5 data bits, Force 0 Parity, 1.5 Stop bit"  ; // Start, 5 bits, Parity  Stop bits -> 1.5
        endcase
    end
    4'b1101: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Odd     Parity, 2   Stop bit"  ; // Start, 6 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Even    Parity, 2   Stop bit"  ; // Start, 6 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Force 1 Parity, 2   Stop bit"  ; // Start, 6 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 6 data bits, Force 0 Parity, 2   Stop bit"  ; // Start, 6 bits, Parity  Stop bits -> 2
        endcase
    end
    4'b1110: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Odd     Parity, 2   Stop bit"  ; // Start, 7 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Even    Parity, 2   Stop bit"  ; // Start, 7 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Force 1 Parity, 2   Stop bit"  ; // Start, 7 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 7 data bits, Force 0 Parity, 2   Stop bit"  ; // Start, 7 bits, Parity  Stop bits -> 2
        endcase
    end
    4'b1111: 
    begin
        case(SERIAL_PARITY)
        SERIAL_PARITY_ODD     : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Odd     Parity, 2   Stop bit"  ; // Start, 8 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_EVEN    : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Even    Parity, 2   Stop bit"  ; // Start, 8 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_FORCE_1 : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Force 1 Parity, 2   Stop bit"  ; // Start, 8 bits, Parity  Stop bits -> 2
        SERIAL_PARITY_FORCE_0 : Serial_Tx_gen_bfm_msg2  = "Tx of 8 data bits, Force 0 Parity, 2   Stop bit"  ; // Start, 8 bits, Parity  Stop bits -> 2
        endcase
    end
    endcase


	// Determine the time period for each bit in the serial stream
	//
    case(BAUD_RATE)
    SERIAL_BAUD_RATE_110    :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_110    ;
    SERIAL_BAUD_RATE_300    :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_300    ;
    SERIAL_BAUD_RATE_600    :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_600    ;
    SERIAL_BAUD_RATE_1200   :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_1200   ;
    SERIAL_BAUD_RATE_2400   :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_2400   ;
    SERIAL_BAUD_RATE_4800   :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_4800   ;
    SERIAL_BAUD_RATE_9600   :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_9600   ;
    SERIAL_BAUD_RATE_14400  :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_14400  ;
    SERIAL_BAUD_RATE_19200  :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_19200  ;
    SERIAL_BAUD_RATE_38400  :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_38400  ;
    SERIAL_BAUD_RATE_57600  :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_57600  ;
    SERIAL_BAUD_RATE_115200 :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_115200 ;
    SERIAL_BAUD_RATE_230400 :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_230400 ;
    SERIAL_BAUD_RATE_921600 :  expected_xfr_bit_time  = BAUD_RATE_PERIOD_921600 ;
    endcase


	// Issue a diagnostic Message
	//
    Serial_Tx_gen_bfm_msg3 = "Serial Tx Start Bit";

    // Apply the start Start Bit
	//

    Tx = 1'b0              ;
    # expected_xfr_bit_time;


    // Determine the number of data bits to transmit
	//
    case(SERIAL_DATA)
    SERIAL_DATA_5_BITS: expected_number_of_data_bits =  5  ; // 5 bits
    SERIAL_DATA_6_BITS: expected_number_of_data_bits =  6  ; // 6 bits
    SERIAL_DATA_7_BITS: expected_number_of_data_bits =  7  ; // 7 bits
    SERIAL_DATA_8_BITS: expected_number_of_data_bits =  8  ; // 8 bits
    endcase

    // Transmit the serial data
    //
	for (i = 0; i < expected_number_of_data_bits; i = i + 1)
    begin

	    // Issue a diagnostic Message
	    //
        case(i[2:0])
        3'h0: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[0]";
        3'h1: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[1]";
        3'h2: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[2]";
        3'h3: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[3]";
        3'h4: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[4]";
        3'h5: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[5]";
        3'h6: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[6]";
        3'h7: Serial_Tx_gen_bfm_msg3 = "Serial Tx Data Bit[7]";
        endcase

        Tx = TARGET_DATA[i];
        # expected_xfr_bit_time;

    end


    // Determine the Odd parity for the valid data bits
	//
    case(SERIAL_DATA)
    SERIAL_DATA_5_BITS: expected_parity_odd = ^TARGET_DATA[4:0];
    SERIAL_DATA_6_BITS: expected_parity_odd = ^TARGET_DATA[5:0];
    SERIAL_DATA_7_BITS: expected_parity_odd = ^TARGET_DATA[6:0];
    SERIAL_DATA_8_BITS: expected_parity_odd = ^TARGET_DATA[7:0];
    endcase

    // Now Determine if there is a parity bit to be sent
    //
	// Note: The default is to do nothing -> NO PARITY
	//
    case(SERIAL_PARITY)
//  SERIAL_PARITY_NONE           : // No parity bit generated so skip this delay
    SERIAL_PARITY_ODD            :
    begin
        Serial_Tx_gen_bfm_msg3 = "Serial Tx Odd Parity Bit"    ;
        Tx                     =  expected_parity_odd          ;
        # expected_xfr_bit_time                                ;
    end
    SERIAL_PARITY_EVEN           :
    begin
        Serial_Tx_gen_bfm_msg3 = "Serial Tx Even Parity Bit"   ;
        Tx                     = ~expected_parity_odd          ;
        # expected_xfr_bit_time                                ;
    end
    SERIAL_PARITY_FORCE_1        :
    begin
        Serial_Tx_gen_bfm_msg3 = "Serial Tx Force 1 Parity Bit";
        Tx                     = 1'b1                          ;
        # expected_xfr_bit_time                                ;
    end
    SERIAL_PARITY_FORCE_0        :
    begin
        Serial_Tx_gen_bfm_msg3 = "Serial Tx Force 0 Parity Bit";
        Tx                     = 1'b0                          ;
        # expected_xfr_bit_time                                ;
    end
    endcase



    // Determine the number of Stop Bits
    //
    // Note: There is always at least one.
    //

    // Print a diagnostic message
    //
    Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 1";

    Tx = 1'b1                ;
    #expected_xfr_bit_time   ;    

    // Now determine how many more "Stop" bits to generate
    //

    case({SERIAL_PARITY[0], SERIAL_STOP, SERIAL_DATA})
    4'b0000:                                                  ;    // Start, 5 bits,         Stop bits -> 1
    4'b0001:                                                  ;    // Start, 6 bits,         Stop bits -> 1
    4'b0010:                                                  ;    // Start, 7 bits,         Stop bits -> 1
    4'b0011:                                                  ;    // Start, 8 bits,         Stop bits -> 1

    4'b0100: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 0.5";    // Start, 5 bits,         Stop bits -> 1.5
    4'b0101: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 2"  ;    // Start, 6 bits,         Stop bits -> 2
    4'b0110: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 2"  ;    // Start, 7 bits,         Stop bits -> 2
    4'b0111: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 2"  ;    // Start, 8 bits,         Stop bits -> 2

    4'b1000:                                                  ;    // Start, 5 bits, Parity  Stop bits -> 1
    4'b1001:                                                  ;    // Start, 6 bits, Parity  Stop bits -> 1
    4'b1010:                                                  ;    // Start, 7 bits, Parity  Stop bits -> 1
    4'b1011:                                                  ;    // Start, 8 bits, Parity  Stop bits -> 1

    4'b1100: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 0.5";    // Start, 5 bits, Parity, Stop bits -> 1.5
    4'b1101: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 2"  ;    // Start, 6 bits, Parity, Stop bits -> 2
    4'b1110: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 2"  ;    // Start, 7 bits, Parity, Stop bits -> 2
    4'b1111: Serial_Tx_gen_bfm_msg3 = "Serial Tx Stop bit 2"  ;    // Start, 8 bits, Parity, Stop bits -> 2
    endcase


    case({SERIAL_PARITY[0], SERIAL_STOP, SERIAL_DATA})
    4'b0000:                                                  ;    // Start, 5 bits,         Stop bits -> 1
    4'b0001:                                                  ;    // Start, 6 bits,         Stop bits -> 1
    4'b0010:                                                  ;    // Start, 7 bits,         Stop bits -> 1
    4'b0011:                                                  ;    // Start, 8 bits,         Stop bits -> 1

    4'b0100: # (0.5 * expected_xfr_bit_time)                  ;    // Start, 5 bits,         Stop bits -> 1.5
    4'b0101: # (1.0 * expected_xfr_bit_time)                  ;    // Start, 6 bits,         Stop bits -> 2
    4'b0110: # (1.0 * expected_xfr_bit_time)                  ;    // Start, 7 bits,         Stop bits -> 2
    4'b0111: # (1.0 * expected_xfr_bit_time)                  ;    // Start, 8 bits,         Stop bits -> 2

    4'b1000:                                                  ;    // Start, 5 bits, Parity  Stop bits -> 1
    4'b1001:                                                  ;    // Start, 6 bits, Parity  Stop bits -> 1
    4'b1010:                                                  ;    // Start, 7 bits, Parity  Stop bits -> 1
    4'b1011:                                                  ;    // Start, 8 bits, Parity  Stop bits -> 1

    4'b1100: # (0.5 * expected_xfr_bit_time)                  ;    // Start, 5 bits, Parity, Stop bits -> 1.5
    4'b1101: # (1.0 * expected_xfr_bit_time)                  ;    // Start, 6 bits, Parity, Stop bits -> 2
    4'b1110: # (1.0 * expected_xfr_bit_time)                  ;    // Start, 7 bits, Parity, Stop bits -> 2
    4'b1111: # (1.0 * expected_xfr_bit_time)                  ;    // Start, 8 bits, Parity, Stop bits -> 2
    endcase


	// Clear the message fields
	//
    Serial_Tx_gen_bfm_msg1 = "NO ACTIVITY";  // Bus used for depositing test messages in ASCI
    Serial_Tx_gen_bfm_msg2 = "NO ACTIVITY";  // Bus used for depositing test messages in ASCI
    Serial_Tx_gen_bfm_msg3 = "NO ACTIVITY";  // Bus used for depositing test messages in ASCI
    Serial_Tx_gen_bfm_msg4 = "NO ACTIVITY";  // Bus used for depositing test messages in ASCI
end
endtask


endmodule
