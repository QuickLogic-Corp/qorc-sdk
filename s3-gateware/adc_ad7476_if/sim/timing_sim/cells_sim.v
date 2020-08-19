// ../../../quicklogic/pp3/primitives/vcc/vcc.sim.v {{{
(* whitebox *)
module VCC (
    output wire VCC
);

    assign VCC = 1'b1;

endmodule
// ../../../quicklogic/pp3/primitives/vcc/vcc.sim.v }}}

// ../../../quicklogic/pp3/primitives/gnd/gnd.sim.v {{{
(* whitebox *)
module GND (
    output wire GND
);

    assign GND = 1'b0;

endmodule
// ../../../quicklogic/pp3/primitives/gnd/gnd.sim.v }}}

// ../../../quicklogic/pp3/primitives/fpga_interconnect/fpga_interconnect.sim.v {{{
`timescale 1ns/10ps
(* whitebox *)
module fpga_interconnect(
		datain,
		dataout
		);
    input wire datain;
    output wire dataout;

    specify
        (datain=>dataout)="";
    endspecify

    assign dataout = datain;

endmodule
// ../../../quicklogic/pp3/primitives/fpga_interconnect/fpga_interconnect.sim.v }}}

// ../../../quicklogic/pp3/primitives/clock/clock_cell.sim.v {{{
`timescale 1ns/10ps
(* whitebox *)
(* FASM_FEATURES="INTERFACE.ASSP.INV.ASSPInvPortAlias" *)
module CLOCK_CELL(I_PAD, O_CLK);

    (* iopad_external_pin *)
    input  wire I_PAD;

    (* CLOCK=0 *)
    (* DELAY_CONST_I_PAD="{iopath_IP_IC}" *)
    output wire O_CLK;
	
	specify
        (I_PAD=>O_CLK)="";
    endspecify

    assign O_CLK = I_PAD;

endmodule
// ../../../quicklogic/pp3/primitives/clock/clock_cell.sim.v }}}

// ../../../quicklogic/pp3/primitives/bidir/bidir_cell.sim.v {{{
`timescale 1ns/10ps
(* whitebox *)
(* FASM_PARAMS="INV.ESEL=ESEL;INV.OSEL=OSEL;INV.FIXHOLD=FIXHOLD;INV.WPD=WPD;INV.DS=DS" *)
module BIDIR_CELL(
    I_PAD_$inp, I_DAT, I_EN,
    O_PAD_$out, O_DAT, O_EN
);
    (* iopad_external_pin *)
    input  wire I_PAD_$inp;
    input  wire I_EN;

    input  wire O_DAT;
    input  wire O_EN;

    (* DELAY_CONST_I_PAD_$inp="{iopath_IP_IZ}" *)
    (* DELAY_CONST_I_EN="1e-10" *)
    output wire I_DAT;

    (* DELAY_CONST_O_DAT="{iopath_OQI_IP}" *)
    (* DELAY_CONST_O_EN="{iopath_IE_IP}" *)
	(* iopad_external_pin *)
    output wire O_PAD_$out;
	
    specify
        (O_DAT => O_PAD_$out) = "";
        (O_EN => O_PAD_$out) = "";
        (I_PAD_$inp => I_DAT) = "";
        (I_EN => I_DAT) = "";
    endspecify
    parameter [0:0] ESEL    = 0;
    parameter [0:0] OSEL    = 0;
    parameter [0:0] FIXHOLD = 0;
    parameter [0:0] WPD     = 0;
    parameter [0:0] DS      = 0;
    assign I_DAT = (I_EN == 1'b1) ? I_PAD_$inp : 1'b0;
    assign O_PAD_$out = (O_EN == 1'b1) ? O_DAT : 1'b0;

endmodule
// ../../../quicklogic/pp3/primitives/bidir/bidir_cell.sim.v }}}

// ../../../quicklogic/pp3/primitives/sdiomux/sdiomux_cell.sim.v {{{
`timescale 1ns/10ps
(* whitebox *)
module SDIOMUX_CELL(
    I_PAD_$inp, I_DAT, I_EN,
    O_PAD_$out, O_DAT, O_EN
);
    (* iopad_external_pin *)
    input  wire I_PAD_$inp;
    input  wire I_EN;

    input  wire O_DAT;
    input  wire O_EN;

    (* DELAY_CONST_I_PAD_$inp="{iopath_IP_IZ}" *)
    (* DELAY_CONST_I_EN="{iopath_FBIO_In_En0_FBIO_In0}" *)
    output wire I_DAT;

    (* DELAY_CONST_O_DAT="{iopath_OQI_IP}" *)
    (* DELAY_CONST_O_EN="{iopath_OE_IP}" *)
    (* iopad_external_pin *)
    output wire O_PAD_$out;
	
    specify
        (O_DAT => O_PAD_$out) = "";
        (O_EN => O_PAD_$out) = "";
        (I_PAD_$inp => I_DAT) = "";
        (I_EN => I_DAT) = "";
    endspecify
    assign I_DAT = (I_EN == 1'b0) ? I_PAD_$inp : 1'b0;
    assign O_PAD_$out = (O_EN == 1'b0) ? O_DAT : 1'b0;

endmodule
// ../../../quicklogic/pp3/primitives/sdiomux/sdiomux_cell.sim.v }}}

// ../../../quicklogic/pp3/primitives/logic/t_frag.sim.v {{{
`timescale 1ns/10ps
(* FASM_PARAMS="INV.TA1=XAS1;INV.TA2=XAS2;INV.TB1=XBS1;INV.TB2=XBS2" *)
(* MODEL_NAME="T_FRAG" *)
(* whitebox *)
module T_FRAG (TBS, XAB, XSL, XA1, XA2, XB1, XB2, XZ);
    input  wire TBS;

    input  wire XAB;
    input  wire XSL;
    input  wire XA1;
    input  wire XA2;
    input  wire XB1;
    input  wire XB2;
    (* DELAY_CONST_TBS="{iopath_TBS_CZ}" *)
    (* DELAY_CONST_XAB="{iopath_TAB_TZ}" *)
    (* DELAY_CONST_XSL="{iopath_TSL_TZ}" *)
    (* DELAY_CONST_XA1="{iopath_TA1_TZ}" *)
    (* DELAY_CONST_XA2="{iopath_TA2_TZ}" *)
    (* DELAY_CONST_XB1="{iopath_TB1_TZ}" *)
    (* DELAY_CONST_XB2="{iopath_TB2_TZ}" *)
    output wire XZ;
    
    specify
        (TBS => XZ) = "";
        (XAB => XZ) = "";
        (XSL => XZ) = "";
        (XA1 => XZ) = "";
        (XA2 => XZ) = "";
        (XB1 => XZ) = "";
        (XB2 => XZ) = "";
    endspecify
    parameter [0:0] XAS1 = 1'b0;
    parameter [0:0] XAS2 = 1'b0;
    parameter [0:0] XBS1 = 1'b0;
    parameter [0:0] XBS2 = 1'b0;
    wire XAP1 = (XAS1) ? ~XA1 : XA1;
    wire XAP2 = (XAS2) ? ~XA2 : XA2;
    wire XBP1 = (XBS1) ? ~XB1 : XB1;
    wire XBP2 = (XBS2) ? ~XB2 : XB2;
    wire XAI = XSL ? XAP2 : XAP1;
    wire XBI = XSL ? XBP2 : XBP1;
    wire XZI = XAB ? XBI : XAI;
    assign XZ = TBS ? XZI : 1'b0;

endmodule
// ../../../quicklogic/pp3/primitives/logic/t_frag.sim.v }}}

// ../../../quicklogic/pp3/primitives/logic/b_frag.sim.v {{{
`timescale 1ns/10ps
(* FASM_PARAMS="INV.BA1=XAS1;INV.BA2=XAS2;INV.BB1=XBS1;INV.BB2=XBS2" *)
(* MODEL_NAME="T_FRAG" *)
(* whitebox *)
module B_FRAG (TBS, XAB, XSL, XA1, XA2, XB1, XB2, XZ);
    input  wire TBS;

    input  wire XAB;
    input  wire XSL;
    input  wire XA1;
    input  wire XA2;
    input  wire XB1;
    input  wire XB2;
    (* DELAY_CONST_TBS="{iopath_TBS_CZ}" *)
    (* DELAY_CONST_XAB="{iopath_BAB_CZ}" *)
    (* DELAY_CONST_XSL="{iopath_BSL_CZ}" *)
    (* DELAY_CONST_XA1="{iopath_BA1_CZ}" *)
    (* DELAY_CONST_XA2="{iopath_BA2_CZ}" *)
    (* DELAY_CONST_XB1="{iopath_BB1_CZ}" *)
    (* DELAY_CONST_XB2="{iopath_BB2_CZ}" *)
    output wire XZ;
   
    specify
        (TBS => XZ) = "";
        (XAB => XZ) = "";
        (XSL => XZ) = "";
        (XA1 => XZ) = "";
        (XA2 => XZ) = "";
        (XB1 => XZ) = "";
        (XB2 => XZ) = "";
    endspecify
    parameter [0:0] XAS1 = 1'b0;
    parameter [0:0] XAS2 = 1'b0;
    parameter [0:0] XBS1 = 1'b0;
    parameter [0:0] XBS2 = 1'b0;
    wire XAP1 = (XAS1) ? ~XA1 : XA1;
    wire XAP2 = (XAS2) ? ~XA2 : XA2;
    wire XBP1 = (XBS1) ? ~XB1 : XB1;
    wire XBP2 = (XBS2) ? ~XB2 : XB2;
    wire XAI = XSL ? XAP2 : XAP1;
    wire XBI = XSL ? XBP2 : XBP1;
    wire XZI = XAB ? XBI : XAI;
    assign XZ = TBS ? XZI : 1'b0;

endmodule
// ../../../quicklogic/pp3/primitives/logic/b_frag.sim.v }}}

// ../../../quicklogic/pp3/primitives/logic/q_frag.sim.v {{{
`timescale 1ns/10ps
(* FASM_PARAMS="ZINV.QCK=Z_QCKS" *)
(* whitebox *)
module Q_FRAG(QCK, QST, QRT, QEN, QDI, QDS, CZI, QZ);
    (* CLOCK *)
	(* clkbuf_sink *)
    input  wire QCK;
	(* SETUP="QCK 1e-10" *) (* NO_COMB *)
    input  wire QST;
	(* SETUP="QCK 1e-10" *) (* NO_COMB *)
    input  wire QRT;
	(* SETUP="QCK {setup_QCK_QEN}" *) (* NO_COMB *)
	(* HOLD="QCK {hold_QCK_QEN}" *) (* NO_COMB *)
    input  wire QEN;

	(* SETUP="QCK {setup_QCK_QDI}" *) (* NO_COMB *)
	(* HOLD="QCK {hold_QCK_QDI}" *) (* NO_COMB *)
    input  wire QDI;

	(* SETUP="QCK {setup_QCK_QDS}" *) (* NO_COMB *)
	(* HOLD="QCK {hold_QCK_QDS}" *) (* NO_COMB *)
    input  wire QDS;
	(* SETUP="QCK {setup_QCK_QDI}" *) (* NO_COMB *)
	(* HOLD="QCK {hold_QCK_QDI}" *) (* NO_COMB *)
    input  wire CZI;

	(* CLK_TO_Q = "QCK {iopath_QCK_QZ}" *)
    output reg  QZ;
    
    specify
        (QCK => QZ) = "";
		$setup(CZI, posedge QCK, "");
        $hold(posedge QCK, CZI, "");
        $setup(QDI, posedge QCK, "");
        $hold(posedge QCK, QDI, "");
        $setup(QST, posedge QCK, "");
        $hold(posedge QCK, QST, "");
        $setup(QRT, posedge QCK, "");
        $hold(posedge QCK, QRT, "");
        $setup(QEN, posedge QCK, "");
        $hold(posedge QCK, QEN, "");
        $setup(QDS, posedge QCK, "");
        $hold(posedge QCK, QDS, "");
    endspecify
    parameter [0:0] Z_QCKS = 1'b1;
    wire d = (QDS) ? QDI : CZI;
    initial QZ <= 1'b0;
	always @(posedge QCK or posedge QST or posedge QRT) begin
		if (QST)
			QZ <= 1'b1;
		else if (QRT)
			QZ <= 1'b0;
		else if (QEN)
			QZ <= d;
	end

endmodule
// ../../../quicklogic/pp3/primitives/logic/q_frag.sim.v }}}

// ../../../quicklogic/pp3/primitives/logic/f_frag.sim.v {{{
`timescale 1ns/10ps
(* whitebox *)
module F_FRAG (F1, F2, FS, FZ);
    input  wire F1;
    input  wire F2;
    input  wire FS;

    (* DELAY_CONST_F1="{iopath_F1_FZ}" *)
    (* DELAY_CONST_F2="{iopath_F2_FZ}" *)
    (* DELAY_CONST_FS="{iopath_FS_FZ}" *)
    output wire FZ;
    specify
        (F1 => FZ) = "";
        (F2 => FZ) = "";
        (FS => FZ) = "";
    endspecify
    assign FZ = FS ? F2 : F1;

endmodule
// ../../../quicklogic/pp3/primitives/logic/f_frag.sim.v }}}

// ../../../quicklogic/pp3/primitives/logic/c_frag.sim.v {{{
(* FASM_PARAMS="INV.TA1=TAS1;INV.TA2=TAS2;INV.TB1=TBS1;INV.TB2=TBS2;INV.BA1=BAS1;INV.BA2=BAS2;INV.BB1=BBS1;INV.BB2=BBS2" *)
(* whitebox *)
module C_FRAG (TBS, TAB, TSL, TA1, TA2, TB1, TB2, BAB, BSL, BA1, BA2, BB1, BB2, TZ, CZ);
    input  wire TBS;

    input  wire TAB;
    input  wire TSL;
    input  wire TA1;
    input  wire TA2;
    input  wire TB1;
    input  wire TB2;

    input  wire BAB;
    input  wire BSL;
    input  wire BA1;
    input  wire BA2;
    input  wire BB1;
    input  wire BB2;

    (* DELAY_CONST_TAB="{iopath_TAB_TZ}" *)
    (* DELAY_CONST_TSL="{iopath_TSL_TZ}" *)
    (* DELAY_CONST_TA1="{iopath_TA1_TZ}" *)
    (* DELAY_CONST_TA2="{iopath_TA2_TZ}" *)
    (* DELAY_CONST_TB1="{iopath_TB1_TZ}" *)
    (* DELAY_CONST_TB2="{iopath_TB2_TZ}" *)
    output wire TZ;

    (* DELAY_CONST_TBS="{iopath_TBS_CZ}" *)
    (* DELAY_CONST_TAB="{iopath_TAB_CZ}" *)
    (* DELAY_CONST_TSL="{iopath_TSL_CZ}" *)
    (* DELAY_CONST_TA1="{iopath_TA1_CZ}" *)
    (* DELAY_CONST_TA2="{iopath_TA2_CZ}" *)
    (* DELAY_CONST_TB1="{iopath_TB1_CZ}" *)
    (* DELAY_CONST_TB2="{iopath_TB2_CZ}" *)
    (* DELAY_CONST_BAB="{iopath_BAB_CZ}" *)
    (* DELAY_CONST_BSL="{iopath_BSL_CZ}" *)
    (* DELAY_CONST_BA1="{iopath_BA1_CZ}" *)
    (* DELAY_CONST_BA2="{iopath_BA2_CZ}" *)
    (* DELAY_CONST_BB1="{iopath_BB1_CZ}" *)
    (* DELAY_CONST_BB2="{iopath_BB2_CZ}" *)
    output wire CZ;
    parameter [0:0] TAS1 = 1'b0;
    parameter [0:0] TAS2 = 1'b0;
    parameter [0:0] TBS1 = 1'b0;
    parameter [0:0] TBS2 = 1'b0;

    parameter [0:0] BAS1 = 1'b0;
    parameter [0:0] BAS2 = 1'b0;
    parameter [0:0] BBS1 = 1'b0;
    parameter [0:0] BBS2 = 1'b0;
    wire TAP1 = (TAS1) ? ~TA1 : TA1;
    wire TAP2 = (TAS2) ? ~TA2 : TA2;
    wire TBP1 = (TBS1) ? ~TB1 : TB1;
    wire TBP2 = (TBS2) ? ~TB2 : TB2;

    wire BAP1 = (BAS1) ? ~BA1 : BA1;
    wire BAP2 = (BAS2) ? ~BA2 : BA2;
    wire BBP1 = (BBS1) ? ~BB1 : BB1;
    wire BBP2 = (BBS2) ? ~BB2 : BB2;
    wire TAI = TSL ? TAP2 : TAP1;
    wire TBI = TSL ? TBP2 : TBP1;
    
    wire BAI = BSL ? BAP2 : BAP1;
    wire BBI = BSL ? BBP2 : BBP1;
    wire TZI = TAB ? TBI : TAI;
    wire BZI = BAB ? BBI : BAI;
    wire CZI = TBS ? BZI : TZI;
    assign TZ = TZI;
    assign CZ = CZI;

    specify
        (TBS => CZ) = "";
        (TAB => CZ) = "";
        (TSL => CZ) = "";
        (TA1 => CZ) = "";
        (TA2 => CZ) = "";
        (TB1 => CZ) = "";
        (TB2 => CZ) = "";
        (BAB => CZ) = "";
        (BSL => CZ) = "";
        (BA1 => CZ) = "";
        (BA2 => CZ) = "";
        (BB1 => CZ) = "";
        (BB2 => CZ) = "";
        (TAB => TZ) = "";
        (TSL => TZ) = "";
        (TA1 => TZ) = "";
        (TA2 => TZ) = "";
        (TB1 => TZ) = "";
        (TB2 => TZ) = "";
    endspecify

endmodule
// ../../../quicklogic/pp3/primitives/logic/c_frag.sim.v }}}

// ../../../quicklogic/pp3/primitives/logic/logic_macro.sim.v {{{
(* FASM_PARAMS="INV.TA1=TAS1;INV.TA2=TAS2;INV.TB1=TBS1;INV.TB2=TBS2;INV.BA1=BAS1;INV.BA2=BAS2;INV.BB1=BBS1;INV.BB2=BBS2;ZINV.QCK=Z_QCKS" *)
(* whitebox *)
module LOGIC_MACRO (QST, QDS, TBS, TAB, TSL, TA1, TA2, TB1, TB2, BAB, BSL, BA1, BA2, BB1, BB2, QDI, QEN, QCK, QRT, F1, F2, FS, TZ, CZ, QZ, FZ);

    (* NO_SEQ *)
    input  wire TBS;
    (* NO_SEQ *)
    input  wire TAB;
    (* NO_SEQ *)
    input  wire TSL;
    (* NO_SEQ *)
    input  wire TA1;
    (* NO_SEQ *)
    input  wire TA2;
    (* NO_SEQ *)
    input  wire TB1;
    (* NO_SEQ *)
    input  wire TB2;
    (* NO_SEQ *)
    input  wire BAB;
    (* NO_SEQ *)
    input  wire BSL;
    (* NO_SEQ *)
    input  wire BA1;
    (* NO_SEQ *)
    input  wire BA2;
    (* NO_SEQ *)
    input  wire BB1;
    (* NO_SEQ *)
    input  wire BB2;

    (* DELAY_CONST_TAB="{iopath_TAB_TZ}" *)
    (* DELAY_CONST_TSL="{iopath_TSL_TZ}" *)
    (* DELAY_CONST_TA1="{iopath_TA1_TZ}" *)
    (* DELAY_CONST_TA2="{iopath_TA2_TZ}" *)
    (* DELAY_CONST_TB1="{iopath_TB1_TZ}" *)
    (* DELAY_CONST_TB2="{iopath_TB2_TZ}" *)
    output wire TZ;

    (* DELAY_CONST_TBS="{iopath_TBS_CZ}" *)
    (* DELAY_CONST_TAB="{iopath_TAB_CZ}" *)
    (* DELAY_CONST_TSL="{iopath_TSL_CZ}" *)
    (* DELAY_CONST_TA1="{iopath_TA1_CZ}" *)
    (* DELAY_CONST_TA2="{iopath_TA2_CZ}" *)
    (* DELAY_CONST_TB1="{iopath_TB1_CZ}" *)
    (* DELAY_CONST_TB2="{iopath_TB2_CZ}" *)
    (* DELAY_CONST_BAB="{iopath_BAB_CZ}" *)
    (* DELAY_CONST_BSL="{iopath_BSL_CZ}" *)
    (* DELAY_CONST_BA1="{iopath_BA1_CZ}" *)
    (* DELAY_CONST_BA2="{iopath_BA2_CZ}" *)
    (* DELAY_CONST_BB1="{iopath_BB1_CZ}" *)
    (* DELAY_CONST_BB2="{iopath_BB2_CZ}" *)
    output wire CZ;
    parameter [0:0] TAS1 = 1'b0;
    parameter [0:0] TAS2 = 1'b0;
    parameter [0:0] TBS1 = 1'b0;
    parameter [0:0] TBS2 = 1'b0;

    parameter [0:0] BAS1 = 1'b0;
    parameter [0:0] BAS2 = 1'b0;
    parameter [0:0] BBS1 = 1'b0;
    parameter [0:0] BBS2 = 1'b0;
    wire TAP1 = (TAS1) ? ~TA1 : TA1;
    wire TAP2 = (TAS2) ? ~TA2 : TA2;
    wire TBP1 = (TBS1) ? ~TB1 : TB1;
    wire TBP2 = (TBS2) ? ~TB2 : TB2;

    wire BAP1 = (BAS1) ? ~BA1 : BA1;
    wire BAP2 = (BAS2) ? ~BA2 : BA2;
    wire BBP1 = (BBS1) ? ~BB1 : BB1;
    wire BBP2 = (BBS2) ? ~BB2 : BB2;
    wire TAI = TSL ? TAP2 : TAP1;
    wire TBI = TSL ? TBP2 : TBP1;
    
    wire BAI = BSL ? BAP2 : BAP1;
    wire BBI = BSL ? BBP2 : BBP1;
    wire TZI = TAB ? TBI : TAI;
    wire BZI = BAB ? BBI : BAI;
    wire CZI = TBS ? BZI : TZI;
    assign TZ = TZI;
    assign CZ = CZI;

    (* CLOCK *)
    (* clkbuf_sink *)
    input  wire QCK;
	(* SETUP="QCK 1e-10" *)
    (* NO_COMB *)
    input  wire QST;
	(* SETUP="QCK 1e-10" *)
    (* NO_COMB *)
    input  wire QRT;
	(* SETUP="QCK {setup_QCK_QEN}" *)
	(* HOLD="QCK {hold_QCK_QEN}" *)
    (* NO_COMB *)
    input  wire QEN;

	(* SETUP="QCK {setup_QCK_QDI}" *)
	(* HOLD="QCK {hold_QCK_QDI}" *)
    (* NO_COMB *)
    input  wire QDI;

	(* SETUP="QCK {setup_QCK_QDS}" *)
	(* HOLD="QCK {hold_QCK_QDS}" *)
    (* NO_COMB *)
    input  wire QDS;

	(* CLK_TO_Q = "QCK {iopath_QCK_QZ}" *)
    (* DELAY_CONST_TBS="{iopath_TBS_CZ}" *)
    (* DELAY_CONST_TAB="{iopath_TAB_CZ}" *)
    (* DELAY_CONST_TSL="{iopath_TSL_CZ}" *)
    (* DELAY_CONST_TA1="{iopath_TA1_CZ}" *)
    (* DELAY_CONST_TA2="{iopath_TA2_CZ}" *)
    (* DELAY_CONST_TB1="{iopath_TB1_CZ}" *)
    (* DELAY_CONST_TB2="{iopath_TB2_CZ}" *)
    (* DELAY_CONST_BAB="{iopath_BAB_CZ}" *)
    (* DELAY_CONST_BSL="{iopath_BSL_CZ}" *)
    (* DELAY_CONST_BA1="{iopath_BA1_CZ}" *)
    (* DELAY_CONST_BA2="{iopath_BA2_CZ}" *)
    (* DELAY_CONST_BB1="{iopath_BB1_CZ}" *)
    (* DELAY_CONST_BB2="{iopath_BB2_CZ}" *)
	(* SETUP="QCK {setup_QCK_QDI}" *)
	(* HOLD="QCK {hold_QCK_QDI}" *)
    output reg  QZ;
    parameter [0:0] Z_QCKS = 1'b1;
    wire QZI = (QDS) ? QDI : CZI;
    initial QZ <= 1'b0;
	always @(posedge QCK or posedge QST or posedge QRT) begin
		if (QST)
			QZ <= 1'b1;
		else if (QRT)
			QZ <= 1'b0;
		else if (QEN)
			QZ <= QZI;
	end

    input  wire F1;
    input  wire F2;
    input  wire FS;

    (* DELAY_CONST_F1="{iopath_F1_FZ}" *)
    (* DELAY_CONST_F2="{iopath_F2_FZ}" *)
    (* DELAY_CONST_FS="{iopath_FS_FZ}" *)
    output wire FZ;
    assign FZ = FS ? F2 : F1;

endmodule
// ../../../quicklogic/pp3/primitives/logic/logic_macro.sim.v }}}

// ../../../quicklogic/pp3/primitives/assp/assp_bfm.sim.v {{{

`timescale 1ns/10ps
module ahb_gen_bfm       (

                         A2F_HCLK,
                         A2F_HRESET,

                         A2F_HADDRS,
                         A2F_HSEL,
                         A2F_HTRANSS,
                         A2F_HSIZES,
                         A2F_HWRITES,
                         A2F_HREADYS,
                         A2F_HWDATAS,

                         A2F_HREADYOUTS,
                         A2F_HRESPS,
                         A2F_HRDATAS

			             );

parameter                ADDRWIDTH                   = 32;
parameter                DATAWIDTH                   = 32;

parameter                DEFAULT_AHB_ADDRESS         = {(ADDRWIDTH){1'b1}};

parameter                STD_CLK_DLY                 = 2;

parameter                ENABLE_AHB_REG_WR_DEBUG_MSG = 1'b1;
parameter                ENABLE_AHB_REG_RD_DEBUG_MSG = 1'b1;

parameter                TEST_MSG_ARRAY_SIZE         = (64 * 8);

input                    A2F_HCLK;
input                    A2F_HRESET;

output  [ADDRWIDTH-1:0]  A2F_HADDRS;
output                   A2F_HSEL;
output            [1:0]  A2F_HTRANSS;
output            [2:0]  A2F_HSIZES;
output                   A2F_HWRITES;
output                   A2F_HREADYS;
output  [DATAWIDTH-1:0]  A2F_HWDATAS;

input                    A2F_HREADYOUTS;
input                    A2F_HRESPS;
input   [DATAWIDTH-1:0]  A2F_HRDATAS;


wire                     A2F_HCLK;
wire                     A2F_HRESET;

reg     [ADDRWIDTH-1:0]  A2F_HADDRS;
reg                      A2F_HSEL;
reg               [1:0]  A2F_HTRANSS;
reg               [2:0]  A2F_HSIZES;
reg                      A2F_HWRITES;
reg                      A2F_HREADYS;
reg     [DATAWIDTH-1:0]  A2F_HWDATAS;

wire                     A2F_HREADYOUTS;
wire                     A2F_HRESPS;
wire    [DATAWIDTH-1:0]  A2F_HRDATAS;


reg	   [TEST_MSG_ARRAY_SIZE-1:0]  ahb_bfm_msg1;  
reg	   [TEST_MSG_ARRAY_SIZE-1:0]  ahb_bfm_msg2;  
reg	   [TEST_MSG_ARRAY_SIZE-1:0]  ahb_bfm_msg3;  
reg	   [TEST_MSG_ARRAY_SIZE-1:0]  ahb_bfm_msg4;  
reg    [TEST_MSG_ARRAY_SIZE-1:0]  ahb_bfm_msg5;  
reg    [TEST_MSG_ARRAY_SIZE-1:0]  ahb_bfm_msg6;  

initial
begin

    A2F_HADDRS   <=  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL     <=  1'b0;                 
    A2F_HTRANSS  <=  2'h0;                 
    A2F_HSIZES   <=  3'h0;                 
    A2F_HWRITES  <=  1'b0;                 
    A2F_HREADYS  <=  1'b0;                 
    A2F_HWDATAS  <=  {(DATAWIDTH){1'b0}};  

	ahb_bfm_msg1 <= "NO ACTIVITY";		
	ahb_bfm_msg2 <= "NO ACTIVITY";		
	ahb_bfm_msg3 <= "NO ACTIVITY";		
	ahb_bfm_msg4 <= "NO ACTIVITY";		
    ahb_bfm_msg5 <= "NO ACTIVITY";      
    ahb_bfm_msg6 <= "NO ACTIVITY";      
end

`ifdef GSIM
task ahb_read_al4s3b_fabric;
input   [ADDRWIDTH-1:0]	TARGET_ADDRESS;        
input             [2:0]	TARGET_XFR_SIZE;       
output  [DATAWIDTH-1:0]	TARGET_DATA;           

reg     [DATAWIDTH-1:0]   read_data;

integer i, j, k;

begin

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg1  = "AHB Single Read";
	ahb_bfm_msg2  = "Address Phase";
	ahb_bfm_msg3  = "SEQ";

    A2F_HADDRS    =  TARGET_ADDRESS;       


    A2F_HSEL      =  1'b1;                 
    A2F_HREADYS   =  1'b1;                 
    A2F_HTRANSS   =  2'h2;                 

    A2F_HSIZES    =  TARGET_XFR_SIZE;      

    A2F_HWRITES   =  1'b0;                 
    A2F_HWDATAS   =  {(DATAWIDTH){1'b0}};  

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg2  = "Data Phase";
	ahb_bfm_msg3  = "IDLE";
	ahb_bfm_msg4  = "Waiting for Slave";

    A2F_HADDRS    =  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL      =  1'b0;                 
    A2F_HTRANSS   =  2'h0;                 
    A2F_HSIZES    =  3'h0;                 

	while (A2F_HREADYOUTS == 1'b0)
    begin
        @(posedge A2F_HCLK) #STD_CLK_DLY;
    end

    A2F_HREADYS   =  1'b0;             
    TARGET_DATA   = A2F_HRDATAS;       

	ahb_bfm_msg1 <= "NO ACTIVITY";
	ahb_bfm_msg2 <= "NO ACTIVITY";
	ahb_bfm_msg3 <= "NO ACTIVITY";
	ahb_bfm_msg4 <= "NO ACTIVITY";
	ahb_bfm_msg5 <= "NO ACTIVITY";
	ahb_bfm_msg6 <= "NO ACTIVITY";

end
endtask


task ahb_write_al4s3b_fabric;
input   [ADDRWIDTH-1:0]	TARGET_ADDRESS;        
input             [2:0]	TARGET_XFR_SIZE;       
input   [DATAWIDTH-1:0]	TARGET_DATA;           

reg     [DATAWIDTH-1:0]   read_data;

integer i, j, k;

begin

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg1  = "AHB Single Write";
	ahb_bfm_msg2  = "Address Phase";
	ahb_bfm_msg3  = "SEQ";

    A2F_HADDRS    =  TARGET_ADDRESS;       

    A2F_HSEL      =  1'b1;                 
    A2F_HREADYS   =  1'b1;                 
    A2F_HTRANSS   =  2'h2;                 
	
    A2F_HSIZES    =  TARGET_XFR_SIZE;     

    A2F_HWRITES   =  1'b1;                
    A2F_HWDATAS   =  {(DATAWIDTH){1'b0}}; 

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg2  = "Data Phase";
	ahb_bfm_msg3  = "IDLE";
	ahb_bfm_msg4  = "Waiting for Slave";

    A2F_HADDRS    =  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL      =  1'b0;                 
    A2F_HTRANSS   =  2'h0;                 
    A2F_HSIZES    =  3'h0;                 
    A2F_HWDATAS   =  TARGET_DATA;          
    A2F_HWRITES   =  1'b0;                 

	while (A2F_HREADYOUTS == 1'b0)
    begin
        @(posedge A2F_HCLK) #STD_CLK_DLY;
    end

    A2F_HREADYS   =  1'b0;             
    TARGET_DATA   = A2F_HRDATAS;       

	ahb_bfm_msg1 <= "NO ACTIVITY";
	ahb_bfm_msg2 <= "NO ACTIVITY";
	ahb_bfm_msg3 <= "NO ACTIVITY";
	ahb_bfm_msg4 <= "NO ACTIVITY";
	ahb_bfm_msg5 <= "NO ACTIVITY";
	ahb_bfm_msg6 <= "NO ACTIVITY";

end
endtask

task ahb_read_word_al4s3b_fabric;
input   [ADDRWIDTH-1:0]	TARGET_ADDRESS;        
output  [DATAWIDTH-1:0]	TARGET_DATA;           

reg     [DATAWIDTH-1:0]   read_data;

integer i, j, k;

begin
	
    wait (A2F_HRESET == 0);
    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg1  = "AHB Single Read";
	ahb_bfm_msg2  = "Address Phase";
	ahb_bfm_msg3  = "SEQ";

    A2F_HADDRS    =  TARGET_ADDRESS;       

    A2F_HSEL      =  1'b1;                
    A2F_HREADYS   =  1'b1;                
    A2F_HTRANSS   =  2'h2;                

    A2F_HSIZES    =  3'b010;               

    A2F_HWRITES   =  1'b0;                 
    A2F_HWDATAS   =  {(DATAWIDTH){1'b0}};  

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg2  = "Data Phase";
	ahb_bfm_msg3  = "IDLE";
	ahb_bfm_msg4  = "Waiting for Slave";


    A2F_HADDRS    =  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL      =  1'b0;                 
    A2F_HTRANSS   =  2'h0;                 
    A2F_HSIZES    =  3'h0;                 

	while (A2F_HREADYOUTS == 1'b0)
    begin
        @(posedge A2F_HCLK) #STD_CLK_DLY;
    end

    A2F_HREADYS   =  1'b0;             
    TARGET_DATA   = A2F_HRDATAS;       

	ahb_bfm_msg1 <= "NO ACTIVITY";
	ahb_bfm_msg2 <= "NO ACTIVITY";
	ahb_bfm_msg3 <= "NO ACTIVITY";
	ahb_bfm_msg4 <= "NO ACTIVITY";
	ahb_bfm_msg5 <= "NO ACTIVITY";
	ahb_bfm_msg6 <= "NO ACTIVITY";

end
endtask


task ahb_write_word_al4s3b_fabric;
input   [ADDRWIDTH-1:0]	TARGET_ADDRESS;        
input   [DATAWIDTH-1:0]	TARGET_DATA;           

reg     [DATAWIDTH-1:0]   read_data;

integer i, j, k;

begin

	wait (A2F_HRESET == 0);
	
    @(posedge A2F_HCLK) #STD_CLK_DLY;


	ahb_bfm_msg1  = "AHB Single Write";
	ahb_bfm_msg2  = "Address Phase";
	ahb_bfm_msg3  = "SEQ";

    A2F_HADDRS    =  TARGET_ADDRESS;      

    A2F_HSEL      =  1'b1;                
    A2F_HREADYS   =  1'b1;                
    A2F_HTRANSS   =  2'h2;                

    A2F_HSIZES    =  3'b010;               

    A2F_HWRITES   =  1'b1;                 
    A2F_HWDATAS   =  {(DATAWIDTH){1'b0}};  

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg2  = "Data Phase";
	ahb_bfm_msg3  = "IDLE";
	ahb_bfm_msg4  = "Waiting for Slave";


    A2F_HADDRS    =  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL      =  1'b0;                 
    A2F_HTRANSS   =  2'h0;                 
    A2F_HSIZES    =  3'h0;                 
    A2F_HWDATAS   =  TARGET_DATA;          
    A2F_HWRITES   =  1'b0;                 

	while (A2F_HREADYOUTS == 1'b0)
    begin
        @(posedge A2F_HCLK) #STD_CLK_DLY;
    end

    A2F_HREADYS   =  1'b0;             
    TARGET_DATA   = A2F_HRDATAS;       

	ahb_bfm_msg1 <= "NO ACTIVITY";
	ahb_bfm_msg2 <= "NO ACTIVITY";
	ahb_bfm_msg3 <= "NO ACTIVITY";
	ahb_bfm_msg4 <= "NO ACTIVITY";
	ahb_bfm_msg5 <= "NO ACTIVITY";
	ahb_bfm_msg6 <= "NO ACTIVITY";
	
end
endtask

task ahb_write_al4s3b_fabric_mod;
input   [ADDRWIDTH-1:0]	TARGET_ADDRESS;        
input             [2:0]	TARGET_XFR_SIZE;       
input   [DATAWIDTH-1:0]	TARGET_DATA;           

reg     [DATAWIDTH-1:0]   read_data;

integer i, j, k;

begin

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg1  = "AHB Single Write";
	ahb_bfm_msg2  = "Address Phase";
	ahb_bfm_msg3  = "SEQ";

    A2F_HADDRS    =  {TARGET_ADDRESS[ADDRWIDTH-1:11],(TARGET_ADDRESS[10:0] << 2)} ;       

    A2F_HSEL      =  1'b1;                
    A2F_HREADYS   =  1'b1;                
    A2F_HTRANSS   =  2'h2;                

    A2F_HSIZES    =  TARGET_XFR_SIZE;    

    A2F_HWRITES   =  1'b1;               
    A2F_HWDATAS   =  {(DATAWIDTH){1'b0}};

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg2  = "Data Phase";
	ahb_bfm_msg3  = "IDLE";
	ahb_bfm_msg4  = "Waiting for Slave";

    A2F_HADDRS    =  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL      =  1'b0;                 
    A2F_HTRANSS   =  2'h0;                 
    A2F_HSIZES    =  3'h0;                 
    A2F_HWDATAS   =  TARGET_DATA;          
    A2F_HWRITES   =  1'b0;                 

	while (A2F_HREADYOUTS == 1'b0)
    begin
        @(posedge A2F_HCLK) #STD_CLK_DLY;
    end

    A2F_HREADYS   =  1'b0;             
    TARGET_DATA   = A2F_HRDATAS;       

	ahb_bfm_msg1 <= "NO ACTIVITY";
	ahb_bfm_msg2 <= "NO ACTIVITY";
	ahb_bfm_msg3 <= "NO ACTIVITY";
	ahb_bfm_msg4 <= "NO ACTIVITY";
	ahb_bfm_msg5 <= "NO ACTIVITY";
	ahb_bfm_msg6 <= "NO ACTIVITY";

end
endtask


task ahb_read_al4s3b_fabric_mod;
input   [ADDRWIDTH-1:0]	TARGET_ADDRESS;        
input             [2:0]	TARGET_XFR_SIZE;       
output  [DATAWIDTH-1:0]	TARGET_DATA;           

reg     [DATAWIDTH-1:0]   read_data;

integer i, j, k;

begin

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg1  = "AHB Single Read";
	ahb_bfm_msg2  = "Address Phase";
	ahb_bfm_msg3  = "SEQ";

    A2F_HADDRS    =  {TARGET_ADDRESS[ADDRWIDTH-1:11],(TARGET_ADDRESS[10:0] << 2)} ;       

    A2F_HSEL      =  1'b1;                 
    A2F_HREADYS   =  1'b1;                 
    A2F_HTRANSS   =  2'h2;                 

    A2F_HSIZES    =  TARGET_XFR_SIZE;      

    A2F_HWRITES   =  1'b0;                 
    A2F_HWDATAS   =  {(DATAWIDTH){1'b0}};  

    @(posedge A2F_HCLK) #STD_CLK_DLY;

	ahb_bfm_msg2  = "Data Phase";
	ahb_bfm_msg3  = "IDLE";
	ahb_bfm_msg4  = "Waiting for Slave";

    A2F_HADDRS    =  DEFAULT_AHB_ADDRESS;  
    A2F_HSEL      =  1'b0;                 
    A2F_HTRANSS   =  2'h0;                 
    A2F_HSIZES    =  3'h0;                 

	while (A2F_HREADYOUTS == 1'b0)
    begin
        @(posedge A2F_HCLK) #STD_CLK_DLY;
    end

    A2F_HREADYS   =  1'b0;             
    TARGET_DATA   = A2F_HRDATAS;       

	ahb_bfm_msg1 <= "NO ACTIVITY";
	ahb_bfm_msg2 <= "NO ACTIVITY";
	ahb_bfm_msg3 <= "NO ACTIVITY";
	ahb_bfm_msg4 <= "NO ACTIVITY";
	ahb_bfm_msg5 <= "NO ACTIVITY";
	ahb_bfm_msg6 <= "NO ACTIVITY";

end
endtask
`endif

endmodule

`timescale 1ns/10ps
module	oscillator_s1 
		(

		OSC_CLK_EN,
		OSC_CLK

		);

parameter 		T_CYCLE_CLK = (1000.0/19.2);

input			OSC_CLK_EN;
output			OSC_CLK;

wire			OSC_CLK_EN;
wire			OSC_CLK;

reg				osc_int_clk;


assign	OSC_CLK = OSC_CLK_EN ? osc_int_clk : 1'bZ;

initial
begin
	osc_int_clk	= 0;	
`ifdef GSIM
	forever				
	begin
		#(T_CYCLE_CLK/2) osc_int_clk = 1;
		#(T_CYCLE_CLK/2) osc_int_clk = 0;
	end 
`endif
end

endmodule

`timescale 1ns/10ps
module sdma_bfm       (

  						sdma_req_i,
						sdma_sreq_i,
						sdma_done_o,
						sdma_active_o

			             );
						 
input    [3:0]  	sdma_req_i;											 
input    [3:0]  	sdma_sreq_i;												 
output   [3:0]  	sdma_done_o;						
output   [3:0]  	sdma_active_o;	

reg [3:0] sdma_done_sig;
reg [3:0] sdma_active_sig;

assign sdma_done_o 		= sdma_done_sig;
assign sdma_active_o 	= sdma_active_sig;

initial 
begin
sdma_done_sig 	<= 4'h0;
sdma_active_sig <= 4'h0;

end

`ifdef GSIM
task drive_dma_active;
input [3:0] dma_active_i;
begin
    sdma_active_sig <= dma_active_i;
	#100;
end
endtask
`endif
endmodule					

`timescale 1ns / 10ps
module ahb2fb_asynbrig_if (

                           A2F_HCLK,       
                           A2F_HRESET,     

                           A2F_HSEL,
                           A2F_HADDRS,
                           A2F_HTRANSS,
                           A2F_HSIZES,
                           A2F_HWRITES,
                           A2F_HREADYS,

                           A2F_HREADYOUTS,
                           A2F_HRESPS,

                           AHB_ASYNC_ADDR_O,
                           AHB_ASYNC_READ_EN_O,
                           AHB_ASYNC_WRITE_EN_O,
                           AHB_ASYNC_BYTE_STROBE_O,

                           AHB_ASYNC_STB_TOGGLE_O,

                           FABRIC_ASYNC_ACK_TOGGLE_I

					       );


    parameter    DATAWIDTH              = 32;
	parameter    APERWIDTH              = 17;

	parameter    STATE_WIDTH            = 1;

	parameter    AHB_ASYNC_IDLE         = 0;
	parameter    AHB_ASYNC_WAIT         = 1;

    input                    A2F_HCLK;       
    input                    A2F_HRESET;     

    input    [APERWIDTH-1:0] A2F_HADDRS;
    input                    A2F_HSEL;
    input              [1:0] A2F_HTRANSS;
    input              [2:0] A2F_HSIZES;
    input                    A2F_HWRITES;
    input                    A2F_HREADYS;

    output                   A2F_HREADYOUTS;
    output                   A2F_HRESPS;

    output   [APERWIDTH-1:0] AHB_ASYNC_ADDR_O;
    output                   AHB_ASYNC_READ_EN_O;
    output                   AHB_ASYNC_WRITE_EN_O;
    output             [3:0] AHB_ASYNC_BYTE_STROBE_O;

    output                   AHB_ASYNC_STB_TOGGLE_O;

    input                    FABRIC_ASYNC_ACK_TOGGLE_I;

    wire                     A2F_HCLK;      
    wire                     A2F_HRESET;    

    wire     [APERWIDTH-1:0] A2F_HADDRS;
    wire                     A2F_HSEL;
    wire               [1:0] A2F_HTRANSS;
    wire               [2:0] A2F_HSIZES;
    wire                     A2F_HWRITES;
    wire                     A2F_HREADYS;

    reg                      A2F_HREADYOUTS;
    reg                      A2F_HREADYOUTS_nxt;

    wire                     A2F_HRESPS;

    reg      [APERWIDTH-1:0] AHB_ASYNC_ADDR_O;
    reg                      AHB_ASYNC_READ_EN_O;
    reg                      AHB_ASYNC_WRITE_EN_O;

    reg                [3:0] AHB_ASYNC_BYTE_STROBE_O;
    reg                [3:0] AHB_ASYNC_BYTE_STROBE_O_nxt;



    reg                      AHB_ASYNC_STB_TOGGLE_O;
    reg                      AHB_ASYNC_STB_TOGGLE_O_nxt;

    wire                     FABRIC_ASYNC_ACK_TOGGLE_I;

    wire                     trans_req;           

	reg    [STATE_WIDTH-1:0] ahb_to_fabric_state;
	reg    [STATE_WIDTH-1:0] ahb_to_fabric_state_nxt;

    reg                      fabric_async_ack_toggle_i_1ff;
    reg                      fabric_async_ack_toggle_i_2ff;
    reg                      fabric_async_ack_toggle_i_3ff;

    wire                     fabric_async_ack;

    assign trans_req        =   A2F_HSEL
	                        &   A2F_HREADYS 
						    &   A2F_HTRANSS[1]; 
                                                

    assign fabric_async_ack = fabric_async_ack_toggle_i_2ff ^ fabric_async_ack_toggle_i_3ff;

    assign A2F_HRESPS       = 1'b0;  

    always @(posedge A2F_HCLK or posedge A2F_HRESET)
    begin
        if (A2F_HRESET)
        begin
	       ahb_to_fabric_state           <=   AHB_ASYNC_IDLE;

           AHB_ASYNC_ADDR_O              <=   {(APERWIDTH){1'b0}}; 
           AHB_ASYNC_READ_EN_O           <=   1'b0;
           AHB_ASYNC_WRITE_EN_O          <=   1'b0;
           AHB_ASYNC_BYTE_STROBE_O       <=   4'b0;

           AHB_ASYNC_STB_TOGGLE_O        <=   1'b0;

           fabric_async_ack_toggle_i_1ff <=   1'b0;
           fabric_async_ack_toggle_i_2ff <=   1'b0;
           fabric_async_ack_toggle_i_3ff <=   1'b0;

           A2F_HREADYOUTS                <=   1'b0;
        end
        else 
        begin
	       ahb_to_fabric_state           <=   ahb_to_fabric_state_nxt;

           if (trans_req)
           begin
               AHB_ASYNC_ADDR_O          <=   A2F_HADDRS[APERWIDTH-1:0];
               AHB_ASYNC_READ_EN_O       <=  ~A2F_HWRITES ;
               AHB_ASYNC_WRITE_EN_O      <=   A2F_HWRITES ;
               AHB_ASYNC_BYTE_STROBE_O   <=   AHB_ASYNC_BYTE_STROBE_O_nxt;
		   end

           AHB_ASYNC_STB_TOGGLE_O        <=   AHB_ASYNC_STB_TOGGLE_O_nxt;

           fabric_async_ack_toggle_i_1ff <=   FABRIC_ASYNC_ACK_TOGGLE_I;
           fabric_async_ack_toggle_i_2ff <=   fabric_async_ack_toggle_i_1ff;
           fabric_async_ack_toggle_i_3ff <=   fabric_async_ack_toggle_i_2ff;

           A2F_HREADYOUTS                <=   A2F_HREADYOUTS_nxt;
        end
    end

    always @(A2F_HSIZES or A2F_HADDRS)
    begin
        case(A2F_HSIZES)
        3'b000:                                
        begin
           case(A2F_HADDRS[1:0])
           2'b00:    AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b0001;
           2'b01:    AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b0010;
           2'b10:    AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b0100;
           2'b11:    AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b1000;
           default:  AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b0000;
           endcase
        end
        3'b001:                                 
        begin
            case(A2F_HADDRS[1])
            1'b0:    AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b0011;
            1'b1:    AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b1100;
            default: AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b0000;
	        endcase
        end
        default:     AHB_ASYNC_BYTE_STROBE_O_nxt <= 4'b1111; 
        endcase
    end

    always @(
            trans_req              or
            fabric_async_ack       or
            AHB_ASYNC_STB_TOGGLE_O or
            ahb_to_fabric_state
    )
    begin
	    case(ahb_to_fabric_state)
	    AHB_ASYNC_IDLE:
        begin
            case(trans_req)
            1'b0:  
            begin
	            ahb_to_fabric_state_nxt    <=  AHB_ASYNC_IDLE;
                A2F_HREADYOUTS_nxt         <=  1'b1;
                AHB_ASYNC_STB_TOGGLE_O_nxt <=  AHB_ASYNC_STB_TOGGLE_O;
            end 
            1'b1: 
            begin
	            ahb_to_fabric_state_nxt    <=  AHB_ASYNC_WAIT;
                A2F_HREADYOUTS_nxt         <=  1'b0;
                AHB_ASYNC_STB_TOGGLE_O_nxt <= ~AHB_ASYNC_STB_TOGGLE_O;
            end 
            endcase
        end
	    AHB_ASYNC_WAIT:
        begin
            AHB_ASYNC_STB_TOGGLE_O_nxt     <=  AHB_ASYNC_STB_TOGGLE_O;

            case(fabric_async_ack)
            1'b0:  
            begin
	            ahb_to_fabric_state_nxt    <=  AHB_ASYNC_WAIT;
                A2F_HREADYOUTS_nxt         <=  1'b0;
            end 
            1'b1:  
            begin
	            ahb_to_fabric_state_nxt    <=  AHB_ASYNC_IDLE;
                A2F_HREADYOUTS_nxt         <=  1'b1;
            end 
            endcase
        end
        default:        
        begin
	        ahb_to_fabric_state_nxt        <=  AHB_ASYNC_IDLE;
            A2F_HREADYOUTS_nxt             <=  1'b0;
            AHB_ASYNC_STB_TOGGLE_O_nxt     <=  AHB_ASYNC_STB_TOGGLE_O;
        end
        endcase
    end
    
endmodule

`timescale 1ns / 10ps
module fb2ahb_asynbrig_if (

                           A2F_HRDATAS,

                           AHB_ASYNC_READ_EN_I,
                           AHB_ASYNC_WRITE_EN_I,
                           AHB_ASYNC_BYTE_STROBE_I,

                           AHB_ASYNC_STB_TOGGLE_I,

                           WB_CLK_I,
                           WB_RST_I,
                           WB_ACK_I,
                           WB_DAT_I,
    
                           WB_CYC_O,
                           WB_BYTE_STB_O,
                           WB_WE_O,
                           WB_RD_O,
                           WB_STB_O,

                           FABRIC_ASYNC_ACK_TOGGLE_O

					       );


    parameter    DATAWIDTH              = 32;

	parameter    STATE_WIDTH            = 1;

	parameter    FAB_ASYNC_IDLE         = 0;
	parameter    FAB_ASYNC_WAIT         = 1;

    output   [DATAWIDTH-1:0] A2F_HRDATAS;

    input                    AHB_ASYNC_READ_EN_I;
    input                    AHB_ASYNC_WRITE_EN_I;
    input              [3:0] AHB_ASYNC_BYTE_STROBE_I;

    input                    AHB_ASYNC_STB_TOGGLE_I;


    input                    WB_CLK_I;
    input                    WB_RST_I;
    input                    WB_ACK_I;
    input    [DATAWIDTH-1:0] WB_DAT_I;
    
    output                   WB_CYC_O;
    output             [3:0] WB_BYTE_STB_O;
    output                   WB_WE_O;
    output                   WB_RD_O;
    output                   WB_STB_O;

    output                   FABRIC_ASYNC_ACK_TOGGLE_O;


    reg      [DATAWIDTH-1:0] A2F_HRDATAS;
    reg      [DATAWIDTH-1:0] A2F_HRDATAS_nxt;

    wire                     AHB_ASYNC_READ_EN_I;
    wire                     AHB_ASYNC_WRITE_EN_I;

    wire               [3:0] AHB_ASYNC_BYTE_STROBE_I;

    wire                     AHB_ASYNC_STB_TOGGLE_I;


    wire                     WB_CLK_I;
    wire                     WB_RST_I;
    wire                     WB_ACK_I;
    
    reg                      WB_CYC_O;
    reg                      WB_CYC_O_nxt;

    reg                [3:0] WB_BYTE_STB_O;
    reg                [3:0] WB_BYTE_STB_O_nxt;

    reg                      WB_WE_O;
    reg                      WB_WE_O_nxt;

    reg                      WB_RD_O;
    reg                      WB_RD_O_nxt;

    reg                      WB_STB_O;
    reg                      WB_STB_O_nxt;

    reg                      FABRIC_ASYNC_ACK_TOGGLE_O;
    reg                      FABRIC_ASYNC_ACK_TOGGLE_O_nxt;

	reg    [STATE_WIDTH-1:0] fabric_to_ahb_state;
	reg    [STATE_WIDTH-1:0] fabric_to_ahb_state_nxt;

    reg                      ahb_async_stb_toggle_i_1ff;
    reg                      ahb_async_stb_toggle_i_2ff;
    reg                      ahb_async_stb_toggle_i_3ff;

    wire                     ahb_async_stb;

    assign ahb_async_stb = ahb_async_stb_toggle_i_2ff ^ ahb_async_stb_toggle_i_3ff;

    always @(posedge WB_CLK_I or posedge WB_RST_I)
    begin
        if (WB_RST_I)
        begin
	        fabric_to_ahb_state         <= FAB_ASYNC_IDLE;

            A2F_HRDATAS                 <= {(DATAWIDTH){1'b0}};

            WB_CYC_O                    <= 1'b0;
            WB_BYTE_STB_O               <= 4'b0;
            WB_WE_O                     <= 1'b0;
            WB_RD_O                     <= 1'b0;
            WB_STB_O                    <= 1'b0;

            FABRIC_ASYNC_ACK_TOGGLE_O   <= 1'b0;

            ahb_async_stb_toggle_i_1ff  <= 1'b0;
            ahb_async_stb_toggle_i_2ff  <= 1'b0;
            ahb_async_stb_toggle_i_3ff  <= 1'b0;

        end
        else 
        begin

	        fabric_to_ahb_state         <=  fabric_to_ahb_state_nxt;

            A2F_HRDATAS                 <=  A2F_HRDATAS_nxt;

            WB_CYC_O                    <=  WB_CYC_O_nxt;
            WB_BYTE_STB_O               <=  WB_BYTE_STB_O_nxt;
            WB_WE_O                     <=  WB_WE_O_nxt;
            WB_RD_O                     <=  WB_RD_O_nxt;
            WB_STB_O                    <=  WB_STB_O_nxt;

            FABRIC_ASYNC_ACK_TOGGLE_O   <=  FABRIC_ASYNC_ACK_TOGGLE_O_nxt;

            ahb_async_stb_toggle_i_1ff  <=  AHB_ASYNC_STB_TOGGLE_I;
            ahb_async_stb_toggle_i_2ff  <=  ahb_async_stb_toggle_i_1ff;
            ahb_async_stb_toggle_i_3ff  <=  ahb_async_stb_toggle_i_2ff;

        end
    end

    always @(
            ahb_async_stb             or
            AHB_ASYNC_READ_EN_I       or
            AHB_ASYNC_WRITE_EN_I      or
            AHB_ASYNC_BYTE_STROBE_I   or
            A2F_HRDATAS               or
            WB_ACK_I                  or
            WB_DAT_I                  or
            WB_CYC_O                  or
            WB_BYTE_STB_O             or
            WB_WE_O                   or
            WB_RD_O                   or
            WB_STB_O                  or
            FABRIC_ASYNC_ACK_TOGGLE_O or
            fabric_to_ahb_state
    )
    begin
	    case(fabric_to_ahb_state)
	    FAB_ASYNC_IDLE:
        begin
            FABRIC_ASYNC_ACK_TOGGLE_O_nxt     <=  FABRIC_ASYNC_ACK_TOGGLE_O;
            A2F_HRDATAS_nxt                   <=  A2F_HRDATAS;

            case(ahb_async_stb)
            1'b0:  
            begin
	            fabric_to_ahb_state_nxt       <=  FAB_ASYNC_IDLE;

                WB_CYC_O_nxt                  <=  1'b0;
                WB_BYTE_STB_O_nxt             <=  4'b0;
                WB_WE_O_nxt                   <=  1'b0;
                WB_RD_O_nxt                   <=  1'b0;
                WB_STB_O_nxt                  <=  1'b0;

            end 
            1'b1: 
            begin
	            fabric_to_ahb_state_nxt       <=  FAB_ASYNC_WAIT;

                WB_CYC_O_nxt                  <=  1'b1;
                WB_BYTE_STB_O_nxt             <=  AHB_ASYNC_BYTE_STROBE_I;
                WB_WE_O_nxt                   <=  AHB_ASYNC_WRITE_EN_I;
                WB_RD_O_nxt                   <=  AHB_ASYNC_READ_EN_I;
                WB_STB_O_nxt                  <=  1'b1;

            end 
            endcase
        end
	    FAB_ASYNC_WAIT:
        begin

            case(WB_ACK_I)
            1'b0:  
            begin
	            fabric_to_ahb_state_nxt       <=  FAB_ASYNC_WAIT;

                A2F_HRDATAS_nxt               <=  A2F_HRDATAS;

                WB_CYC_O_nxt                  <=  WB_CYC_O;
                WB_BYTE_STB_O_nxt             <=  WB_BYTE_STB_O;
                WB_WE_O_nxt                   <=  WB_WE_O;
                WB_RD_O_nxt                   <=  WB_RD_O;
                WB_STB_O_nxt                  <=  WB_STB_O;

                FABRIC_ASYNC_ACK_TOGGLE_O_nxt <=  FABRIC_ASYNC_ACK_TOGGLE_O;
            end 
            1'b1:  
            begin
	            fabric_to_ahb_state_nxt       <=  FAB_ASYNC_IDLE;

                A2F_HRDATAS_nxt               <=  WB_DAT_I;

                WB_CYC_O_nxt                  <=  1'b0;
                WB_BYTE_STB_O_nxt             <=  4'b0;
                WB_WE_O_nxt                   <=  1'b0;
                WB_RD_O_nxt                   <=  1'b0;
                WB_STB_O_nxt                  <=  1'b0;

                FABRIC_ASYNC_ACK_TOGGLE_O_nxt <= ~FABRIC_ASYNC_ACK_TOGGLE_O;
            end 
            endcase
        end
        default:        
        begin
	        fabric_to_ahb_state_nxt           <=  FAB_ASYNC_IDLE;

            A2F_HRDATAS_nxt                   <=  A2F_HRDATAS;

            WB_CYC_O_nxt                      <=  1'b0;
            WB_BYTE_STB_O_nxt                 <=  4'b0;
            WB_WE_O_nxt                       <=  1'b0;
            WB_RD_O_nxt                       <=  1'b0;
            WB_STB_O_nxt                      <=  1'b0;

            FABRIC_ASYNC_ACK_TOGGLE_O_nxt     <=  FABRIC_ASYNC_ACK_TOGGLE_O;
        end
        endcase
    end
    
endmodule

`timescale 1ns / 10ps
module ahb2fb_asynbrig (

                           A2F_HCLK,
                           A2F_HRESET,

                           A2F_HADDRS,
                           A2F_HSEL,
                           A2F_HTRANSS,
                           A2F_HSIZES,
                           A2F_HWRITES,
                           A2F_HREADYS,
                           A2F_HWDATAS,

                           A2F_HREADYOUTS,
                           A2F_HRESPS,
                           A2F_HRDATAS,

                           WB_CLK_I,
                           WB_RST_I,
                           WB_DAT_I,
                           WB_ACK_I,

                           WB_ADR_O,
                           WB_CYC_O,
                           WB_BYTE_STB_O,
                           WB_WE_O,
                           WB_RD_O,
                           WB_STB_O,
                           WB_DAT_O

				         );

    parameter    ADDRWIDTH              = 32;
    parameter    DATAWIDTH              = 32;
	parameter    APERWIDTH              = 17;


    input                    A2F_HCLK;      
    input                    A2F_HRESET;    

    input    [ADDRWIDTH-1:0] A2F_HADDRS;
    input                    A2F_HSEL;
    input              [1:0] A2F_HTRANSS;
    input              [2:0] A2F_HSIZES;
    input                    A2F_HWRITES;
    input                    A2F_HREADYS;
    input    [DATAWIDTH-1:0] A2F_HWDATAS;

    output                   A2F_HREADYOUTS;
    output                   A2F_HRESPS;
    output   [DATAWIDTH-1:0] A2F_HRDATAS;

    input                    WB_CLK_I;       
    input                    WB_RST_I;       
    input    [DATAWIDTH-1:0] WB_DAT_I;       
    input                    WB_ACK_I;       
    
    output   [APERWIDTH-1:0] WB_ADR_O;       
    output                   WB_CYC_O;       
    output             [3:0] WB_BYTE_STB_O;  
    output                   WB_WE_O;        
    output                   WB_RD_O;        
    output                   WB_STB_O;       
    output   [DATAWIDTH-1:0] WB_DAT_O;       


    wire                     A2F_HCLK;       
    wire                     A2F_HRESET;     

    wire     [ADDRWIDTH-1:0] A2F_HADDRS;
    wire                     A2F_HSEL;
    wire               [1:0] A2F_HTRANSS;
    wire               [2:0] A2F_HSIZES;
    wire                     A2F_HWRITES;
    wire                     A2F_HREADYS;
    wire     [DATAWIDTH-1:0] A2F_HWDATAS;

    wire                     A2F_HREADYOUTS;
    wire                     A2F_HRESPS;
    wire     [DATAWIDTH-1:0] A2F_HRDATAS;

    wire                     WB_CLK_I;        
    wire                     WB_RST_I;        
    wire     [DATAWIDTH-1:0] WB_DAT_I;        
    wire                     WB_ACK_I;        
    
    wire     [APERWIDTH-1:0] WB_ADR_O;        
    wire                     WB_CYC_O;        
    wire               [3:0] WB_BYTE_STB_O;   
    wire                     WB_WE_O;         
    wire                     WB_RD_O;         
    wire                     WB_STB_O;        
    wire     [DATAWIDTH-1:0] WB_DAT_O;        

    wire     [APERWIDTH-1:0] ahb_async_addr;
    wire                     ahb_async_read_en;
    wire                     ahb_async_write_en;
    wire               [3:0] ahb_async_byte_strobe;

    wire                     ahb_async_stb_toggle;

    wire                     fabric_async_ack_toggle;

    assign WB_DAT_O    =  A2F_HWDATAS;

    assign WB_ADR_O    =  ahb_async_addr;

	ahb2fb_asynbrig_if         
	                                   #(

      .DATAWIDTH                        ( DATAWIDTH                           ),
      .APERWIDTH                        ( APERWIDTH                           )

                                        )
      u_FFE_ahb_to_fabric_async_bridge_interface         
                                        (
      .A2F_HCLK                         ( A2F_HCLK                            ),
      .A2F_HRESET                       ( A2F_HRESET                          ),

      .A2F_HSEL                         ( A2F_HSEL                            ),
      .A2F_HADDRS                       ( A2F_HADDRS[APERWIDTH-1:0]           ),
      .A2F_HTRANSS                      ( A2F_HTRANSS                         ),
      .A2F_HSIZES                       ( A2F_HSIZES                          ),
      .A2F_HWRITES                      ( A2F_HWRITES                         ),
      .A2F_HREADYS                      ( A2F_HREADYS                         ),

      .A2F_HREADYOUTS                   ( A2F_HREADYOUTS                      ),
      .A2F_HRESPS                       ( A2F_HRESPS                          ),

      .AHB_ASYNC_ADDR_O                 ( ahb_async_addr                      ),
      .AHB_ASYNC_READ_EN_O              ( ahb_async_read_en                   ),
      .AHB_ASYNC_WRITE_EN_O             ( ahb_async_write_en                  ),
      .AHB_ASYNC_BYTE_STROBE_O          ( ahb_async_byte_strobe               ),
      .AHB_ASYNC_STB_TOGGLE_O           ( ahb_async_stb_toggle                ),

	  .FABRIC_ASYNC_ACK_TOGGLE_I        (fabric_async_ack_toggle              )

      );


    fb2ahb_asynbrig_if      u_FFE_fabric_to_ahb_async_bridge_interface         
                                        (
      .A2F_HRDATAS                      ( A2F_HRDATAS                         ),

      .AHB_ASYNC_READ_EN_I              ( ahb_async_read_en                   ),
      .AHB_ASYNC_WRITE_EN_I             ( ahb_async_write_en                  ),
      .AHB_ASYNC_BYTE_STROBE_I          ( ahb_async_byte_strobe               ),
      .AHB_ASYNC_STB_TOGGLE_I           ( ahb_async_stb_toggle                ),

      .WB_CLK_I                         ( WB_CLK_I                            ), 
      .WB_RST_I                         ( WB_RST_I                            ), 
      .WB_ACK_I                         ( WB_ACK_I                            ), 
      .WB_DAT_I                         ( WB_DAT_I                            ), 
    
      .WB_CYC_O                         ( WB_CYC_O                            ), 
      .WB_BYTE_STB_O                    ( WB_BYTE_STB_O                       ), 
      .WB_WE_O                          ( WB_WE_O                             ), 
      .WB_RD_O                          ( WB_RD_O                             ), 
      .WB_STB_O                         ( WB_STB_O                            ), 

	  .FABRIC_ASYNC_ACK_TOGGLE_O        (fabric_async_ack_toggle              )

      );
endmodule


`timescale 1ns/10ps
module qlal4s3b_cell_macro_bfm (

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
  
                SDMA_Req,
                SDMA_Sreq,
                SDMA_Done,
                SDMA_Active,

                FB_msg_out,
                FB_Int_Clr,
                FB_Start,
                FB_Busy,

                Sys_Clk0,
                Sys_Clk0_Rst,
                Sys_Clk1,
                Sys_Clk1_Rst,
 
                Sys_PKfb_Clk,
                Sys_PKfb_Rst,
                FB_PKfbData,
                FB_PKfbPush,
                FB_PKfbSOF,
                FB_PKfbEOF,
                FB_PKfbOverflow,

                Sensor_Int,
                TimeStamp,

                Sys_Pclk,
                Sys_Pclk_Rst,
                Sys_PSel,
                SPIm_Paddr,
                SPIm_PEnable,
                SPIm_PWrite,
                SPIm_PWdata,
                SPIm_Prdata,
                SPIm_PReady,
                SPIm_PSlvErr,

                Device_ID,

                FBIO_In,
                FBIO_In_En,
                FBIO_Out,
                FBIO_Out_En,

                SFBIO,
                Device_ID_6S,
                Device_ID_4S,
                SPIm_PWdata_26S,
                SPIm_PWdata_24S,
                SPIm_PWdata_14S,
                SPIm_PWdata_11S,
                SPIm_PWdata_0S,
                SPIm_Paddr_8S,
                SPIm_Paddr_6S,
                FB_PKfbPush_1S,
                FB_PKfbData_31S,
                FB_PKfbData_21S,
                FB_PKfbData_19S,
                FB_PKfbData_9S,
                FB_PKfbData_6S,
                Sys_PKfb_ClkS,
                FB_BusyS,
                WB_CLKS
                );

output  [16:0]  WBs_ADR;
output          WBs_CYC;
output   [3:0]  WBs_BYTE_STB;
output          WBs_WE;
output          WBs_RD;
output          WBs_STB;
output  [31:0]  WBs_WR_DAT;
input           WB_CLK;
output          WB_RST;
input   [31:0]  WBs_RD_DAT;
input           WBs_ACK;

input    [3:0]  SDMA_Req;
input    [3:0]  SDMA_Sreq;
output   [3:0]  SDMA_Done;
output   [3:0]  SDMA_Active;

input    [3:0]  FB_msg_out;
input    [7:0]  FB_Int_Clr;
output          FB_Start;
input           FB_Busy;

output          Sys_Clk0;
output          Sys_Clk0_Rst;
output          Sys_Clk1;
output          Sys_Clk1_Rst;

input           Sys_PKfb_Clk;
output          Sys_PKfb_Rst;
input   [31:0]  FB_PKfbData;
input    [3:0]  FB_PKfbPush;
input           FB_PKfbSOF;
input           FB_PKfbEOF;
output          FB_PKfbOverflow;

output   [7:0]  Sensor_Int;
output  [23:0]  TimeStamp;

output          Sys_Pclk;
output          Sys_Pclk_Rst;
input           Sys_PSel;
input   [15:0]  SPIm_Paddr;
input           SPIm_PEnable;
input           SPIm_PWrite;
input   [31:0]  SPIm_PWdata;
output  [31:0]  SPIm_Prdata;
output          SPIm_PReady;
output          SPIm_PSlvErr;

input   [15:0]  Device_ID;

output  [13:0]  FBIO_In;
input   [13:0]  FBIO_In_En;
input   [13:0]  FBIO_Out;
input   [13:0]  FBIO_Out_En;

inout   [13:0]  SFBIO;
input           Device_ID_6S; 
input           Device_ID_4S; 
input           SPIm_PWdata_26S; 
input           SPIm_PWdata_24S;  
input           SPIm_PWdata_14S; 
input           SPIm_PWdata_11S; 
input           SPIm_PWdata_0S; 
input           SPIm_Paddr_8S; 
input           SPIm_Paddr_6S; 
input           FB_PKfbPush_1S; 
input           FB_PKfbData_31S; 
input           FB_PKfbData_21S;
input           FB_PKfbData_19S;
input           FB_PKfbData_9S;
input           FB_PKfbData_6S;
input           Sys_PKfb_ClkS;
input           FB_BusyS;
input           WB_CLKS;


wire    [16:0]  WBs_ADR;
wire            WBs_CYC;
wire     [3:0]  WBs_BYTE_STB;
wire            WBs_WE;
wire            WBs_RD;
wire            WBs_STB;
wire    [31:0]  WBs_WR_DAT;
wire            WB_CLK;
reg             WB_RST;
wire    [31:0]  WBs_RD_DAT;
wire            WBs_ACK;

wire     [3:0]  SDMA_Req;
wire     [3:0]  SDMA_Sreq;
wire      [3:0]  SDMA_Done;
wire      [3:0]  SDMA_Active;

wire     [3:0]  FB_msg_out;
wire     [7:0]  FB_Int_Clr;
reg             FB_Start;
wire            FB_Busy;

wire            Sys_Clk0;
reg             Sys_Clk0_Rst;
wire            Sys_Clk1;
reg             Sys_Clk1_Rst;

wire            Sys_PKfb_Clk;
reg             Sys_PKfb_Rst;
wire    [31:0]  FB_PKfbData;
wire     [3:0]  FB_PKfbPush;
wire            FB_PKfbSOF;
wire            FB_PKfbEOF;
reg             FB_PKfbOverflow;

reg      [7:0]  Sensor_Int;
reg     [23:0]  TimeStamp;

reg             Sys_Pclk;
reg             Sys_Pclk_Rst;
wire            Sys_PSel;

wire    [15:0]  SPIm_Paddr;
wire            SPIm_PEnable;
wire            SPIm_PWrite;
wire    [31:0]  SPIm_PWdata;
reg     [31:0]  SPIm_Prdata;
reg             SPIm_PReady;
reg             SPIm_PSlvErr;

wire    [15:0]  Device_ID;

reg     [13:0]  FBIO_In;
wire    [13:0]  FBIO_In_En;
wire    [13:0]  FBIO_Out;
wire    [13:0]  FBIO_Out_En;

wire    [13:0]  SFBIO;
wire            Device_ID_6S; 
wire            Device_ID_4S; 

wire            SPIm_PWdata_26S; 
wire            SPIm_PWdata_24S;  
wire            SPIm_PWdata_14S; 
wire            SPIm_PWdata_11S; 
wire            SPIm_PWdata_0S; 
wire            SPIm_Paddr_8S; 
wire            SPIm_Paddr_6S; 

wire            FB_PKfbPush_1S; 
wire            FB_PKfbData_31S; 
wire            FB_PKfbData_21S;
wire            FB_PKfbData_19S;
wire            FB_PKfbData_9S;
wire            FB_PKfbData_6S;
wire            Sys_PKfb_ClkS;

wire            FB_BusyS;
wire            WB_CLKS;

parameter       ADDRWIDTH                   = 32;
parameter       DATAWIDTH                   = 32;
parameter       APERWIDTH                   = 17;

parameter       ENABLE_AHB_REG_WR_DEBUG_MSG = 1'b1;
parameter       ENABLE_AHB_REG_RD_DEBUG_MSG = 1'b1; 

parameter       T_CYCLE_CLK_SYS_CLK0        = 200;
parameter       T_CYCLE_CLK_SYS_CLK1        = 650;
parameter       T_CYCLE_CLK_A2F_HCLK        = (1000.0/(80.0/12)) ; 

parameter       SYS_CLK0_RESET_LOOP         = 5;
parameter       SYS_CLK1_RESET_LOOP         = 5;
parameter       WB_CLK_RESET_LOOP           = 5;
parameter       A2F_HCLK_RESET_LOOP         = 5;

integer         Sys_Clk0_Reset_Loop_Cnt;
integer         Sys_Clk1_Reset_Loop_Cnt;
integer         WB_CLK_Reset_Loop_Cnt;
integer         A2F_HCLK_Reset_Loop_Cnt;


wire            A2F_HCLK;
reg             A2F_HRESET;

wire    [31:0]  A2F_HADDRS;
wire            A2F_HSEL;
wire     [1:0]  A2F_HTRANSS;
wire     [2:0]  A2F_HSIZES;
wire            A2F_HWRITES;
wire            A2F_HREADYS;
wire    [31:0]  A2F_HWDATAS;

wire            A2F_HREADYOUTS;
wire            A2F_HRESPS;
wire    [31:0]  A2F_HRDATAS;

initial
begin

    Sys_Clk0_Rst  <= 1'b1;
`ifdef GSIM
    for (Sys_Clk0_Reset_Loop_Cnt = 0; 
	     Sys_Clk0_Reset_Loop_Cnt < SYS_CLK0_RESET_LOOP; 
         Sys_Clk0_Reset_Loop_Cnt = Sys_Clk0_Reset_Loop_Cnt + 1)
    begin
        wait (Sys_Clk0 == 1'b1) #1;
        wait (Sys_Clk0 == 1'b0) #1;
    end

    wait (Sys_Clk0 == 1'b1) #1;
`endif
    Sys_Clk0_Rst  <= 1'b0;

end

initial
begin

    Sys_Clk1_Rst  <= 1'b1;
`ifdef GSIM
    for (Sys_Clk1_Reset_Loop_Cnt = 0; 
	     Sys_Clk1_Reset_Loop_Cnt < SYS_CLK1_RESET_LOOP; 
         Sys_Clk1_Reset_Loop_Cnt = Sys_Clk1_Reset_Loop_Cnt + 1)
    begin
        wait (Sys_Clk1 == 1'b1) #1;
        wait (Sys_Clk1 == 1'b0) #1;
    end

    wait (Sys_Clk1 == 1'b1) #1;
`endif
    Sys_Clk1_Rst  <= 1'b0;

end

initial
begin

    WB_RST  <= 1'b1;
`ifdef GSIM
    for (WB_CLK_Reset_Loop_Cnt = 0; 
	     WB_CLK_Reset_Loop_Cnt < WB_CLK_RESET_LOOP; 
         WB_CLK_Reset_Loop_Cnt = WB_CLK_Reset_Loop_Cnt + 1)
    begin
        wait (WB_CLK == 1'b1) #1;
        wait (WB_CLK == 1'b0) #1;
    end

    wait (WB_CLK == 1'b1) #1;
`endif
    WB_RST  <= 1'b0;

end

initial
begin

    A2F_HRESET  <= 1'b1;
`ifdef GSIM
    for (A2F_HCLK_Reset_Loop_Cnt = 0; 
	     A2F_HCLK_Reset_Loop_Cnt < A2F_HCLK_RESET_LOOP; 
         A2F_HCLK_Reset_Loop_Cnt = A2F_HCLK_Reset_Loop_Cnt + 1)
    begin
        wait (A2F_HCLK == 1'b1) #1;
        wait (A2F_HCLK == 1'b0) #1;
    end

    wait (A2F_HCLK == 1'b1) #1;
`endif
    A2F_HRESET  <= 1'b0;

end

initial
begin

    FB_Start        <=  1'b0;

    Sys_PKfb_Rst    <=  1'b0;
    FB_PKfbOverflow <=  1'b0;

    Sensor_Int      <=  8'h0;
    TimeStamp       <= 24'h0;

    Sys_Pclk        <=  1'b0;
    Sys_Pclk_Rst    <=  1'b0;

    SPIm_Prdata     <= 32'h0;
    SPIm_PReady     <=  1'b0;
    SPIm_PSlvErr    <=  1'b0;

    FBIO_In         <= 14'h0;

end

ahb2fb_asynbrig 
                                #(
    .ADDRWIDTH                   ( ADDRWIDTH                   ),
    .DATAWIDTH                   ( DATAWIDTH                   ),
	.APERWIDTH                   ( APERWIDTH                   )
	                             )
    u_ffe_ahb_to_fabric_async_bridge 
                                 (

    .A2F_HCLK                    ( A2F_HCLK                    ),
    .A2F_HRESET                  ( A2F_HRESET                  ),

    .A2F_HADDRS                  ( A2F_HADDRS                  ),
    .A2F_HSEL                    ( A2F_HSEL                    ),
    .A2F_HTRANSS                 ( A2F_HTRANSS                 ),
    .A2F_HSIZES                  ( A2F_HSIZES                  ),
    .A2F_HWRITES                 ( A2F_HWRITES                 ),
    .A2F_HREADYS                 ( A2F_HREADYS                 ),
    .A2F_HWDATAS                 ( A2F_HWDATAS                 ),

    .A2F_HREADYOUTS              ( A2F_HREADYOUTS              ),
    .A2F_HRESPS                  ( A2F_HRESPS                  ),
    .A2F_HRDATAS                 ( A2F_HRDATAS                 ),

    .WB_CLK_I                    ( WB_CLK                      ),
    .WB_RST_I                    ( WB_RST                      ),
    .WB_DAT_I                    ( WBs_RD_DAT                  ),
    .WB_ACK_I                    ( WBs_ACK                     ),

    .WB_ADR_O                    ( WBs_ADR                     ),
    .WB_CYC_O                    ( WBs_CYC                     ),
    .WB_BYTE_STB_O               ( WBs_BYTE_STB                ),
    .WB_WE_O                     ( WBs_WE                      ),
    .WB_RD_O                     ( WBs_RD                      ),
    .WB_STB_O                    ( WBs_STB                     ),
    .WB_DAT_O                    ( WBs_WR_DAT                  )

    );


ahb_gen_bfm
                                #(
    .ADDRWIDTH                   ( ADDRWIDTH                   ),
    .DATAWIDTH                   ( DATAWIDTH                   ),
	.DEFAULT_AHB_ADDRESS         ( {(ADDRWIDTH){1'b1}}         ),
	.STD_CLK_DLY                 ( 2                           ),
    .ENABLE_AHB_REG_WR_DEBUG_MSG ( ENABLE_AHB_REG_WR_DEBUG_MSG ),
    .ENABLE_AHB_REG_RD_DEBUG_MSG ( ENABLE_AHB_REG_RD_DEBUG_MSG )
                                 )
    u_ahb_gen_bfm
                                 (
    .A2F_HCLK                    ( A2F_HCLK                    ),
    .A2F_HRESET                  ( A2F_HRESET                  ),

    .A2F_HADDRS                  ( A2F_HADDRS                  ),
    .A2F_HSEL                    ( A2F_HSEL                    ),
    .A2F_HTRANSS                 ( A2F_HTRANSS                 ),
    .A2F_HSIZES                  ( A2F_HSIZES                  ),
    .A2F_HWRITES                 ( A2F_HWRITES                 ),
    .A2F_HREADYS                 ( A2F_HREADYS                 ),
    .A2F_HWDATAS                 ( A2F_HWDATAS                 ),

    .A2F_HREADYOUTS              ( A2F_HREADYOUTS              ),
    .A2F_HRESPS                  ( A2F_HRESPS                  ),
    .A2F_HRDATAS                 ( A2F_HRDATAS                 )

    );

oscillator_s1 #(.T_CYCLE_CLK (T_CYCLE_CLK_SYS_CLK0)) u_osc_sys_clk0   (.OSC_CLK_EN (1'b1), .OSC_CLK (Sys_Clk0));
oscillator_s1 #(.T_CYCLE_CLK (T_CYCLE_CLK_SYS_CLK1)) u_osc_sys_clk1   (.OSC_CLK_EN (1'b1), .OSC_CLK (Sys_Clk1));
oscillator_s1 #(.T_CYCLE_CLK (T_CYCLE_CLK_A2F_HCLK)) u_osc_a2f_hclk   (.OSC_CLK_EN (1'b1), .OSC_CLK (A2F_HCLK));

sdma_bfm sdma_bfm_inst0 (
							.sdma_req_i			( SDMA_Req),
                            .sdma_sreq_i		( SDMA_Sreq),
                            .sdma_done_o		( SDMA_Done),
                            .sdma_active_o		( SDMA_Active)
						);



endmodule 

`timescale 1ns/10ps
(* whitebox *)
(* keep *)
module ASSP(
    input	WB_CLK,
    input	WBs_ACK,
    input	[31:0]WBs_RD_DAT,
    output	[3:0]WBs_BYTE_STB,
    output	WBs_CYC,
    output	WBs_WE,
    output	WBs_RD,
    output	WBs_STB,
    output	[16:0]WBs_ADR,
    input	[3:0]SDMA_Req,
    input	[3:0]SDMA_Sreq,
    output	[3:0]SDMA_Done,
    output	[3:0]SDMA_Active,
    input	[3:0]FB_msg_out,
    input	[7:0]FB_Int_Clr,
    output	FB_Start,
    input	FB_Busy,
    output	WB_RST,
    output	Sys_PKfb_Rst,
    output	Sys_Clk0,
    output	Sys_Clk0_Rst,
    output	Sys_Clk1,
    output	Sys_Clk1_Rst,
    output	Sys_Pclk,
    output	Sys_Pclk_Rst,
    input	Sys_PKfb_Clk,
    input	[31:0]FB_PKfbData,
    output	[31:0]WBs_WR_DAT,
    input	[3:0]FB_PKfbPush,
    input	FB_PKfbSOF,
    input	FB_PKfbEOF,
    output	[7:0]Sensor_Int,
    output	FB_PKfbOverflow,
    output	[23:0]TimeStamp,
    input	Sys_PSel,
    input	[15:0]SPIm_Paddr,
    input	SPIm_PEnable,
    input	SPIm_PWrite,
    input	[31:0]SPIm_PWdata,
    output	SPIm_PReady,
    output	SPIm_PSlvErr,
    output	[31:0]SPIm_Prdata,
    input	[15:0]Device_ID,
    input	[13:0]FBIO_In_En,
    input	[13:0]FBIO_Out,
    input	[13:0]FBIO_Out_En,
    output	[13:0]FBIO_In,
    inout 	[13:0]SFBIO,
    input   Device_ID_6S,
    input   Device_ID_4S,
    input   SPIm_PWdata_26S,
    input   SPIm_PWdata_24S,
    input   SPIm_PWdata_14S,
    input   SPIm_PWdata_11S,
    input   SPIm_PWdata_0S,
    input   SPIm_Paddr_8S,
    input   SPIm_Paddr_6S,
    input   FB_PKfbPush_1S,
    input   FB_PKfbData_31S,
    input   FB_PKfbData_21S,
    input   FB_PKfbData_19S,
    input   FB_PKfbData_9S,
    input   FB_PKfbData_6S,
    input   Sys_PKfb_ClkS,
    input   FB_BusyS,
    input   WB_CLKS);
	
qlal4s3b_cell_macro_bfm	 u_ASSP_bfm_inst(
		.WBs_ADR     (WBs_ADR),
        .WBs_CYC     (WBs_CYC),
        .WBs_BYTE_STB(WBs_BYTE_STB),
        .WBs_WE      (WBs_WE),
        .WBs_RD      (WBs_RD), 
        .WBs_STB     (WBs_STB),
        .WBs_WR_DAT  (WBs_WR_DAT),
        .WB_CLK      (WB_CLK),
        .WB_RST      (WB_RST),
        .WBs_RD_DAT  (WBs_RD_DAT),
        .WBs_ACK     (WBs_ACK),

        .SDMA_Req     (SDMA_Req),
        .SDMA_Sreq    (SDMA_Sreq),
        .SDMA_Done    (SDMA_Done),
        .SDMA_Active  (SDMA_Active),

        .FB_msg_out    (FB_msg_out),
        .FB_Int_Clr    (FB_Int_Clr),
        .FB_Start      (FB_Start),
        .FB_Busy       (FB_Busy),

        .Sys_Clk0      (Sys_Clk0),
        .Sys_Clk0_Rst  (Sys_Clk0_Rst),
        .Sys_Clk1      (Sys_Clk1),
        .Sys_Clk1_Rst  (Sys_Clk1_Rst),

        .Sys_PKfb_Clk     (Sys_PKfb_Clk),
        .Sys_PKfb_Rst     (Sys_PKfb_Rst),
        .FB_PKfbData      (FB_PKfbData),
        .FB_PKfbPush      (FB_PKfbPush),
        .FB_PKfbSOF       (FB_PKfbSOF),
        .FB_PKfbEOF       (FB_PKfbEOF),
        .FB_PKfbOverflow  (FB_PKfbOverflow),

        .Sensor_Int       (Sensor_Int),
        .TimeStamp        (TimeStamp),

        .Sys_Pclk        (Sys_Pclk),
        .Sys_Pclk_Rst    (Sys_Pclk_Rst),
        .Sys_PSel        (Sys_PSel),
        .SPIm_Paddr      (SPIm_Paddr),
        .SPIm_PEnable    (SPIm_PEnable),
        .SPIm_PWrite     (SPIm_PWrite),
        .SPIm_PWdata     (SPIm_PWdata),
        .SPIm_Prdata     (SPIm_Prdata),
        .SPIm_PReady     (SPIm_PReady),
        .SPIm_PSlvErr    (SPIm_PSlvErr),

        .Device_ID		 (Device_ID),

        .FBIO_In         (FBIO_In),
        .FBIO_In_En      (FBIO_In_En),
        .FBIO_Out        (FBIO_Out),
        .FBIO_Out_En     (FBIO_Out_En),

        .SFBIO              (SFBIO),
        .Device_ID_6S       (Device_ID_6S),
        .Device_ID_4S       (Device_ID_4S),
        .SPIm_PWdata_26S    (SPIm_PWdata_26S),
        .SPIm_PWdata_24S    (SPIm_PWdata_24S),
        .SPIm_PWdata_14S    (SPIm_PWdata_14S),
        .SPIm_PWdata_11S    (SPIm_PWdata_11S),
        .SPIm_PWdata_0S     (SPIm_PWdata_0S),
        .SPIm_Paddr_8S      (SPIm_Paddr_8S),
        .SPIm_Paddr_6S      (SPIm_Paddr_6S),
        .FB_PKfbPush_1S     (FB_PKfbPush_1S),
        .FB_PKfbData_31S    (FB_PKfbData_31S),
        .FB_PKfbData_21S    (FB_PKfbData_21S),
        .FB_PKfbData_19S    (FB_PKfbData_19S),
        .FB_PKfbData_9S     (FB_PKfbData_9S),
        .FB_PKfbData_6S     (FB_PKfbData_6S),
        .Sys_PKfb_ClkS      (Sys_PKfb_ClkS),
        .FB_BusyS           (FB_BusyS),
        .WB_CLKS            (WB_CLKS)
		);

endmodule 
// ../../../quicklogic/pp3/primitives/assp/assp_bfm.sim.v }}}

// ../../../quicklogic/pp3/primitives/mult/mult.sim.v {{{
`timescale 1ns/10ps
(* whitebox *)
module MULT (
			Amult,
			Bmult,
			Valid_mult,
			Cmult,
			sel_mul_32x32
			);

	input wire  [31:0] Amult;
	input wire  [31:0] Bmult;
	input wire   [1:0] Valid_mult;
	(* DELAY_MATRIX_Amult="{iopath_Amult0_Cmult0} {iopath_Amult0_Cmult1} {iopath_Amult0_Cmult2} {iopath_Amult0_Cmult3} {iopath_Amult0_Cmult4} {iopath_Amult0_Cmult5} {iopath_Amult0_Cmult6} {iopath_Amult0_Cmult7} {iopath_Amult0_Cmult8} {iopath_Amult0_Cmult9} {iopath_Amult0_Cmult10} {iopath_Amult0_Cmult11} {iopath_Amult0_Cmult12} {iopath_Amult0_Cmult13} {iopath_Amult0_Cmult14} {iopath_Amult0_Cmult15} {iopath_Amult0_Cmult16} {iopath_Amult0_Cmult17} {iopath_Amult0_Cmult18} {iopath_Amult0_Cmult19} {iopath_Amult0_Cmult20} {iopath_Amult0_Cmult21} {iopath_Amult0_Cmult22} {iopath_Amult0_Cmult23} {iopath_Amult0_Cmult24} {iopath_Amult0_Cmult25} {iopath_Amult0_Cmult26} {iopath_Amult0_Cmult27} {iopath_Amult0_Cmult28} {iopath_Amult0_Cmult29} {iopath_Amult0_Cmult30} {iopath_Amult0_Cmult31} {iopath_Amult0_Cmult32} {iopath_Amult0_Cmult33} {iopath_Amult0_Cmult34} {iopath_Amult0_Cmult35} {iopath_Amult0_Cmult36} {iopath_Amult0_Cmult37} {iopath_Amult0_Cmult38} {iopath_Amult0_Cmult39} {iopath_Amult0_Cmult40} {iopath_Amult0_Cmult41} {iopath_Amult0_Cmult42} {iopath_Amult0_Cmult43} {iopath_Amult0_Cmult44} {iopath_Amult0_Cmult45} {iopath_Amult0_Cmult46} {iopath_Amult0_Cmult47} {iopath_Amult0_Cmult48} {iopath_Amult0_Cmult49} {iopath_Amult0_Cmult50} {iopath_Amult0_Cmult51} {iopath_Amult0_Cmult52} {iopath_Amult0_Cmult53} {iopath_Amult0_Cmult54} {iopath_Amult0_Cmult55} {iopath_Amult0_Cmult56} {iopath_Amult0_Cmult57} {iopath_Amult0_Cmult58} {iopath_Amult0_Cmult59} {iopath_Amult0_Cmult60} {iopath_Amult0_Cmult61} {iopath_Amult0_Cmult62} {iopath_Amult0_Cmult63} 0 {iopath_Amult1_Cmult1} {iopath_Amult1_Cmult2} {iopath_Amult1_Cmult3} {iopath_Amult1_Cmult4} {iopath_Amult1_Cmult5} {iopath_Amult1_Cmult6} {iopath_Amult1_Cmult7} {iopath_Amult1_Cmult8} {iopath_Amult1_Cmult9} {iopath_Amult1_Cmult10} {iopath_Amult1_Cmult11} {iopath_Amult1_Cmult12} {iopath_Amult1_Cmult13} {iopath_Amult1_Cmult14} {iopath_Amult1_Cmult15} {iopath_Amult1_Cmult16} {iopath_Amult1_Cmult17} {iopath_Amult1_Cmult18} {iopath_Amult1_Cmult19} {iopath_Amult1_Cmult20} {iopath_Amult1_Cmult21} {iopath_Amult1_Cmult22} {iopath_Amult1_Cmult23} {iopath_Amult1_Cmult24} {iopath_Amult1_Cmult25} {iopath_Amult1_Cmult26} {iopath_Amult1_Cmult27} {iopath_Amult1_Cmult28} {iopath_Amult1_Cmult29} {iopath_Amult1_Cmult30} {iopath_Amult1_Cmult31} {iopath_Amult1_Cmult32} {iopath_Amult1_Cmult33} {iopath_Amult1_Cmult34} {iopath_Amult1_Cmult35} {iopath_Amult1_Cmult36} {iopath_Amult1_Cmult37} {iopath_Amult1_Cmult38} {iopath_Amult1_Cmult39} {iopath_Amult1_Cmult40} {iopath_Amult1_Cmult41} {iopath_Amult1_Cmult42} {iopath_Amult1_Cmult43} {iopath_Amult1_Cmult44} {iopath_Amult1_Cmult45} {iopath_Amult1_Cmult46} {iopath_Amult1_Cmult47} {iopath_Amult1_Cmult48} {iopath_Amult1_Cmult49} {iopath_Amult1_Cmult50} {iopath_Amult1_Cmult51} {iopath_Amult1_Cmult52} {iopath_Amult1_Cmult53} {iopath_Amult1_Cmult54} {iopath_Amult1_Cmult55} {iopath_Amult1_Cmult56} {iopath_Amult1_Cmult57} {iopath_Amult1_Cmult58} {iopath_Amult1_Cmult59} {iopath_Amult1_Cmult60} {iopath_Amult1_Cmult61} {iopath_Amult1_Cmult62} {iopath_Amult1_Cmult63} 0 0 {iopath_Amult2_Cmult2} {iopath_Amult2_Cmult3} {iopath_Amult2_Cmult4} {iopath_Amult2_Cmult5} {iopath_Amult2_Cmult6} {iopath_Amult2_Cmult7} {iopath_Amult2_Cmult8} {iopath_Amult2_Cmult9} {iopath_Amult2_Cmult10} {iopath_Amult2_Cmult11} {iopath_Amult2_Cmult12} {iopath_Amult2_Cmult13} {iopath_Amult2_Cmult14} {iopath_Amult2_Cmult15} {iopath_Amult2_Cmult16} {iopath_Amult2_Cmult17} {iopath_Amult2_Cmult18} {iopath_Amult2_Cmult19} {iopath_Amult2_Cmult20} {iopath_Amult2_Cmult21} {iopath_Amult2_Cmult22} {iopath_Amult2_Cmult23} {iopath_Amult2_Cmult24} {iopath_Amult2_Cmult25} {iopath_Amult2_Cmult26} {iopath_Amult2_Cmult27} {iopath_Amult2_Cmult28} {iopath_Amult2_Cmult29} {iopath_Amult2_Cmult30} {iopath_Amult2_Cmult31} {iopath_Amult2_Cmult32} {iopath_Amult2_Cmult33} {iopath_Amult2_Cmult34} {iopath_Amult2_Cmult35} {iopath_Amult2_Cmult36} {iopath_Amult2_Cmult37} {iopath_Amult2_Cmult38} {iopath_Amult2_Cmult39} {iopath_Amult2_Cmult40} {iopath_Amult2_Cmult41} {iopath_Amult2_Cmult42} {iopath_Amult2_Cmult43} {iopath_Amult2_Cmult44} {iopath_Amult2_Cmult45} {iopath_Amult2_Cmult46} {iopath_Amult2_Cmult47} {iopath_Amult2_Cmult48} {iopath_Amult2_Cmult49} {iopath_Amult2_Cmult50} {iopath_Amult2_Cmult51} {iopath_Amult2_Cmult52} {iopath_Amult2_Cmult53} {iopath_Amult2_Cmult54} {iopath_Amult2_Cmult55} {iopath_Amult2_Cmult56} {iopath_Amult2_Cmult57} {iopath_Amult2_Cmult58} {iopath_Amult2_Cmult59} {iopath_Amult2_Cmult60} {iopath_Amult2_Cmult61} {iopath_Amult2_Cmult62} {iopath_Amult2_Cmult63} 0 0 0 {iopath_Amult3_Cmult3} {iopath_Amult3_Cmult4} {iopath_Amult3_Cmult5} {iopath_Amult3_Cmult6} {iopath_Amult3_Cmult7} {iopath_Amult3_Cmult8} {iopath_Amult3_Cmult9} {iopath_Amult3_Cmult10} {iopath_Amult3_Cmult11} {iopath_Amult3_Cmult12} {iopath_Amult3_Cmult13} {iopath_Amult3_Cmult14} {iopath_Amult3_Cmult15} {iopath_Amult3_Cmult16} {iopath_Amult3_Cmult17} {iopath_Amult3_Cmult18} {iopath_Amult3_Cmult19} {iopath_Amult3_Cmult20} {iopath_Amult3_Cmult21} {iopath_Amult3_Cmult22} {iopath_Amult3_Cmult23} {iopath_Amult3_Cmult24} {iopath_Amult3_Cmult25} {iopath_Amult3_Cmult26} {iopath_Amult3_Cmult27} {iopath_Amult3_Cmult28} {iopath_Amult3_Cmult29} {iopath_Amult3_Cmult30} {iopath_Amult3_Cmult31} {iopath_Amult3_Cmult32} {iopath_Amult3_Cmult33} {iopath_Amult3_Cmult34} {iopath_Amult3_Cmult35} {iopath_Amult3_Cmult36} {iopath_Amult3_Cmult37} {iopath_Amult3_Cmult38} {iopath_Amult3_Cmult39} {iopath_Amult3_Cmult40} {iopath_Amult3_Cmult41} {iopath_Amult3_Cmult42} {iopath_Amult3_Cmult43} {iopath_Amult3_Cmult44} {iopath_Amult3_Cmult45} {iopath_Amult3_Cmult46} {iopath_Amult3_Cmult47} {iopath_Amult3_Cmult48} {iopath_Amult3_Cmult49} {iopath_Amult3_Cmult50} {iopath_Amult3_Cmult51} {iopath_Amult3_Cmult52} {iopath_Amult3_Cmult53} {iopath_Amult3_Cmult54} {iopath_Amult3_Cmult55} {iopath_Amult3_Cmult56} {iopath_Amult3_Cmult57} {iopath_Amult3_Cmult58} {iopath_Amult3_Cmult59} {iopath_Amult3_Cmult60} {iopath_Amult3_Cmult61} {iopath_Amult3_Cmult62} {iopath_Amult3_Cmult63} 0 0 0 0 {iopath_Amult4_Cmult4} {iopath_Amult4_Cmult5} {iopath_Amult4_Cmult6} {iopath_Amult4_Cmult7} {iopath_Amult4_Cmult8} {iopath_Amult4_Cmult9} {iopath_Amult4_Cmult10} {iopath_Amult4_Cmult11} {iopath_Amult4_Cmult12} {iopath_Amult4_Cmult13} {iopath_Amult4_Cmult14} {iopath_Amult4_Cmult15} {iopath_Amult4_Cmult16} {iopath_Amult4_Cmult17} {iopath_Amult4_Cmult18} {iopath_Amult4_Cmult19} {iopath_Amult4_Cmult20} {iopath_Amult4_Cmult21} {iopath_Amult4_Cmult22} {iopath_Amult4_Cmult23} {iopath_Amult4_Cmult24} {iopath_Amult4_Cmult25} {iopath_Amult4_Cmult26} {iopath_Amult4_Cmult27} {iopath_Amult4_Cmult28} {iopath_Amult4_Cmult29} {iopath_Amult4_Cmult30} {iopath_Amult4_Cmult31} {iopath_Amult4_Cmult32} {iopath_Amult4_Cmult33} {iopath_Amult4_Cmult34} {iopath_Amult4_Cmult35} {iopath_Amult4_Cmult36} {iopath_Amult4_Cmult37} {iopath_Amult4_Cmult38} {iopath_Amult4_Cmult39} {iopath_Amult4_Cmult40} {iopath_Amult4_Cmult41} {iopath_Amult4_Cmult42} {iopath_Amult4_Cmult43} {iopath_Amult4_Cmult44} {iopath_Amult4_Cmult45} {iopath_Amult4_Cmult46} {iopath_Amult4_Cmult47} {iopath_Amult4_Cmult48} {iopath_Amult4_Cmult49} {iopath_Amult4_Cmult50} {iopath_Amult4_Cmult51} {iopath_Amult4_Cmult52} {iopath_Amult4_Cmult53} {iopath_Amult4_Cmult54} {iopath_Amult4_Cmult55} {iopath_Amult4_Cmult56} {iopath_Amult4_Cmult57} {iopath_Amult4_Cmult58} {iopath_Amult4_Cmult59} {iopath_Amult4_Cmult60} {iopath_Amult4_Cmult61} {iopath_Amult4_Cmult62} {iopath_Amult4_Cmult63} 0 0 0 0 0 {iopath_Amult5_Cmult5} {iopath_Amult5_Cmult6} {iopath_Amult5_Cmult7} {iopath_Amult5_Cmult8} {iopath_Amult5_Cmult9} {iopath_Amult5_Cmult10} {iopath_Amult5_Cmult11} {iopath_Amult5_Cmult12} {iopath_Amult5_Cmult13} {iopath_Amult5_Cmult14} {iopath_Amult5_Cmult15} {iopath_Amult5_Cmult16} {iopath_Amult5_Cmult17} {iopath_Amult5_Cmult18} {iopath_Amult5_Cmult19} {iopath_Amult5_Cmult20} {iopath_Amult5_Cmult21} {iopath_Amult5_Cmult22} {iopath_Amult5_Cmult23} {iopath_Amult5_Cmult24} {iopath_Amult5_Cmult25} {iopath_Amult5_Cmult26} {iopath_Amult5_Cmult27} {iopath_Amult5_Cmult28} {iopath_Amult5_Cmult29} {iopath_Amult5_Cmult30} {iopath_Amult5_Cmult31} {iopath_Amult5_Cmult32} {iopath_Amult5_Cmult33} {iopath_Amult5_Cmult34} {iopath_Amult5_Cmult35} {iopath_Amult5_Cmult36} {iopath_Amult5_Cmult37} {iopath_Amult5_Cmult38} {iopath_Amult5_Cmult39} {iopath_Amult5_Cmult40} {iopath_Amult5_Cmult41} {iopath_Amult5_Cmult42} {iopath_Amult5_Cmult43} {iopath_Amult5_Cmult44} {iopath_Amult5_Cmult45} {iopath_Amult5_Cmult46} {iopath_Amult5_Cmult47} {iopath_Amult5_Cmult48} {iopath_Amult5_Cmult49} {iopath_Amult5_Cmult50} {iopath_Amult5_Cmult51} {iopath_Amult5_Cmult52} {iopath_Amult5_Cmult53} {iopath_Amult5_Cmult54} {iopath_Amult5_Cmult55} {iopath_Amult5_Cmult56} {iopath_Amult5_Cmult57} {iopath_Amult5_Cmult58} {iopath_Amult5_Cmult59} {iopath_Amult5_Cmult60} {iopath_Amult5_Cmult61} {iopath_Amult5_Cmult62} {iopath_Amult5_Cmult63} 0 0 0 0 0 0 {iopath_Amult6_Cmult6} {iopath_Amult6_Cmult7} {iopath_Amult6_Cmult8} {iopath_Amult6_Cmult9} {iopath_Amult6_Cmult10} {iopath_Amult6_Cmult11} {iopath_Amult6_Cmult12} {iopath_Amult6_Cmult13} {iopath_Amult6_Cmult14} {iopath_Amult6_Cmult15} {iopath_Amult6_Cmult16} {iopath_Amult6_Cmult17} {iopath_Amult6_Cmult18} {iopath_Amult6_Cmult19} {iopath_Amult6_Cmult20} {iopath_Amult6_Cmult21} {iopath_Amult6_Cmult22} {iopath_Amult6_Cmult23} {iopath_Amult6_Cmult24} {iopath_Amult6_Cmult25} {iopath_Amult6_Cmult26} {iopath_Amult6_Cmult27} {iopath_Amult6_Cmult28} {iopath_Amult6_Cmult29} {iopath_Amult6_Cmult30} {iopath_Amult6_Cmult31} {iopath_Amult6_Cmult32} {iopath_Amult6_Cmult33} {iopath_Amult6_Cmult34} {iopath_Amult6_Cmult35} {iopath_Amult6_Cmult36} {iopath_Amult6_Cmult37} {iopath_Amult6_Cmult38} {iopath_Amult6_Cmult39} {iopath_Amult6_Cmult40} {iopath_Amult6_Cmult41} {iopath_Amult6_Cmult42} {iopath_Amult6_Cmult43} {iopath_Amult6_Cmult44} {iopath_Amult6_Cmult45} {iopath_Amult6_Cmult46} {iopath_Amult6_Cmult47} {iopath_Amult6_Cmult48} {iopath_Amult6_Cmult49} {iopath_Amult6_Cmult50} {iopath_Amult6_Cmult51} {iopath_Amult6_Cmult52} {iopath_Amult6_Cmult53} {iopath_Amult6_Cmult54} {iopath_Amult6_Cmult55} {iopath_Amult6_Cmult56} {iopath_Amult6_Cmult57} {iopath_Amult6_Cmult58} {iopath_Amult6_Cmult59} {iopath_Amult6_Cmult60} {iopath_Amult6_Cmult61} {iopath_Amult6_Cmult62} {iopath_Amult6_Cmult63} 0 0 0 0 0 0 0 {iopath_Amult7_Cmult7} {iopath_Amult7_Cmult8} {iopath_Amult7_Cmult9} {iopath_Amult7_Cmult10} {iopath_Amult7_Cmult11} {iopath_Amult7_Cmult12} {iopath_Amult7_Cmult13} {iopath_Amult7_Cmult14} {iopath_Amult7_Cmult15} {iopath_Amult7_Cmult16} {iopath_Amult7_Cmult17} {iopath_Amult7_Cmult18} {iopath_Amult7_Cmult19} {iopath_Amult7_Cmult20} {iopath_Amult7_Cmult21} {iopath_Amult7_Cmult22} {iopath_Amult7_Cmult23} {iopath_Amult7_Cmult24} {iopath_Amult7_Cmult25} {iopath_Amult7_Cmult26} {iopath_Amult7_Cmult27} {iopath_Amult7_Cmult28} {iopath_Amult7_Cmult29} {iopath_Amult7_Cmult30} {iopath_Amult7_Cmult31} {iopath_Amult7_Cmult32} {iopath_Amult7_Cmult33} {iopath_Amult7_Cmult34} {iopath_Amult7_Cmult35} {iopath_Amult7_Cmult36} {iopath_Amult7_Cmult37} {iopath_Amult7_Cmult38} {iopath_Amult7_Cmult39} {iopath_Amult7_Cmult40} {iopath_Amult7_Cmult41} {iopath_Amult7_Cmult42} {iopath_Amult7_Cmult43} {iopath_Amult7_Cmult44} {iopath_Amult7_Cmult45} {iopath_Amult7_Cmult46} {iopath_Amult7_Cmult47} {iopath_Amult7_Cmult48} {iopath_Amult7_Cmult49} {iopath_Amult7_Cmult50} {iopath_Amult7_Cmult51} {iopath_Amult7_Cmult52} {iopath_Amult7_Cmult53} {iopath_Amult7_Cmult54} {iopath_Amult7_Cmult55} {iopath_Amult7_Cmult56} {iopath_Amult7_Cmult57} {iopath_Amult7_Cmult58} {iopath_Amult7_Cmult59} {iopath_Amult7_Cmult60} {iopath_Amult7_Cmult61} {iopath_Amult7_Cmult62} {iopath_Amult7_Cmult63} 0 0 0 0 0 0 0 0 {iopath_Amult8_Cmult8} {iopath_Amult8_Cmult9} {iopath_Amult8_Cmult10} {iopath_Amult8_Cmult11} {iopath_Amult8_Cmult12} {iopath_Amult8_Cmult13} {iopath_Amult8_Cmult14} {iopath_Amult8_Cmult15} {iopath_Amult8_Cmult16} {iopath_Amult8_Cmult17} {iopath_Amult8_Cmult18} {iopath_Amult8_Cmult19} {iopath_Amult8_Cmult20} {iopath_Amult8_Cmult21} {iopath_Amult8_Cmult22} {iopath_Amult8_Cmult23} {iopath_Amult8_Cmult24} {iopath_Amult8_Cmult25} {iopath_Amult8_Cmult26} {iopath_Amult8_Cmult27} {iopath_Amult8_Cmult28} {iopath_Amult8_Cmult29} {iopath_Amult8_Cmult30} {iopath_Amult8_Cmult31} {iopath_Amult8_Cmult32} {iopath_Amult8_Cmult33} {iopath_Amult8_Cmult34} {iopath_Amult8_Cmult35} {iopath_Amult8_Cmult36} {iopath_Amult8_Cmult37} {iopath_Amult8_Cmult38} {iopath_Amult8_Cmult39} {iopath_Amult8_Cmult40} {iopath_Amult8_Cmult41} {iopath_Amult8_Cmult42} {iopath_Amult8_Cmult43} {iopath_Amult8_Cmult44} {iopath_Amult8_Cmult45} {iopath_Amult8_Cmult46} {iopath_Amult8_Cmult47} {iopath_Amult8_Cmult48} {iopath_Amult8_Cmult49} {iopath_Amult8_Cmult50} {iopath_Amult8_Cmult51} {iopath_Amult8_Cmult52} {iopath_Amult8_Cmult53} {iopath_Amult8_Cmult54} {iopath_Amult8_Cmult55} {iopath_Amult8_Cmult56} {iopath_Amult8_Cmult57} {iopath_Amult8_Cmult58} {iopath_Amult8_Cmult59} {iopath_Amult8_Cmult60} {iopath_Amult8_Cmult61} {iopath_Amult8_Cmult62} {iopath_Amult8_Cmult63} 0 0 0 0 0 0 0 0 0 {iopath_Amult9_Cmult9} {iopath_Amult9_Cmult10} {iopath_Amult9_Cmult11} {iopath_Amult9_Cmult12} {iopath_Amult9_Cmult13} {iopath_Amult9_Cmult14} {iopath_Amult9_Cmult15} {iopath_Amult9_Cmult16} {iopath_Amult9_Cmult17} {iopath_Amult9_Cmult18} {iopath_Amult9_Cmult19} {iopath_Amult9_Cmult20} {iopath_Amult9_Cmult21} {iopath_Amult9_Cmult22} {iopath_Amult9_Cmult23} {iopath_Amult9_Cmult24} {iopath_Amult9_Cmult25} {iopath_Amult9_Cmult26} {iopath_Amult9_Cmult27} {iopath_Amult9_Cmult28} {iopath_Amult9_Cmult29} {iopath_Amult9_Cmult30} {iopath_Amult9_Cmult31} {iopath_Amult9_Cmult32} {iopath_Amult9_Cmult33} {iopath_Amult9_Cmult34} {iopath_Amult9_Cmult35} {iopath_Amult9_Cmult36} {iopath_Amult9_Cmult37} {iopath_Amult9_Cmult38} {iopath_Amult9_Cmult39} {iopath_Amult9_Cmult40} {iopath_Amult9_Cmult41} {iopath_Amult9_Cmult42} {iopath_Amult9_Cmult43} {iopath_Amult9_Cmult44} {iopath_Amult9_Cmult45} {iopath_Amult9_Cmult46} {iopath_Amult9_Cmult47} {iopath_Amult9_Cmult48} {iopath_Amult9_Cmult49} {iopath_Amult9_Cmult50} {iopath_Amult9_Cmult51} {iopath_Amult9_Cmult52} {iopath_Amult9_Cmult53} {iopath_Amult9_Cmult54} {iopath_Amult9_Cmult55} {iopath_Amult9_Cmult56} {iopath_Amult9_Cmult57} {iopath_Amult9_Cmult58} {iopath_Amult9_Cmult59} {iopath_Amult9_Cmult60} {iopath_Amult9_Cmult61} {iopath_Amult9_Cmult62} {iopath_Amult9_Cmult63} 0 0 0 0 0 0 0 0 0 0 {iopath_Amult10_Cmult10} {iopath_Amult10_Cmult11} {iopath_Amult10_Cmult12} {iopath_Amult10_Cmult13} {iopath_Amult10_Cmult14} {iopath_Amult10_Cmult15} {iopath_Amult10_Cmult16} {iopath_Amult10_Cmult17} {iopath_Amult10_Cmult18} {iopath_Amult10_Cmult19} {iopath_Amult10_Cmult20} {iopath_Amult10_Cmult21} {iopath_Amult10_Cmult22} {iopath_Amult10_Cmult23} {iopath_Amult10_Cmult24} {iopath_Amult10_Cmult25} {iopath_Amult10_Cmult26} {iopath_Amult10_Cmult27} {iopath_Amult10_Cmult28} {iopath_Amult10_Cmult29} {iopath_Amult10_Cmult30} {iopath_Amult10_Cmult31} {iopath_Amult10_Cmult32} {iopath_Amult10_Cmult33} {iopath_Amult10_Cmult34} {iopath_Amult10_Cmult35} {iopath_Amult10_Cmult36} {iopath_Amult10_Cmult37} {iopath_Amult10_Cmult38} {iopath_Amult10_Cmult39} {iopath_Amult10_Cmult40} {iopath_Amult10_Cmult41} {iopath_Amult10_Cmult42} {iopath_Amult10_Cmult43} {iopath_Amult10_Cmult44} {iopath_Amult10_Cmult45} {iopath_Amult10_Cmult46} {iopath_Amult10_Cmult47} {iopath_Amult10_Cmult48} {iopath_Amult10_Cmult49} {iopath_Amult10_Cmult50} {iopath_Amult10_Cmult51} {iopath_Amult10_Cmult52} {iopath_Amult10_Cmult53} {iopath_Amult10_Cmult54} {iopath_Amult10_Cmult55} {iopath_Amult10_Cmult56} {iopath_Amult10_Cmult57} {iopath_Amult10_Cmult58} {iopath_Amult10_Cmult59} {iopath_Amult10_Cmult60} {iopath_Amult10_Cmult61} {iopath_Amult10_Cmult62} {iopath_Amult10_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult11_Cmult11} {iopath_Amult11_Cmult12} {iopath_Amult11_Cmult13} {iopath_Amult11_Cmult14} {iopath_Amult11_Cmult15} {iopath_Amult11_Cmult16} {iopath_Amult11_Cmult17} {iopath_Amult11_Cmult18} {iopath_Amult11_Cmult19} {iopath_Amult11_Cmult20} {iopath_Amult11_Cmult21} {iopath_Amult11_Cmult22} {iopath_Amult11_Cmult23} {iopath_Amult11_Cmult24} {iopath_Amult11_Cmult25} {iopath_Amult11_Cmult26} {iopath_Amult11_Cmult27} {iopath_Amult11_Cmult28} {iopath_Amult11_Cmult29} {iopath_Amult11_Cmult30} {iopath_Amult11_Cmult31} {iopath_Amult11_Cmult32} {iopath_Amult11_Cmult33} {iopath_Amult11_Cmult34} {iopath_Amult11_Cmult35} {iopath_Amult11_Cmult36} {iopath_Amult11_Cmult37} {iopath_Amult11_Cmult38} {iopath_Amult11_Cmult39} {iopath_Amult11_Cmult40} {iopath_Amult11_Cmult41} {iopath_Amult11_Cmult42} {iopath_Amult11_Cmult43} {iopath_Amult11_Cmult44} {iopath_Amult11_Cmult45} {iopath_Amult11_Cmult46} {iopath_Amult11_Cmult47} {iopath_Amult11_Cmult48} {iopath_Amult11_Cmult49} {iopath_Amult11_Cmult50} {iopath_Amult11_Cmult51} {iopath_Amult11_Cmult52} {iopath_Amult11_Cmult53} {iopath_Amult11_Cmult54} {iopath_Amult11_Cmult55} {iopath_Amult11_Cmult56} {iopath_Amult11_Cmult57} {iopath_Amult11_Cmult58} {iopath_Amult11_Cmult59} {iopath_Amult11_Cmult60} {iopath_Amult11_Cmult61} {iopath_Amult11_Cmult62} {iopath_Amult11_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult12_Cmult12} {iopath_Amult12_Cmult13} {iopath_Amult12_Cmult14} {iopath_Amult12_Cmult15} {iopath_Amult12_Cmult16} {iopath_Amult12_Cmult17} {iopath_Amult12_Cmult18} {iopath_Amult12_Cmult19} {iopath_Amult12_Cmult20} {iopath_Amult12_Cmult21} {iopath_Amult12_Cmult22} {iopath_Amult12_Cmult23} {iopath_Amult12_Cmult24} {iopath_Amult12_Cmult25} {iopath_Amult12_Cmult26} {iopath_Amult12_Cmult27} {iopath_Amult12_Cmult28} {iopath_Amult12_Cmult29} {iopath_Amult12_Cmult30} {iopath_Amult12_Cmult31} {iopath_Amult12_Cmult32} {iopath_Amult12_Cmult33} {iopath_Amult12_Cmult34} {iopath_Amult12_Cmult35} {iopath_Amult12_Cmult36} {iopath_Amult12_Cmult37} {iopath_Amult12_Cmult38} {iopath_Amult12_Cmult39} {iopath_Amult12_Cmult40} {iopath_Amult12_Cmult41} {iopath_Amult12_Cmult42} {iopath_Amult12_Cmult43} {iopath_Amult12_Cmult44} {iopath_Amult12_Cmult45} {iopath_Amult12_Cmult46} {iopath_Amult12_Cmult47} {iopath_Amult12_Cmult48} {iopath_Amult12_Cmult49} {iopath_Amult12_Cmult50} {iopath_Amult12_Cmult51} {iopath_Amult12_Cmult52} {iopath_Amult12_Cmult53} {iopath_Amult12_Cmult54} {iopath_Amult12_Cmult55} {iopath_Amult12_Cmult56} {iopath_Amult12_Cmult57} {iopath_Amult12_Cmult58} {iopath_Amult12_Cmult59} {iopath_Amult12_Cmult60} {iopath_Amult12_Cmult61} {iopath_Amult12_Cmult62} {iopath_Amult12_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult13_Cmult13} {iopath_Amult13_Cmult14} {iopath_Amult13_Cmult15} {iopath_Amult13_Cmult16} {iopath_Amult13_Cmult17} {iopath_Amult13_Cmult18} {iopath_Amult13_Cmult19} {iopath_Amult13_Cmult20} {iopath_Amult13_Cmult21} {iopath_Amult13_Cmult22} {iopath_Amult13_Cmult23} {iopath_Amult13_Cmult24} {iopath_Amult13_Cmult25} {iopath_Amult13_Cmult26} {iopath_Amult13_Cmult27} {iopath_Amult13_Cmult28} {iopath_Amult13_Cmult29} {iopath_Amult13_Cmult30} {iopath_Amult13_Cmult31} {iopath_Amult13_Cmult32} {iopath_Amult13_Cmult33} {iopath_Amult13_Cmult34} {iopath_Amult13_Cmult35} {iopath_Amult13_Cmult36} {iopath_Amult13_Cmult37} {iopath_Amult13_Cmult38} {iopath_Amult13_Cmult39} {iopath_Amult13_Cmult40} {iopath_Amult13_Cmult41} {iopath_Amult13_Cmult42} {iopath_Amult13_Cmult43} {iopath_Amult13_Cmult44} {iopath_Amult13_Cmult45} {iopath_Amult13_Cmult46} {iopath_Amult13_Cmult47} {iopath_Amult13_Cmult48} {iopath_Amult13_Cmult49} {iopath_Amult13_Cmult50} {iopath_Amult13_Cmult51} {iopath_Amult13_Cmult52} {iopath_Amult13_Cmult53} {iopath_Amult13_Cmult54} {iopath_Amult13_Cmult55} {iopath_Amult13_Cmult56} {iopath_Amult13_Cmult57} {iopath_Amult13_Cmult58} {iopath_Amult13_Cmult59} {iopath_Amult13_Cmult60} {iopath_Amult13_Cmult61} {iopath_Amult13_Cmult62} {iopath_Amult13_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult14_Cmult14} {iopath_Amult14_Cmult15} {iopath_Amult14_Cmult16} {iopath_Amult14_Cmult17} {iopath_Amult14_Cmult18} {iopath_Amult14_Cmult19} {iopath_Amult14_Cmult20} {iopath_Amult14_Cmult21} {iopath_Amult14_Cmult22} {iopath_Amult14_Cmult23} {iopath_Amult14_Cmult24} {iopath_Amult14_Cmult25} {iopath_Amult14_Cmult26} {iopath_Amult14_Cmult27} {iopath_Amult14_Cmult28} {iopath_Amult14_Cmult29} {iopath_Amult14_Cmult30} {iopath_Amult14_Cmult31} {iopath_Amult14_Cmult32} {iopath_Amult14_Cmult33} {iopath_Amult14_Cmult34} {iopath_Amult14_Cmult35} {iopath_Amult14_Cmult36} {iopath_Amult14_Cmult37} {iopath_Amult14_Cmult38} {iopath_Amult14_Cmult39} {iopath_Amult14_Cmult40} {iopath_Amult14_Cmult41} {iopath_Amult14_Cmult42} {iopath_Amult14_Cmult43} {iopath_Amult14_Cmult44} {iopath_Amult14_Cmult45} {iopath_Amult14_Cmult46} {iopath_Amult14_Cmult47} {iopath_Amult14_Cmult48} {iopath_Amult14_Cmult49} {iopath_Amult14_Cmult50} {iopath_Amult14_Cmult51} {iopath_Amult14_Cmult52} {iopath_Amult14_Cmult53} {iopath_Amult14_Cmult54} {iopath_Amult14_Cmult55} {iopath_Amult14_Cmult56} {iopath_Amult14_Cmult57} {iopath_Amult14_Cmult58} {iopath_Amult14_Cmult59} {iopath_Amult14_Cmult60} {iopath_Amult14_Cmult61} {iopath_Amult14_Cmult62} {iopath_Amult14_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult15_Cmult15} {iopath_Amult15_Cmult16} {iopath_Amult15_Cmult17} {iopath_Amult15_Cmult18} {iopath_Amult15_Cmult19} {iopath_Amult15_Cmult20} {iopath_Amult15_Cmult21} {iopath_Amult15_Cmult22} {iopath_Amult15_Cmult23} {iopath_Amult15_Cmult24} {iopath_Amult15_Cmult25} {iopath_Amult15_Cmult26} {iopath_Amult15_Cmult27} {iopath_Amult15_Cmult28} {iopath_Amult15_Cmult29} {iopath_Amult15_Cmult30} {iopath_Amult15_Cmult31} {iopath_Amult15_Cmult32} {iopath_Amult15_Cmult33} {iopath_Amult15_Cmult34} {iopath_Amult15_Cmult35} {iopath_Amult15_Cmult36} {iopath_Amult15_Cmult37} {iopath_Amult15_Cmult38} {iopath_Amult15_Cmult39} {iopath_Amult15_Cmult40} {iopath_Amult15_Cmult41} {iopath_Amult15_Cmult42} {iopath_Amult15_Cmult43} {iopath_Amult15_Cmult44} {iopath_Amult15_Cmult45} {iopath_Amult15_Cmult46} {iopath_Amult15_Cmult47} {iopath_Amult15_Cmult48} {iopath_Amult15_Cmult49} {iopath_Amult15_Cmult50} {iopath_Amult15_Cmult51} {iopath_Amult15_Cmult52} {iopath_Amult15_Cmult53} {iopath_Amult15_Cmult54} {iopath_Amult15_Cmult55} {iopath_Amult15_Cmult56} {iopath_Amult15_Cmult57} {iopath_Amult15_Cmult58} {iopath_Amult15_Cmult59} {iopath_Amult15_Cmult60} {iopath_Amult15_Cmult61} {iopath_Amult15_Cmult62} {iopath_Amult15_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult16_Cmult16} {iopath_Amult16_Cmult17} {iopath_Amult16_Cmult18} {iopath_Amult16_Cmult19} {iopath_Amult16_Cmult20} {iopath_Amult16_Cmult21} {iopath_Amult16_Cmult22} {iopath_Amult16_Cmult23} {iopath_Amult16_Cmult24} {iopath_Amult16_Cmult25} {iopath_Amult16_Cmult26} {iopath_Amult16_Cmult27} {iopath_Amult16_Cmult28} {iopath_Amult16_Cmult29} {iopath_Amult16_Cmult30} {iopath_Amult16_Cmult31} {iopath_Amult16_Cmult32} {iopath_Amult16_Cmult33} {iopath_Amult16_Cmult34} {iopath_Amult16_Cmult35} {iopath_Amult16_Cmult36} {iopath_Amult16_Cmult37} {iopath_Amult16_Cmult38} {iopath_Amult16_Cmult39} {iopath_Amult16_Cmult40} {iopath_Amult16_Cmult41} {iopath_Amult16_Cmult42} {iopath_Amult16_Cmult43} {iopath_Amult16_Cmult44} {iopath_Amult16_Cmult45} {iopath_Amult16_Cmult46} {iopath_Amult16_Cmult47} {iopath_Amult16_Cmult48} {iopath_Amult16_Cmult49} {iopath_Amult16_Cmult50} {iopath_Amult16_Cmult51} {iopath_Amult16_Cmult52} {iopath_Amult16_Cmult53} {iopath_Amult16_Cmult54} {iopath_Amult16_Cmult55} {iopath_Amult16_Cmult56} {iopath_Amult16_Cmult57} {iopath_Amult16_Cmult58} {iopath_Amult16_Cmult59} {iopath_Amult16_Cmult60} {iopath_Amult16_Cmult61} {iopath_Amult16_Cmult62} {iopath_Amult16_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult17_Cmult17} {iopath_Amult17_Cmult18} {iopath_Amult17_Cmult19} {iopath_Amult17_Cmult20} {iopath_Amult17_Cmult21} {iopath_Amult17_Cmult22} {iopath_Amult17_Cmult23} {iopath_Amult17_Cmult24} {iopath_Amult17_Cmult25} {iopath_Amult17_Cmult26} {iopath_Amult17_Cmult27} {iopath_Amult17_Cmult28} {iopath_Amult17_Cmult29} {iopath_Amult17_Cmult30} {iopath_Amult17_Cmult31} {iopath_Amult17_Cmult32} {iopath_Amult17_Cmult33} {iopath_Amult17_Cmult34} {iopath_Amult17_Cmult35} {iopath_Amult17_Cmult36} {iopath_Amult17_Cmult37} {iopath_Amult17_Cmult38} {iopath_Amult17_Cmult39} {iopath_Amult17_Cmult40} {iopath_Amult17_Cmult41} {iopath_Amult17_Cmult42} {iopath_Amult17_Cmult43} {iopath_Amult17_Cmult44} {iopath_Amult17_Cmult45} {iopath_Amult17_Cmult46} {iopath_Amult17_Cmult47} {iopath_Amult17_Cmult48} {iopath_Amult17_Cmult49} {iopath_Amult17_Cmult50} {iopath_Amult17_Cmult51} {iopath_Amult17_Cmult52} {iopath_Amult17_Cmult53} {iopath_Amult17_Cmult54} {iopath_Amult17_Cmult55} {iopath_Amult17_Cmult56} {iopath_Amult17_Cmult57} {iopath_Amult17_Cmult58} {iopath_Amult17_Cmult59} {iopath_Amult17_Cmult60} {iopath_Amult17_Cmult61} {iopath_Amult17_Cmult62} {iopath_Amult17_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult18_Cmult18} {iopath_Amult18_Cmult19} {iopath_Amult18_Cmult20} {iopath_Amult18_Cmult21} {iopath_Amult18_Cmult22} {iopath_Amult18_Cmult23} {iopath_Amult18_Cmult24} {iopath_Amult18_Cmult25} {iopath_Amult18_Cmult26} {iopath_Amult18_Cmult27} {iopath_Amult18_Cmult28} {iopath_Amult18_Cmult29} {iopath_Amult18_Cmult30} {iopath_Amult18_Cmult31} {iopath_Amult18_Cmult32} {iopath_Amult18_Cmult33} {iopath_Amult18_Cmult34} {iopath_Amult18_Cmult35} {iopath_Amult18_Cmult36} {iopath_Amult18_Cmult37} {iopath_Amult18_Cmult38} {iopath_Amult18_Cmult39} {iopath_Amult18_Cmult40} {iopath_Amult18_Cmult41} {iopath_Amult18_Cmult42} {iopath_Amult18_Cmult43} {iopath_Amult18_Cmult44} {iopath_Amult18_Cmult45} {iopath_Amult18_Cmult46} {iopath_Amult18_Cmult47} {iopath_Amult18_Cmult48} {iopath_Amult18_Cmult49} {iopath_Amult18_Cmult50} {iopath_Amult18_Cmult51} {iopath_Amult18_Cmult52} {iopath_Amult18_Cmult53} {iopath_Amult18_Cmult54} {iopath_Amult18_Cmult55} {iopath_Amult18_Cmult56} {iopath_Amult18_Cmult57} {iopath_Amult18_Cmult58} {iopath_Amult18_Cmult59} {iopath_Amult18_Cmult60} {iopath_Amult18_Cmult61} {iopath_Amult18_Cmult62} {iopath_Amult18_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult19_Cmult19} {iopath_Amult19_Cmult20} {iopath_Amult19_Cmult21} {iopath_Amult19_Cmult22} {iopath_Amult19_Cmult23} {iopath_Amult19_Cmult24} {iopath_Amult19_Cmult25} {iopath_Amult19_Cmult26} {iopath_Amult19_Cmult27} {iopath_Amult19_Cmult28} {iopath_Amult19_Cmult29} {iopath_Amult19_Cmult30} {iopath_Amult19_Cmult31} {iopath_Amult19_Cmult32} {iopath_Amult19_Cmult33} {iopath_Amult19_Cmult34} {iopath_Amult19_Cmult35} {iopath_Amult19_Cmult36} {iopath_Amult19_Cmult37} {iopath_Amult19_Cmult38} {iopath_Amult19_Cmult39} {iopath_Amult19_Cmult40} {iopath_Amult19_Cmult41} {iopath_Amult19_Cmult42} {iopath_Amult19_Cmult43} {iopath_Amult19_Cmult44} {iopath_Amult19_Cmult45} {iopath_Amult19_Cmult46} {iopath_Amult19_Cmult47} {iopath_Amult19_Cmult48} {iopath_Amult19_Cmult49} {iopath_Amult19_Cmult50} {iopath_Amult19_Cmult51} {iopath_Amult19_Cmult52} {iopath_Amult19_Cmult53} {iopath_Amult19_Cmult54} {iopath_Amult19_Cmult55} {iopath_Amult19_Cmult56} {iopath_Amult19_Cmult57} {iopath_Amult19_Cmult58} {iopath_Amult19_Cmult59} {iopath_Amult19_Cmult60} {iopath_Amult19_Cmult61} {iopath_Amult19_Cmult62} {iopath_Amult19_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult20_Cmult20} {iopath_Amult20_Cmult21} {iopath_Amult20_Cmult22} {iopath_Amult20_Cmult23} {iopath_Amult20_Cmult24} {iopath_Amult20_Cmult25} {iopath_Amult20_Cmult26} {iopath_Amult20_Cmult27} {iopath_Amult20_Cmult28} {iopath_Amult20_Cmult29} {iopath_Amult20_Cmult30} {iopath_Amult20_Cmult31} {iopath_Amult20_Cmult32} {iopath_Amult20_Cmult33} {iopath_Amult20_Cmult34} {iopath_Amult20_Cmult35} {iopath_Amult20_Cmult36} {iopath_Amult20_Cmult37} {iopath_Amult20_Cmult38} {iopath_Amult20_Cmult39} {iopath_Amult20_Cmult40} {iopath_Amult20_Cmult41} {iopath_Amult20_Cmult42} {iopath_Amult20_Cmult43} {iopath_Amult20_Cmult44} {iopath_Amult20_Cmult45} {iopath_Amult20_Cmult46} {iopath_Amult20_Cmult47} {iopath_Amult20_Cmult48} {iopath_Amult20_Cmult49} {iopath_Amult20_Cmult50} {iopath_Amult20_Cmult51} {iopath_Amult20_Cmult52} {iopath_Amult20_Cmult53} {iopath_Amult20_Cmult54} {iopath_Amult20_Cmult55} {iopath_Amult20_Cmult56} {iopath_Amult20_Cmult57} {iopath_Amult20_Cmult58} {iopath_Amult20_Cmult59} {iopath_Amult20_Cmult60} {iopath_Amult20_Cmult61} {iopath_Amult20_Cmult62} {iopath_Amult20_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult21_Cmult21} {iopath_Amult21_Cmult22} {iopath_Amult21_Cmult23} {iopath_Amult21_Cmult24} {iopath_Amult21_Cmult25} {iopath_Amult21_Cmult26} {iopath_Amult21_Cmult27} {iopath_Amult21_Cmult28} {iopath_Amult21_Cmult29} {iopath_Amult21_Cmult30} {iopath_Amult21_Cmult31} {iopath_Amult21_Cmult32} {iopath_Amult21_Cmult33} {iopath_Amult21_Cmult34} {iopath_Amult21_Cmult35} {iopath_Amult21_Cmult36} {iopath_Amult21_Cmult37} {iopath_Amult21_Cmult38} {iopath_Amult21_Cmult39} {iopath_Amult21_Cmult40} {iopath_Amult21_Cmult41} {iopath_Amult21_Cmult42} {iopath_Amult21_Cmult43} {iopath_Amult21_Cmult44} {iopath_Amult21_Cmult45} {iopath_Amult21_Cmult46} {iopath_Amult21_Cmult47} {iopath_Amult21_Cmult48} {iopath_Amult21_Cmult49} {iopath_Amult21_Cmult50} {iopath_Amult21_Cmult51} {iopath_Amult21_Cmult52} {iopath_Amult21_Cmult53} {iopath_Amult21_Cmult54} {iopath_Amult21_Cmult55} {iopath_Amult21_Cmult56} {iopath_Amult21_Cmult57} {iopath_Amult21_Cmult58} {iopath_Amult21_Cmult59} {iopath_Amult21_Cmult60} {iopath_Amult21_Cmult61} {iopath_Amult21_Cmult62} {iopath_Amult21_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult22_Cmult22} {iopath_Amult22_Cmult23} {iopath_Amult22_Cmult24} {iopath_Amult22_Cmult25} {iopath_Amult22_Cmult26} {iopath_Amult22_Cmult27} {iopath_Amult22_Cmult28} {iopath_Amult22_Cmult29} {iopath_Amult22_Cmult30} {iopath_Amult22_Cmult31} {iopath_Amult22_Cmult32} {iopath_Amult22_Cmult33} {iopath_Amult22_Cmult34} {iopath_Amult22_Cmult35} {iopath_Amult22_Cmult36} {iopath_Amult22_Cmult37} {iopath_Amult22_Cmult38} {iopath_Amult22_Cmult39} {iopath_Amult22_Cmult40} {iopath_Amult22_Cmult41} {iopath_Amult22_Cmult42} {iopath_Amult22_Cmult43} {iopath_Amult22_Cmult44} {iopath_Amult22_Cmult45} {iopath_Amult22_Cmult46} {iopath_Amult22_Cmult47} {iopath_Amult22_Cmult48} {iopath_Amult22_Cmult49} {iopath_Amult22_Cmult50} {iopath_Amult22_Cmult51} {iopath_Amult22_Cmult52} {iopath_Amult22_Cmult53} {iopath_Amult22_Cmult54} {iopath_Amult22_Cmult55} {iopath_Amult22_Cmult56} {iopath_Amult22_Cmult57} {iopath_Amult22_Cmult58} {iopath_Amult22_Cmult59} {iopath_Amult22_Cmult60} {iopath_Amult22_Cmult61} {iopath_Amult22_Cmult62} {iopath_Amult22_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult23_Cmult23} {iopath_Amult23_Cmult24} {iopath_Amult23_Cmult25} {iopath_Amult23_Cmult26} {iopath_Amult23_Cmult27} {iopath_Amult23_Cmult28} {iopath_Amult23_Cmult29} {iopath_Amult23_Cmult30} {iopath_Amult23_Cmult31} {iopath_Amult23_Cmult32} {iopath_Amult23_Cmult33} {iopath_Amult23_Cmult34} {iopath_Amult23_Cmult35} {iopath_Amult23_Cmult36} {iopath_Amult23_Cmult37} {iopath_Amult23_Cmult38} {iopath_Amult23_Cmult39} {iopath_Amult23_Cmult40} {iopath_Amult23_Cmult41} {iopath_Amult23_Cmult42} {iopath_Amult23_Cmult43} {iopath_Amult23_Cmult44} {iopath_Amult23_Cmult45} {iopath_Amult23_Cmult46} {iopath_Amult23_Cmult47} {iopath_Amult23_Cmult48} {iopath_Amult23_Cmult49} {iopath_Amult23_Cmult50} {iopath_Amult23_Cmult51} {iopath_Amult23_Cmult52} {iopath_Amult23_Cmult53} {iopath_Amult23_Cmult54} {iopath_Amult23_Cmult55} {iopath_Amult23_Cmult56} {iopath_Amult23_Cmult57} {iopath_Amult23_Cmult58} {iopath_Amult23_Cmult59} {iopath_Amult23_Cmult60} {iopath_Amult23_Cmult61} {iopath_Amult23_Cmult62} {iopath_Amult23_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult24_Cmult24} {iopath_Amult24_Cmult25} {iopath_Amult24_Cmult26} {iopath_Amult24_Cmult27} {iopath_Amult24_Cmult28} {iopath_Amult24_Cmult29} {iopath_Amult24_Cmult30} {iopath_Amult24_Cmult31} {iopath_Amult24_Cmult32} {iopath_Amult24_Cmult33} {iopath_Amult24_Cmult34} {iopath_Amult24_Cmult35} {iopath_Amult24_Cmult36} {iopath_Amult24_Cmult37} {iopath_Amult24_Cmult38} {iopath_Amult24_Cmult39} {iopath_Amult24_Cmult40} {iopath_Amult24_Cmult41} {iopath_Amult24_Cmult42} {iopath_Amult24_Cmult43} {iopath_Amult24_Cmult44} {iopath_Amult24_Cmult45} {iopath_Amult24_Cmult46} {iopath_Amult24_Cmult47} {iopath_Amult24_Cmult48} {iopath_Amult24_Cmult49} {iopath_Amult24_Cmult50} {iopath_Amult24_Cmult51} {iopath_Amult24_Cmult52} {iopath_Amult24_Cmult53} {iopath_Amult24_Cmult54} {iopath_Amult24_Cmult55} {iopath_Amult24_Cmult56} {iopath_Amult24_Cmult57} {iopath_Amult24_Cmult58} {iopath_Amult24_Cmult59} {iopath_Amult24_Cmult60} {iopath_Amult24_Cmult61} {iopath_Amult24_Cmult62} {iopath_Amult24_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult25_Cmult25} {iopath_Amult25_Cmult26} {iopath_Amult25_Cmult27} {iopath_Amult25_Cmult28} {iopath_Amult25_Cmult29} {iopath_Amult25_Cmult30} {iopath_Amult25_Cmult31} {iopath_Amult25_Cmult32} {iopath_Amult25_Cmult33} {iopath_Amult25_Cmult34} {iopath_Amult25_Cmult35} {iopath_Amult25_Cmult36} {iopath_Amult25_Cmult37} {iopath_Amult25_Cmult38} {iopath_Amult25_Cmult39} {iopath_Amult25_Cmult40} {iopath_Amult25_Cmult41} {iopath_Amult25_Cmult42} {iopath_Amult25_Cmult43} {iopath_Amult25_Cmult44} {iopath_Amult25_Cmult45} {iopath_Amult25_Cmult46} {iopath_Amult25_Cmult47} {iopath_Amult25_Cmult48} {iopath_Amult25_Cmult49} {iopath_Amult25_Cmult50} {iopath_Amult25_Cmult51} {iopath_Amult25_Cmult52} {iopath_Amult25_Cmult53} {iopath_Amult25_Cmult54} {iopath_Amult25_Cmult55} {iopath_Amult25_Cmult56} {iopath_Amult25_Cmult57} {iopath_Amult25_Cmult58} {iopath_Amult25_Cmult59} {iopath_Amult25_Cmult60} {iopath_Amult25_Cmult61} {iopath_Amult25_Cmult62} {iopath_Amult25_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult26_Cmult26} {iopath_Amult26_Cmult27} {iopath_Amult26_Cmult28} {iopath_Amult26_Cmult29} {iopath_Amult26_Cmult30} {iopath_Amult26_Cmult31} {iopath_Amult26_Cmult32} {iopath_Amult26_Cmult33} {iopath_Amult26_Cmult34} {iopath_Amult26_Cmult35} {iopath_Amult26_Cmult36} {iopath_Amult26_Cmult37} {iopath_Amult26_Cmult38} {iopath_Amult26_Cmult39} {iopath_Amult26_Cmult40} {iopath_Amult26_Cmult41} {iopath_Amult26_Cmult42} {iopath_Amult26_Cmult43} {iopath_Amult26_Cmult44} {iopath_Amult26_Cmult45} {iopath_Amult26_Cmult46} {iopath_Amult26_Cmult47} {iopath_Amult26_Cmult48} {iopath_Amult26_Cmult49} {iopath_Amult26_Cmult50} {iopath_Amult26_Cmult51} {iopath_Amult26_Cmult52} {iopath_Amult26_Cmult53} {iopath_Amult26_Cmult54} {iopath_Amult26_Cmult55} {iopath_Amult26_Cmult56} {iopath_Amult26_Cmult57} {iopath_Amult26_Cmult58} {iopath_Amult26_Cmult59} {iopath_Amult26_Cmult60} {iopath_Amult26_Cmult61} {iopath_Amult26_Cmult62} {iopath_Amult26_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult27_Cmult27} {iopath_Amult27_Cmult28} {iopath_Amult27_Cmult29} {iopath_Amult27_Cmult30} {iopath_Amult27_Cmult31} {iopath_Amult27_Cmult32} {iopath_Amult27_Cmult33} {iopath_Amult27_Cmult34} {iopath_Amult27_Cmult35} {iopath_Amult27_Cmult36} {iopath_Amult27_Cmult37} {iopath_Amult27_Cmult38} {iopath_Amult27_Cmult39} {iopath_Amult27_Cmult40} {iopath_Amult27_Cmult41} {iopath_Amult27_Cmult42} {iopath_Amult27_Cmult43} {iopath_Amult27_Cmult44} {iopath_Amult27_Cmult45} {iopath_Amult27_Cmult46} {iopath_Amult27_Cmult47} {iopath_Amult27_Cmult48} {iopath_Amult27_Cmult49} {iopath_Amult27_Cmult50} {iopath_Amult27_Cmult51} {iopath_Amult27_Cmult52} {iopath_Amult27_Cmult53} {iopath_Amult27_Cmult54} {iopath_Amult27_Cmult55} {iopath_Amult27_Cmult56} {iopath_Amult27_Cmult57} {iopath_Amult27_Cmult58} {iopath_Amult27_Cmult59} {iopath_Amult27_Cmult60} {iopath_Amult27_Cmult61} {iopath_Amult27_Cmult62} {iopath_Amult27_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult28_Cmult28} {iopath_Amult28_Cmult29} {iopath_Amult28_Cmult30} {iopath_Amult28_Cmult31} {iopath_Amult28_Cmult32} {iopath_Amult28_Cmult33} {iopath_Amult28_Cmult34} {iopath_Amult28_Cmult35} {iopath_Amult28_Cmult36} {iopath_Amult28_Cmult37} {iopath_Amult28_Cmult38} {iopath_Amult28_Cmult39} {iopath_Amult28_Cmult40} {iopath_Amult28_Cmult41} {iopath_Amult28_Cmult42} {iopath_Amult28_Cmult43} {iopath_Amult28_Cmult44} {iopath_Amult28_Cmult45} {iopath_Amult28_Cmult46} {iopath_Amult28_Cmult47} {iopath_Amult28_Cmult48} {iopath_Amult28_Cmult49} {iopath_Amult28_Cmult50} {iopath_Amult28_Cmult51} {iopath_Amult28_Cmult52} {iopath_Amult28_Cmult53} {iopath_Amult28_Cmult54} {iopath_Amult28_Cmult55} {iopath_Amult28_Cmult56} {iopath_Amult28_Cmult57} {iopath_Amult28_Cmult58} {iopath_Amult28_Cmult59} {iopath_Amult28_Cmult60} {iopath_Amult28_Cmult61} {iopath_Amult28_Cmult62} {iopath_Amult28_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult29_Cmult29} {iopath_Amult29_Cmult30} {iopath_Amult29_Cmult31} {iopath_Amult29_Cmult32} {iopath_Amult29_Cmult33} {iopath_Amult29_Cmult34} {iopath_Amult29_Cmult35} {iopath_Amult29_Cmult36} {iopath_Amult29_Cmult37} {iopath_Amult29_Cmult38} {iopath_Amult29_Cmult39} {iopath_Amult29_Cmult40} {iopath_Amult29_Cmult41} {iopath_Amult29_Cmult42} {iopath_Amult29_Cmult43} {iopath_Amult29_Cmult44} {iopath_Amult29_Cmult45} {iopath_Amult29_Cmult46} {iopath_Amult29_Cmult47} {iopath_Amult29_Cmult48} {iopath_Amult29_Cmult49} {iopath_Amult29_Cmult50} {iopath_Amult29_Cmult51} {iopath_Amult29_Cmult52} {iopath_Amult29_Cmult53} {iopath_Amult29_Cmult54} {iopath_Amult29_Cmult55} {iopath_Amult29_Cmult56} {iopath_Amult29_Cmult57} {iopath_Amult29_Cmult58} {iopath_Amult29_Cmult59} {iopath_Amult29_Cmult60} {iopath_Amult29_Cmult61} {iopath_Amult29_Cmult62} {iopath_Amult29_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult30_Cmult30} {iopath_Amult30_Cmult31} {iopath_Amult30_Cmult32} {iopath_Amult30_Cmult33} {iopath_Amult30_Cmult34} {iopath_Amult30_Cmult35} {iopath_Amult30_Cmult36} {iopath_Amult30_Cmult37} {iopath_Amult30_Cmult38} {iopath_Amult30_Cmult39} {iopath_Amult30_Cmult40} {iopath_Amult30_Cmult41} {iopath_Amult30_Cmult42} {iopath_Amult30_Cmult43} {iopath_Amult30_Cmult44} {iopath_Amult30_Cmult45} {iopath_Amult30_Cmult46} {iopath_Amult30_Cmult47} {iopath_Amult30_Cmult48} {iopath_Amult30_Cmult49} {iopath_Amult30_Cmult50} {iopath_Amult30_Cmult51} {iopath_Amult30_Cmult52} {iopath_Amult30_Cmult53} {iopath_Amult30_Cmult54} {iopath_Amult30_Cmult55} {iopath_Amult30_Cmult56} {iopath_Amult30_Cmult57} {iopath_Amult30_Cmult58} {iopath_Amult30_Cmult59} {iopath_Amult30_Cmult60} {iopath_Amult30_Cmult61} {iopath_Amult30_Cmult62} {iopath_Amult30_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult31_Cmult31} {iopath_Amult31_Cmult32} {iopath_Amult31_Cmult33} {iopath_Amult31_Cmult34} {iopath_Amult31_Cmult35} {iopath_Amult31_Cmult36} {iopath_Amult31_Cmult37} {iopath_Amult31_Cmult38} {iopath_Amult31_Cmult39} {iopath_Amult31_Cmult40} {iopath_Amult31_Cmult41} {iopath_Amult31_Cmult42} {iopath_Amult31_Cmult43} {iopath_Amult31_Cmult44} {iopath_Amult31_Cmult45} {iopath_Amult31_Cmult46} {iopath_Amult31_Cmult47} {iopath_Amult31_Cmult48} {iopath_Amult31_Cmult49} {iopath_Amult31_Cmult50} {iopath_Amult31_Cmult51} {iopath_Amult31_Cmult52} {iopath_Amult31_Cmult53} {iopath_Amult31_Cmult54} {iopath_Amult31_Cmult55} {iopath_Amult31_Cmult56} {iopath_Amult31_Cmult57} {iopath_Amult31_Cmult58} {iopath_Amult31_Cmult59} {iopath_Amult31_Cmult60} {iopath_Amult31_Cmult61} {iopath_Amult31_Cmult62} {iopath_Amult31_Cmult63} "*)
	(* DELAY_MATRIX_Bmult="{iopath_Amult0_Cmult0} {iopath_Amult0_Cmult1} {iopath_Amult0_Cmult2} {iopath_Amult0_Cmult3} {iopath_Amult0_Cmult4} {iopath_Amult0_Cmult5} {iopath_Amult0_Cmult6} {iopath_Amult0_Cmult7} {iopath_Amult0_Cmult8} {iopath_Amult0_Cmult9} {iopath_Amult0_Cmult10} {iopath_Amult0_Cmult11} {iopath_Amult0_Cmult12} {iopath_Amult0_Cmult13} {iopath_Amult0_Cmult14} {iopath_Amult0_Cmult15} {iopath_Amult0_Cmult16} {iopath_Amult0_Cmult17} {iopath_Amult0_Cmult18} {iopath_Amult0_Cmult19} {iopath_Amult0_Cmult20} {iopath_Amult0_Cmult21} {iopath_Amult0_Cmult22} {iopath_Amult0_Cmult23} {iopath_Amult0_Cmult24} {iopath_Amult0_Cmult25} {iopath_Amult0_Cmult26} {iopath_Amult0_Cmult27} {iopath_Amult0_Cmult28} {iopath_Amult0_Cmult29} {iopath_Amult0_Cmult30} {iopath_Amult0_Cmult31} {iopath_Amult0_Cmult32} {iopath_Amult0_Cmult33} {iopath_Amult0_Cmult34} {iopath_Amult0_Cmult35} {iopath_Amult0_Cmult36} {iopath_Amult0_Cmult37} {iopath_Amult0_Cmult38} {iopath_Amult0_Cmult39} {iopath_Amult0_Cmult40} {iopath_Amult0_Cmult41} {iopath_Amult0_Cmult42} {iopath_Amult0_Cmult43} {iopath_Amult0_Cmult44} {iopath_Amult0_Cmult45} {iopath_Amult0_Cmult46} {iopath_Amult0_Cmult47} {iopath_Amult0_Cmult48} {iopath_Amult0_Cmult49} {iopath_Amult0_Cmult50} {iopath_Amult0_Cmult51} {iopath_Amult0_Cmult52} {iopath_Amult0_Cmult53} {iopath_Amult0_Cmult54} {iopath_Amult0_Cmult55} {iopath_Amult0_Cmult56} {iopath_Amult0_Cmult57} {iopath_Amult0_Cmult58} {iopath_Amult0_Cmult59} {iopath_Amult0_Cmult60} {iopath_Amult0_Cmult61} {iopath_Amult0_Cmult62} {iopath_Amult0_Cmult63} 0 {iopath_Amult1_Cmult1} {iopath_Amult1_Cmult2} {iopath_Amult1_Cmult3} {iopath_Amult1_Cmult4} {iopath_Amult1_Cmult5} {iopath_Amult1_Cmult6} {iopath_Amult1_Cmult7} {iopath_Amult1_Cmult8} {iopath_Amult1_Cmult9} {iopath_Amult1_Cmult10} {iopath_Amult1_Cmult11} {iopath_Amult1_Cmult12} {iopath_Amult1_Cmult13} {iopath_Amult1_Cmult14} {iopath_Amult1_Cmult15} {iopath_Amult1_Cmult16} {iopath_Amult1_Cmult17} {iopath_Amult1_Cmult18} {iopath_Amult1_Cmult19} {iopath_Amult1_Cmult20} {iopath_Amult1_Cmult21} {iopath_Amult1_Cmult22} {iopath_Amult1_Cmult23} {iopath_Amult1_Cmult24} {iopath_Amult1_Cmult25} {iopath_Amult1_Cmult26} {iopath_Amult1_Cmult27} {iopath_Amult1_Cmult28} {iopath_Amult1_Cmult29} {iopath_Amult1_Cmult30} {iopath_Amult1_Cmult31} {iopath_Amult1_Cmult32} {iopath_Amult1_Cmult33} {iopath_Amult1_Cmult34} {iopath_Amult1_Cmult35} {iopath_Amult1_Cmult36} {iopath_Amult1_Cmult37} {iopath_Amult1_Cmult38} {iopath_Amult1_Cmult39} {iopath_Amult1_Cmult40} {iopath_Amult1_Cmult41} {iopath_Amult1_Cmult42} {iopath_Amult1_Cmult43} {iopath_Amult1_Cmult44} {iopath_Amult1_Cmult45} {iopath_Amult1_Cmult46} {iopath_Amult1_Cmult47} {iopath_Amult1_Cmult48} {iopath_Amult1_Cmult49} {iopath_Amult1_Cmult50} {iopath_Amult1_Cmult51} {iopath_Amult1_Cmult52} {iopath_Amult1_Cmult53} {iopath_Amult1_Cmult54} {iopath_Amult1_Cmult55} {iopath_Amult1_Cmult56} {iopath_Amult1_Cmult57} {iopath_Amult1_Cmult58} {iopath_Amult1_Cmult59} {iopath_Amult1_Cmult60} {iopath_Amult1_Cmult61} {iopath_Amult1_Cmult62} {iopath_Amult1_Cmult63} 0 0 {iopath_Amult2_Cmult2} {iopath_Amult2_Cmult3} {iopath_Amult2_Cmult4} {iopath_Amult2_Cmult5} {iopath_Amult2_Cmult6} {iopath_Amult2_Cmult7} {iopath_Amult2_Cmult8} {iopath_Amult2_Cmult9} {iopath_Amult2_Cmult10} {iopath_Amult2_Cmult11} {iopath_Amult2_Cmult12} {iopath_Amult2_Cmult13} {iopath_Amult2_Cmult14} {iopath_Amult2_Cmult15} {iopath_Amult2_Cmult16} {iopath_Amult2_Cmult17} {iopath_Amult2_Cmult18} {iopath_Amult2_Cmult19} {iopath_Amult2_Cmult20} {iopath_Amult2_Cmult21} {iopath_Amult2_Cmult22} {iopath_Amult2_Cmult23} {iopath_Amult2_Cmult24} {iopath_Amult2_Cmult25} {iopath_Amult2_Cmult26} {iopath_Amult2_Cmult27} {iopath_Amult2_Cmult28} {iopath_Amult2_Cmult29} {iopath_Amult2_Cmult30} {iopath_Amult2_Cmult31} {iopath_Amult2_Cmult32} {iopath_Amult2_Cmult33} {iopath_Amult2_Cmult34} {iopath_Amult2_Cmult35} {iopath_Amult2_Cmult36} {iopath_Amult2_Cmult37} {iopath_Amult2_Cmult38} {iopath_Amult2_Cmult39} {iopath_Amult2_Cmult40} {iopath_Amult2_Cmult41} {iopath_Amult2_Cmult42} {iopath_Amult2_Cmult43} {iopath_Amult2_Cmult44} {iopath_Amult2_Cmult45} {iopath_Amult2_Cmult46} {iopath_Amult2_Cmult47} {iopath_Amult2_Cmult48} {iopath_Amult2_Cmult49} {iopath_Amult2_Cmult50} {iopath_Amult2_Cmult51} {iopath_Amult2_Cmult52} {iopath_Amult2_Cmult53} {iopath_Amult2_Cmult54} {iopath_Amult2_Cmult55} {iopath_Amult2_Cmult56} {iopath_Amult2_Cmult57} {iopath_Amult2_Cmult58} {iopath_Amult2_Cmult59} {iopath_Amult2_Cmult60} {iopath_Amult2_Cmult61} {iopath_Amult2_Cmult62} {iopath_Amult2_Cmult63} 0 0 0 {iopath_Amult3_Cmult3} {iopath_Amult3_Cmult4} {iopath_Amult3_Cmult5} {iopath_Amult3_Cmult6} {iopath_Amult3_Cmult7} {iopath_Amult3_Cmult8} {iopath_Amult3_Cmult9} {iopath_Amult3_Cmult10} {iopath_Amult3_Cmult11} {iopath_Amult3_Cmult12} {iopath_Amult3_Cmult13} {iopath_Amult3_Cmult14} {iopath_Amult3_Cmult15} {iopath_Amult3_Cmult16} {iopath_Amult3_Cmult17} {iopath_Amult3_Cmult18} {iopath_Amult3_Cmult19} {iopath_Amult3_Cmult20} {iopath_Amult3_Cmult21} {iopath_Amult3_Cmult22} {iopath_Amult3_Cmult23} {iopath_Amult3_Cmult24} {iopath_Amult3_Cmult25} {iopath_Amult3_Cmult26} {iopath_Amult3_Cmult27} {iopath_Amult3_Cmult28} {iopath_Amult3_Cmult29} {iopath_Amult3_Cmult30} {iopath_Amult3_Cmult31} {iopath_Amult3_Cmult32} {iopath_Amult3_Cmult33} {iopath_Amult3_Cmult34} {iopath_Amult3_Cmult35} {iopath_Amult3_Cmult36} {iopath_Amult3_Cmult37} {iopath_Amult3_Cmult38} {iopath_Amult3_Cmult39} {iopath_Amult3_Cmult40} {iopath_Amult3_Cmult41} {iopath_Amult3_Cmult42} {iopath_Amult3_Cmult43} {iopath_Amult3_Cmult44} {iopath_Amult3_Cmult45} {iopath_Amult3_Cmult46} {iopath_Amult3_Cmult47} {iopath_Amult3_Cmult48} {iopath_Amult3_Cmult49} {iopath_Amult3_Cmult50} {iopath_Amult3_Cmult51} {iopath_Amult3_Cmult52} {iopath_Amult3_Cmult53} {iopath_Amult3_Cmult54} {iopath_Amult3_Cmult55} {iopath_Amult3_Cmult56} {iopath_Amult3_Cmult57} {iopath_Amult3_Cmult58} {iopath_Amult3_Cmult59} {iopath_Amult3_Cmult60} {iopath_Amult3_Cmult61} {iopath_Amult3_Cmult62} {iopath_Amult3_Cmult63} 0 0 0 0 {iopath_Amult4_Cmult4} {iopath_Amult4_Cmult5} {iopath_Amult4_Cmult6} {iopath_Amult4_Cmult7} {iopath_Amult4_Cmult8} {iopath_Amult4_Cmult9} {iopath_Amult4_Cmult10} {iopath_Amult4_Cmult11} {iopath_Amult4_Cmult12} {iopath_Amult4_Cmult13} {iopath_Amult4_Cmult14} {iopath_Amult4_Cmult15} {iopath_Amult4_Cmult16} {iopath_Amult4_Cmult17} {iopath_Amult4_Cmult18} {iopath_Amult4_Cmult19} {iopath_Amult4_Cmult20} {iopath_Amult4_Cmult21} {iopath_Amult4_Cmult22} {iopath_Amult4_Cmult23} {iopath_Amult4_Cmult24} {iopath_Amult4_Cmult25} {iopath_Amult4_Cmult26} {iopath_Amult4_Cmult27} {iopath_Amult4_Cmult28} {iopath_Amult4_Cmult29} {iopath_Amult4_Cmult30} {iopath_Amult4_Cmult31} {iopath_Amult4_Cmult32} {iopath_Amult4_Cmult33} {iopath_Amult4_Cmult34} {iopath_Amult4_Cmult35} {iopath_Amult4_Cmult36} {iopath_Amult4_Cmult37} {iopath_Amult4_Cmult38} {iopath_Amult4_Cmult39} {iopath_Amult4_Cmult40} {iopath_Amult4_Cmult41} {iopath_Amult4_Cmult42} {iopath_Amult4_Cmult43} {iopath_Amult4_Cmult44} {iopath_Amult4_Cmult45} {iopath_Amult4_Cmult46} {iopath_Amult4_Cmult47} {iopath_Amult4_Cmult48} {iopath_Amult4_Cmult49} {iopath_Amult4_Cmult50} {iopath_Amult4_Cmult51} {iopath_Amult4_Cmult52} {iopath_Amult4_Cmult53} {iopath_Amult4_Cmult54} {iopath_Amult4_Cmult55} {iopath_Amult4_Cmult56} {iopath_Amult4_Cmult57} {iopath_Amult4_Cmult58} {iopath_Amult4_Cmult59} {iopath_Amult4_Cmult60} {iopath_Amult4_Cmult61} {iopath_Amult4_Cmult62} {iopath_Amult4_Cmult63} 0 0 0 0 0 {iopath_Amult5_Cmult5} {iopath_Amult5_Cmult6} {iopath_Amult5_Cmult7} {iopath_Amult5_Cmult8} {iopath_Amult5_Cmult9} {iopath_Amult5_Cmult10} {iopath_Amult5_Cmult11} {iopath_Amult5_Cmult12} {iopath_Amult5_Cmult13} {iopath_Amult5_Cmult14} {iopath_Amult5_Cmult15} {iopath_Amult5_Cmult16} {iopath_Amult5_Cmult17} {iopath_Amult5_Cmult18} {iopath_Amult5_Cmult19} {iopath_Amult5_Cmult20} {iopath_Amult5_Cmult21} {iopath_Amult5_Cmult22} {iopath_Amult5_Cmult23} {iopath_Amult5_Cmult24} {iopath_Amult5_Cmult25} {iopath_Amult5_Cmult26} {iopath_Amult5_Cmult27} {iopath_Amult5_Cmult28} {iopath_Amult5_Cmult29} {iopath_Amult5_Cmult30} {iopath_Amult5_Cmult31} {iopath_Amult5_Cmult32} {iopath_Amult5_Cmult33} {iopath_Amult5_Cmult34} {iopath_Amult5_Cmult35} {iopath_Amult5_Cmult36} {iopath_Amult5_Cmult37} {iopath_Amult5_Cmult38} {iopath_Amult5_Cmult39} {iopath_Amult5_Cmult40} {iopath_Amult5_Cmult41} {iopath_Amult5_Cmult42} {iopath_Amult5_Cmult43} {iopath_Amult5_Cmult44} {iopath_Amult5_Cmult45} {iopath_Amult5_Cmult46} {iopath_Amult5_Cmult47} {iopath_Amult5_Cmult48} {iopath_Amult5_Cmult49} {iopath_Amult5_Cmult50} {iopath_Amult5_Cmult51} {iopath_Amult5_Cmult52} {iopath_Amult5_Cmult53} {iopath_Amult5_Cmult54} {iopath_Amult5_Cmult55} {iopath_Amult5_Cmult56} {iopath_Amult5_Cmult57} {iopath_Amult5_Cmult58} {iopath_Amult5_Cmult59} {iopath_Amult5_Cmult60} {iopath_Amult5_Cmult61} {iopath_Amult5_Cmult62} {iopath_Amult5_Cmult63} 0 0 0 0 0 0 {iopath_Amult6_Cmult6} {iopath_Amult6_Cmult7} {iopath_Amult6_Cmult8} {iopath_Amult6_Cmult9} {iopath_Amult6_Cmult10} {iopath_Amult6_Cmult11} {iopath_Amult6_Cmult12} {iopath_Amult6_Cmult13} {iopath_Amult6_Cmult14} {iopath_Amult6_Cmult15} {iopath_Amult6_Cmult16} {iopath_Amult6_Cmult17} {iopath_Amult6_Cmult18} {iopath_Amult6_Cmult19} {iopath_Amult6_Cmult20} {iopath_Amult6_Cmult21} {iopath_Amult6_Cmult22} {iopath_Amult6_Cmult23} {iopath_Amult6_Cmult24} {iopath_Amult6_Cmult25} {iopath_Amult6_Cmult26} {iopath_Amult6_Cmult27} {iopath_Amult6_Cmult28} {iopath_Amult6_Cmult29} {iopath_Amult6_Cmult30} {iopath_Amult6_Cmult31} {iopath_Amult6_Cmult32} {iopath_Amult6_Cmult33} {iopath_Amult6_Cmult34} {iopath_Amult6_Cmult35} {iopath_Amult6_Cmult36} {iopath_Amult6_Cmult37} {iopath_Amult6_Cmult38} {iopath_Amult6_Cmult39} {iopath_Amult6_Cmult40} {iopath_Amult6_Cmult41} {iopath_Amult6_Cmult42} {iopath_Amult6_Cmult43} {iopath_Amult6_Cmult44} {iopath_Amult6_Cmult45} {iopath_Amult6_Cmult46} {iopath_Amult6_Cmult47} {iopath_Amult6_Cmult48} {iopath_Amult6_Cmult49} {iopath_Amult6_Cmult50} {iopath_Amult6_Cmult51} {iopath_Amult6_Cmult52} {iopath_Amult6_Cmult53} {iopath_Amult6_Cmult54} {iopath_Amult6_Cmult55} {iopath_Amult6_Cmult56} {iopath_Amult6_Cmult57} {iopath_Amult6_Cmult58} {iopath_Amult6_Cmult59} {iopath_Amult6_Cmult60} {iopath_Amult6_Cmult61} {iopath_Amult6_Cmult62} {iopath_Amult6_Cmult63} 0 0 0 0 0 0 0 {iopath_Amult7_Cmult7} {iopath_Amult7_Cmult8} {iopath_Amult7_Cmult9} {iopath_Amult7_Cmult10} {iopath_Amult7_Cmult11} {iopath_Amult7_Cmult12} {iopath_Amult7_Cmult13} {iopath_Amult7_Cmult14} {iopath_Amult7_Cmult15} {iopath_Amult7_Cmult16} {iopath_Amult7_Cmult17} {iopath_Amult7_Cmult18} {iopath_Amult7_Cmult19} {iopath_Amult7_Cmult20} {iopath_Amult7_Cmult21} {iopath_Amult7_Cmult22} {iopath_Amult7_Cmult23} {iopath_Amult7_Cmult24} {iopath_Amult7_Cmult25} {iopath_Amult7_Cmult26} {iopath_Amult7_Cmult27} {iopath_Amult7_Cmult28} {iopath_Amult7_Cmult29} {iopath_Amult7_Cmult30} {iopath_Amult7_Cmult31} {iopath_Amult7_Cmult32} {iopath_Amult7_Cmult33} {iopath_Amult7_Cmult34} {iopath_Amult7_Cmult35} {iopath_Amult7_Cmult36} {iopath_Amult7_Cmult37} {iopath_Amult7_Cmult38} {iopath_Amult7_Cmult39} {iopath_Amult7_Cmult40} {iopath_Amult7_Cmult41} {iopath_Amult7_Cmult42} {iopath_Amult7_Cmult43} {iopath_Amult7_Cmult44} {iopath_Amult7_Cmult45} {iopath_Amult7_Cmult46} {iopath_Amult7_Cmult47} {iopath_Amult7_Cmult48} {iopath_Amult7_Cmult49} {iopath_Amult7_Cmult50} {iopath_Amult7_Cmult51} {iopath_Amult7_Cmult52} {iopath_Amult7_Cmult53} {iopath_Amult7_Cmult54} {iopath_Amult7_Cmult55} {iopath_Amult7_Cmult56} {iopath_Amult7_Cmult57} {iopath_Amult7_Cmult58} {iopath_Amult7_Cmult59} {iopath_Amult7_Cmult60} {iopath_Amult7_Cmult61} {iopath_Amult7_Cmult62} {iopath_Amult7_Cmult63} 0 0 0 0 0 0 0 0 {iopath_Amult8_Cmult8} {iopath_Amult8_Cmult9} {iopath_Amult8_Cmult10} {iopath_Amult8_Cmult11} {iopath_Amult8_Cmult12} {iopath_Amult8_Cmult13} {iopath_Amult8_Cmult14} {iopath_Amult8_Cmult15} {iopath_Amult8_Cmult16} {iopath_Amult8_Cmult17} {iopath_Amult8_Cmult18} {iopath_Amult8_Cmult19} {iopath_Amult8_Cmult20} {iopath_Amult8_Cmult21} {iopath_Amult8_Cmult22} {iopath_Amult8_Cmult23} {iopath_Amult8_Cmult24} {iopath_Amult8_Cmult25} {iopath_Amult8_Cmult26} {iopath_Amult8_Cmult27} {iopath_Amult8_Cmult28} {iopath_Amult8_Cmult29} {iopath_Amult8_Cmult30} {iopath_Amult8_Cmult31} {iopath_Amult8_Cmult32} {iopath_Amult8_Cmult33} {iopath_Amult8_Cmult34} {iopath_Amult8_Cmult35} {iopath_Amult8_Cmult36} {iopath_Amult8_Cmult37} {iopath_Amult8_Cmult38} {iopath_Amult8_Cmult39} {iopath_Amult8_Cmult40} {iopath_Amult8_Cmult41} {iopath_Amult8_Cmult42} {iopath_Amult8_Cmult43} {iopath_Amult8_Cmult44} {iopath_Amult8_Cmult45} {iopath_Amult8_Cmult46} {iopath_Amult8_Cmult47} {iopath_Amult8_Cmult48} {iopath_Amult8_Cmult49} {iopath_Amult8_Cmult50} {iopath_Amult8_Cmult51} {iopath_Amult8_Cmult52} {iopath_Amult8_Cmult53} {iopath_Amult8_Cmult54} {iopath_Amult8_Cmult55} {iopath_Amult8_Cmult56} {iopath_Amult8_Cmult57} {iopath_Amult8_Cmult58} {iopath_Amult8_Cmult59} {iopath_Amult8_Cmult60} {iopath_Amult8_Cmult61} {iopath_Amult8_Cmult62} {iopath_Amult8_Cmult63} 0 0 0 0 0 0 0 0 0 {iopath_Amult9_Cmult9} {iopath_Amult9_Cmult10} {iopath_Amult9_Cmult11} {iopath_Amult9_Cmult12} {iopath_Amult9_Cmult13} {iopath_Amult9_Cmult14} {iopath_Amult9_Cmult15} {iopath_Amult9_Cmult16} {iopath_Amult9_Cmult17} {iopath_Amult9_Cmult18} {iopath_Amult9_Cmult19} {iopath_Amult9_Cmult20} {iopath_Amult9_Cmult21} {iopath_Amult9_Cmult22} {iopath_Amult9_Cmult23} {iopath_Amult9_Cmult24} {iopath_Amult9_Cmult25} {iopath_Amult9_Cmult26} {iopath_Amult9_Cmult27} {iopath_Amult9_Cmult28} {iopath_Amult9_Cmult29} {iopath_Amult9_Cmult30} {iopath_Amult9_Cmult31} {iopath_Amult9_Cmult32} {iopath_Amult9_Cmult33} {iopath_Amult9_Cmult34} {iopath_Amult9_Cmult35} {iopath_Amult9_Cmult36} {iopath_Amult9_Cmult37} {iopath_Amult9_Cmult38} {iopath_Amult9_Cmult39} {iopath_Amult9_Cmult40} {iopath_Amult9_Cmult41} {iopath_Amult9_Cmult42} {iopath_Amult9_Cmult43} {iopath_Amult9_Cmult44} {iopath_Amult9_Cmult45} {iopath_Amult9_Cmult46} {iopath_Amult9_Cmult47} {iopath_Amult9_Cmult48} {iopath_Amult9_Cmult49} {iopath_Amult9_Cmult50} {iopath_Amult9_Cmult51} {iopath_Amult9_Cmult52} {iopath_Amult9_Cmult53} {iopath_Amult9_Cmult54} {iopath_Amult9_Cmult55} {iopath_Amult9_Cmult56} {iopath_Amult9_Cmult57} {iopath_Amult9_Cmult58} {iopath_Amult9_Cmult59} {iopath_Amult9_Cmult60} {iopath_Amult9_Cmult61} {iopath_Amult9_Cmult62} {iopath_Amult9_Cmult63} 0 0 0 0 0 0 0 0 0 0 {iopath_Amult10_Cmult10} {iopath_Amult10_Cmult11} {iopath_Amult10_Cmult12} {iopath_Amult10_Cmult13} {iopath_Amult10_Cmult14} {iopath_Amult10_Cmult15} {iopath_Amult10_Cmult16} {iopath_Amult10_Cmult17} {iopath_Amult10_Cmult18} {iopath_Amult10_Cmult19} {iopath_Amult10_Cmult20} {iopath_Amult10_Cmult21} {iopath_Amult10_Cmult22} {iopath_Amult10_Cmult23} {iopath_Amult10_Cmult24} {iopath_Amult10_Cmult25} {iopath_Amult10_Cmult26} {iopath_Amult10_Cmult27} {iopath_Amult10_Cmult28} {iopath_Amult10_Cmult29} {iopath_Amult10_Cmult30} {iopath_Amult10_Cmult31} {iopath_Amult10_Cmult32} {iopath_Amult10_Cmult33} {iopath_Amult10_Cmult34} {iopath_Amult10_Cmult35} {iopath_Amult10_Cmult36} {iopath_Amult10_Cmult37} {iopath_Amult10_Cmult38} {iopath_Amult10_Cmult39} {iopath_Amult10_Cmult40} {iopath_Amult10_Cmult41} {iopath_Amult10_Cmult42} {iopath_Amult10_Cmult43} {iopath_Amult10_Cmult44} {iopath_Amult10_Cmult45} {iopath_Amult10_Cmult46} {iopath_Amult10_Cmult47} {iopath_Amult10_Cmult48} {iopath_Amult10_Cmult49} {iopath_Amult10_Cmult50} {iopath_Amult10_Cmult51} {iopath_Amult10_Cmult52} {iopath_Amult10_Cmult53} {iopath_Amult10_Cmult54} {iopath_Amult10_Cmult55} {iopath_Amult10_Cmult56} {iopath_Amult10_Cmult57} {iopath_Amult10_Cmult58} {iopath_Amult10_Cmult59} {iopath_Amult10_Cmult60} {iopath_Amult10_Cmult61} {iopath_Amult10_Cmult62} {iopath_Amult10_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult11_Cmult11} {iopath_Amult11_Cmult12} {iopath_Amult11_Cmult13} {iopath_Amult11_Cmult14} {iopath_Amult11_Cmult15} {iopath_Amult11_Cmult16} {iopath_Amult11_Cmult17} {iopath_Amult11_Cmult18} {iopath_Amult11_Cmult19} {iopath_Amult11_Cmult20} {iopath_Amult11_Cmult21} {iopath_Amult11_Cmult22} {iopath_Amult11_Cmult23} {iopath_Amult11_Cmult24} {iopath_Amult11_Cmult25} {iopath_Amult11_Cmult26} {iopath_Amult11_Cmult27} {iopath_Amult11_Cmult28} {iopath_Amult11_Cmult29} {iopath_Amult11_Cmult30} {iopath_Amult11_Cmult31} {iopath_Amult11_Cmult32} {iopath_Amult11_Cmult33} {iopath_Amult11_Cmult34} {iopath_Amult11_Cmult35} {iopath_Amult11_Cmult36} {iopath_Amult11_Cmult37} {iopath_Amult11_Cmult38} {iopath_Amult11_Cmult39} {iopath_Amult11_Cmult40} {iopath_Amult11_Cmult41} {iopath_Amult11_Cmult42} {iopath_Amult11_Cmult43} {iopath_Amult11_Cmult44} {iopath_Amult11_Cmult45} {iopath_Amult11_Cmult46} {iopath_Amult11_Cmult47} {iopath_Amult11_Cmult48} {iopath_Amult11_Cmult49} {iopath_Amult11_Cmult50} {iopath_Amult11_Cmult51} {iopath_Amult11_Cmult52} {iopath_Amult11_Cmult53} {iopath_Amult11_Cmult54} {iopath_Amult11_Cmult55} {iopath_Amult11_Cmult56} {iopath_Amult11_Cmult57} {iopath_Amult11_Cmult58} {iopath_Amult11_Cmult59} {iopath_Amult11_Cmult60} {iopath_Amult11_Cmult61} {iopath_Amult11_Cmult62} {iopath_Amult11_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult12_Cmult12} {iopath_Amult12_Cmult13} {iopath_Amult12_Cmult14} {iopath_Amult12_Cmult15} {iopath_Amult12_Cmult16} {iopath_Amult12_Cmult17} {iopath_Amult12_Cmult18} {iopath_Amult12_Cmult19} {iopath_Amult12_Cmult20} {iopath_Amult12_Cmult21} {iopath_Amult12_Cmult22} {iopath_Amult12_Cmult23} {iopath_Amult12_Cmult24} {iopath_Amult12_Cmult25} {iopath_Amult12_Cmult26} {iopath_Amult12_Cmult27} {iopath_Amult12_Cmult28} {iopath_Amult12_Cmult29} {iopath_Amult12_Cmult30} {iopath_Amult12_Cmult31} {iopath_Amult12_Cmult32} {iopath_Amult12_Cmult33} {iopath_Amult12_Cmult34} {iopath_Amult12_Cmult35} {iopath_Amult12_Cmult36} {iopath_Amult12_Cmult37} {iopath_Amult12_Cmult38} {iopath_Amult12_Cmult39} {iopath_Amult12_Cmult40} {iopath_Amult12_Cmult41} {iopath_Amult12_Cmult42} {iopath_Amult12_Cmult43} {iopath_Amult12_Cmult44} {iopath_Amult12_Cmult45} {iopath_Amult12_Cmult46} {iopath_Amult12_Cmult47} {iopath_Amult12_Cmult48} {iopath_Amult12_Cmult49} {iopath_Amult12_Cmult50} {iopath_Amult12_Cmult51} {iopath_Amult12_Cmult52} {iopath_Amult12_Cmult53} {iopath_Amult12_Cmult54} {iopath_Amult12_Cmult55} {iopath_Amult12_Cmult56} {iopath_Amult12_Cmult57} {iopath_Amult12_Cmult58} {iopath_Amult12_Cmult59} {iopath_Amult12_Cmult60} {iopath_Amult12_Cmult61} {iopath_Amult12_Cmult62} {iopath_Amult12_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult13_Cmult13} {iopath_Amult13_Cmult14} {iopath_Amult13_Cmult15} {iopath_Amult13_Cmult16} {iopath_Amult13_Cmult17} {iopath_Amult13_Cmult18} {iopath_Amult13_Cmult19} {iopath_Amult13_Cmult20} {iopath_Amult13_Cmult21} {iopath_Amult13_Cmult22} {iopath_Amult13_Cmult23} {iopath_Amult13_Cmult24} {iopath_Amult13_Cmult25} {iopath_Amult13_Cmult26} {iopath_Amult13_Cmult27} {iopath_Amult13_Cmult28} {iopath_Amult13_Cmult29} {iopath_Amult13_Cmult30} {iopath_Amult13_Cmult31} {iopath_Amult13_Cmult32} {iopath_Amult13_Cmult33} {iopath_Amult13_Cmult34} {iopath_Amult13_Cmult35} {iopath_Amult13_Cmult36} {iopath_Amult13_Cmult37} {iopath_Amult13_Cmult38} {iopath_Amult13_Cmult39} {iopath_Amult13_Cmult40} {iopath_Amult13_Cmult41} {iopath_Amult13_Cmult42} {iopath_Amult13_Cmult43} {iopath_Amult13_Cmult44} {iopath_Amult13_Cmult45} {iopath_Amult13_Cmult46} {iopath_Amult13_Cmult47} {iopath_Amult13_Cmult48} {iopath_Amult13_Cmult49} {iopath_Amult13_Cmult50} {iopath_Amult13_Cmult51} {iopath_Amult13_Cmult52} {iopath_Amult13_Cmult53} {iopath_Amult13_Cmult54} {iopath_Amult13_Cmult55} {iopath_Amult13_Cmult56} {iopath_Amult13_Cmult57} {iopath_Amult13_Cmult58} {iopath_Amult13_Cmult59} {iopath_Amult13_Cmult60} {iopath_Amult13_Cmult61} {iopath_Amult13_Cmult62} {iopath_Amult13_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult14_Cmult14} {iopath_Amult14_Cmult15} {iopath_Amult14_Cmult16} {iopath_Amult14_Cmult17} {iopath_Amult14_Cmult18} {iopath_Amult14_Cmult19} {iopath_Amult14_Cmult20} {iopath_Amult14_Cmult21} {iopath_Amult14_Cmult22} {iopath_Amult14_Cmult23} {iopath_Amult14_Cmult24} {iopath_Amult14_Cmult25} {iopath_Amult14_Cmult26} {iopath_Amult14_Cmult27} {iopath_Amult14_Cmult28} {iopath_Amult14_Cmult29} {iopath_Amult14_Cmult30} {iopath_Amult14_Cmult31} {iopath_Amult14_Cmult32} {iopath_Amult14_Cmult33} {iopath_Amult14_Cmult34} {iopath_Amult14_Cmult35} {iopath_Amult14_Cmult36} {iopath_Amult14_Cmult37} {iopath_Amult14_Cmult38} {iopath_Amult14_Cmult39} {iopath_Amult14_Cmult40} {iopath_Amult14_Cmult41} {iopath_Amult14_Cmult42} {iopath_Amult14_Cmult43} {iopath_Amult14_Cmult44} {iopath_Amult14_Cmult45} {iopath_Amult14_Cmult46} {iopath_Amult14_Cmult47} {iopath_Amult14_Cmult48} {iopath_Amult14_Cmult49} {iopath_Amult14_Cmult50} {iopath_Amult14_Cmult51} {iopath_Amult14_Cmult52} {iopath_Amult14_Cmult53} {iopath_Amult14_Cmult54} {iopath_Amult14_Cmult55} {iopath_Amult14_Cmult56} {iopath_Amult14_Cmult57} {iopath_Amult14_Cmult58} {iopath_Amult14_Cmult59} {iopath_Amult14_Cmult60} {iopath_Amult14_Cmult61} {iopath_Amult14_Cmult62} {iopath_Amult14_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult15_Cmult15} {iopath_Amult15_Cmult16} {iopath_Amult15_Cmult17} {iopath_Amult15_Cmult18} {iopath_Amult15_Cmult19} {iopath_Amult15_Cmult20} {iopath_Amult15_Cmult21} {iopath_Amult15_Cmult22} {iopath_Amult15_Cmult23} {iopath_Amult15_Cmult24} {iopath_Amult15_Cmult25} {iopath_Amult15_Cmult26} {iopath_Amult15_Cmult27} {iopath_Amult15_Cmult28} {iopath_Amult15_Cmult29} {iopath_Amult15_Cmult30} {iopath_Amult15_Cmult31} {iopath_Amult15_Cmult32} {iopath_Amult15_Cmult33} {iopath_Amult15_Cmult34} {iopath_Amult15_Cmult35} {iopath_Amult15_Cmult36} {iopath_Amult15_Cmult37} {iopath_Amult15_Cmult38} {iopath_Amult15_Cmult39} {iopath_Amult15_Cmult40} {iopath_Amult15_Cmult41} {iopath_Amult15_Cmult42} {iopath_Amult15_Cmult43} {iopath_Amult15_Cmult44} {iopath_Amult15_Cmult45} {iopath_Amult15_Cmult46} {iopath_Amult15_Cmult47} {iopath_Amult15_Cmult48} {iopath_Amult15_Cmult49} {iopath_Amult15_Cmult50} {iopath_Amult15_Cmult51} {iopath_Amult15_Cmult52} {iopath_Amult15_Cmult53} {iopath_Amult15_Cmult54} {iopath_Amult15_Cmult55} {iopath_Amult15_Cmult56} {iopath_Amult15_Cmult57} {iopath_Amult15_Cmult58} {iopath_Amult15_Cmult59} {iopath_Amult15_Cmult60} {iopath_Amult15_Cmult61} {iopath_Amult15_Cmult62} {iopath_Amult15_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult16_Cmult16} {iopath_Amult16_Cmult17} {iopath_Amult16_Cmult18} {iopath_Amult16_Cmult19} {iopath_Amult16_Cmult20} {iopath_Amult16_Cmult21} {iopath_Amult16_Cmult22} {iopath_Amult16_Cmult23} {iopath_Amult16_Cmult24} {iopath_Amult16_Cmult25} {iopath_Amult16_Cmult26} {iopath_Amult16_Cmult27} {iopath_Amult16_Cmult28} {iopath_Amult16_Cmult29} {iopath_Amult16_Cmult30} {iopath_Amult16_Cmult31} {iopath_Amult16_Cmult32} {iopath_Amult16_Cmult33} {iopath_Amult16_Cmult34} {iopath_Amult16_Cmult35} {iopath_Amult16_Cmult36} {iopath_Amult16_Cmult37} {iopath_Amult16_Cmult38} {iopath_Amult16_Cmult39} {iopath_Amult16_Cmult40} {iopath_Amult16_Cmult41} {iopath_Amult16_Cmult42} {iopath_Amult16_Cmult43} {iopath_Amult16_Cmult44} {iopath_Amult16_Cmult45} {iopath_Amult16_Cmult46} {iopath_Amult16_Cmult47} {iopath_Amult16_Cmult48} {iopath_Amult16_Cmult49} {iopath_Amult16_Cmult50} {iopath_Amult16_Cmult51} {iopath_Amult16_Cmult52} {iopath_Amult16_Cmult53} {iopath_Amult16_Cmult54} {iopath_Amult16_Cmult55} {iopath_Amult16_Cmult56} {iopath_Amult16_Cmult57} {iopath_Amult16_Cmult58} {iopath_Amult16_Cmult59} {iopath_Amult16_Cmult60} {iopath_Amult16_Cmult61} {iopath_Amult16_Cmult62} {iopath_Amult16_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult17_Cmult17} {iopath_Amult17_Cmult18} {iopath_Amult17_Cmult19} {iopath_Amult17_Cmult20} {iopath_Amult17_Cmult21} {iopath_Amult17_Cmult22} {iopath_Amult17_Cmult23} {iopath_Amult17_Cmult24} {iopath_Amult17_Cmult25} {iopath_Amult17_Cmult26} {iopath_Amult17_Cmult27} {iopath_Amult17_Cmult28} {iopath_Amult17_Cmult29} {iopath_Amult17_Cmult30} {iopath_Amult17_Cmult31} {iopath_Amult17_Cmult32} {iopath_Amult17_Cmult33} {iopath_Amult17_Cmult34} {iopath_Amult17_Cmult35} {iopath_Amult17_Cmult36} {iopath_Amult17_Cmult37} {iopath_Amult17_Cmult38} {iopath_Amult17_Cmult39} {iopath_Amult17_Cmult40} {iopath_Amult17_Cmult41} {iopath_Amult17_Cmult42} {iopath_Amult17_Cmult43} {iopath_Amult17_Cmult44} {iopath_Amult17_Cmult45} {iopath_Amult17_Cmult46} {iopath_Amult17_Cmult47} {iopath_Amult17_Cmult48} {iopath_Amult17_Cmult49} {iopath_Amult17_Cmult50} {iopath_Amult17_Cmult51} {iopath_Amult17_Cmult52} {iopath_Amult17_Cmult53} {iopath_Amult17_Cmult54} {iopath_Amult17_Cmult55} {iopath_Amult17_Cmult56} {iopath_Amult17_Cmult57} {iopath_Amult17_Cmult58} {iopath_Amult17_Cmult59} {iopath_Amult17_Cmult60} {iopath_Amult17_Cmult61} {iopath_Amult17_Cmult62} {iopath_Amult17_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult18_Cmult18} {iopath_Amult18_Cmult19} {iopath_Amult18_Cmult20} {iopath_Amult18_Cmult21} {iopath_Amult18_Cmult22} {iopath_Amult18_Cmult23} {iopath_Amult18_Cmult24} {iopath_Amult18_Cmult25} {iopath_Amult18_Cmult26} {iopath_Amult18_Cmult27} {iopath_Amult18_Cmult28} {iopath_Amult18_Cmult29} {iopath_Amult18_Cmult30} {iopath_Amult18_Cmult31} {iopath_Amult18_Cmult32} {iopath_Amult18_Cmult33} {iopath_Amult18_Cmult34} {iopath_Amult18_Cmult35} {iopath_Amult18_Cmult36} {iopath_Amult18_Cmult37} {iopath_Amult18_Cmult38} {iopath_Amult18_Cmult39} {iopath_Amult18_Cmult40} {iopath_Amult18_Cmult41} {iopath_Amult18_Cmult42} {iopath_Amult18_Cmult43} {iopath_Amult18_Cmult44} {iopath_Amult18_Cmult45} {iopath_Amult18_Cmult46} {iopath_Amult18_Cmult47} {iopath_Amult18_Cmult48} {iopath_Amult18_Cmult49} {iopath_Amult18_Cmult50} {iopath_Amult18_Cmult51} {iopath_Amult18_Cmult52} {iopath_Amult18_Cmult53} {iopath_Amult18_Cmult54} {iopath_Amult18_Cmult55} {iopath_Amult18_Cmult56} {iopath_Amult18_Cmult57} {iopath_Amult18_Cmult58} {iopath_Amult18_Cmult59} {iopath_Amult18_Cmult60} {iopath_Amult18_Cmult61} {iopath_Amult18_Cmult62} {iopath_Amult18_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult19_Cmult19} {iopath_Amult19_Cmult20} {iopath_Amult19_Cmult21} {iopath_Amult19_Cmult22} {iopath_Amult19_Cmult23} {iopath_Amult19_Cmult24} {iopath_Amult19_Cmult25} {iopath_Amult19_Cmult26} {iopath_Amult19_Cmult27} {iopath_Amult19_Cmult28} {iopath_Amult19_Cmult29} {iopath_Amult19_Cmult30} {iopath_Amult19_Cmult31} {iopath_Amult19_Cmult32} {iopath_Amult19_Cmult33} {iopath_Amult19_Cmult34} {iopath_Amult19_Cmult35} {iopath_Amult19_Cmult36} {iopath_Amult19_Cmult37} {iopath_Amult19_Cmult38} {iopath_Amult19_Cmult39} {iopath_Amult19_Cmult40} {iopath_Amult19_Cmult41} {iopath_Amult19_Cmult42} {iopath_Amult19_Cmult43} {iopath_Amult19_Cmult44} {iopath_Amult19_Cmult45} {iopath_Amult19_Cmult46} {iopath_Amult19_Cmult47} {iopath_Amult19_Cmult48} {iopath_Amult19_Cmult49} {iopath_Amult19_Cmult50} {iopath_Amult19_Cmult51} {iopath_Amult19_Cmult52} {iopath_Amult19_Cmult53} {iopath_Amult19_Cmult54} {iopath_Amult19_Cmult55} {iopath_Amult19_Cmult56} {iopath_Amult19_Cmult57} {iopath_Amult19_Cmult58} {iopath_Amult19_Cmult59} {iopath_Amult19_Cmult60} {iopath_Amult19_Cmult61} {iopath_Amult19_Cmult62} {iopath_Amult19_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult20_Cmult20} {iopath_Amult20_Cmult21} {iopath_Amult20_Cmult22} {iopath_Amult20_Cmult23} {iopath_Amult20_Cmult24} {iopath_Amult20_Cmult25} {iopath_Amult20_Cmult26} {iopath_Amult20_Cmult27} {iopath_Amult20_Cmult28} {iopath_Amult20_Cmult29} {iopath_Amult20_Cmult30} {iopath_Amult20_Cmult31} {iopath_Amult20_Cmult32} {iopath_Amult20_Cmult33} {iopath_Amult20_Cmult34} {iopath_Amult20_Cmult35} {iopath_Amult20_Cmult36} {iopath_Amult20_Cmult37} {iopath_Amult20_Cmult38} {iopath_Amult20_Cmult39} {iopath_Amult20_Cmult40} {iopath_Amult20_Cmult41} {iopath_Amult20_Cmult42} {iopath_Amult20_Cmult43} {iopath_Amult20_Cmult44} {iopath_Amult20_Cmult45} {iopath_Amult20_Cmult46} {iopath_Amult20_Cmult47} {iopath_Amult20_Cmult48} {iopath_Amult20_Cmult49} {iopath_Amult20_Cmult50} {iopath_Amult20_Cmult51} {iopath_Amult20_Cmult52} {iopath_Amult20_Cmult53} {iopath_Amult20_Cmult54} {iopath_Amult20_Cmult55} {iopath_Amult20_Cmult56} {iopath_Amult20_Cmult57} {iopath_Amult20_Cmult58} {iopath_Amult20_Cmult59} {iopath_Amult20_Cmult60} {iopath_Amult20_Cmult61} {iopath_Amult20_Cmult62} {iopath_Amult20_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult21_Cmult21} {iopath_Amult21_Cmult22} {iopath_Amult21_Cmult23} {iopath_Amult21_Cmult24} {iopath_Amult21_Cmult25} {iopath_Amult21_Cmult26} {iopath_Amult21_Cmult27} {iopath_Amult21_Cmult28} {iopath_Amult21_Cmult29} {iopath_Amult21_Cmult30} {iopath_Amult21_Cmult31} {iopath_Amult21_Cmult32} {iopath_Amult21_Cmult33} {iopath_Amult21_Cmult34} {iopath_Amult21_Cmult35} {iopath_Amult21_Cmult36} {iopath_Amult21_Cmult37} {iopath_Amult21_Cmult38} {iopath_Amult21_Cmult39} {iopath_Amult21_Cmult40} {iopath_Amult21_Cmult41} {iopath_Amult21_Cmult42} {iopath_Amult21_Cmult43} {iopath_Amult21_Cmult44} {iopath_Amult21_Cmult45} {iopath_Amult21_Cmult46} {iopath_Amult21_Cmult47} {iopath_Amult21_Cmult48} {iopath_Amult21_Cmult49} {iopath_Amult21_Cmult50} {iopath_Amult21_Cmult51} {iopath_Amult21_Cmult52} {iopath_Amult21_Cmult53} {iopath_Amult21_Cmult54} {iopath_Amult21_Cmult55} {iopath_Amult21_Cmult56} {iopath_Amult21_Cmult57} {iopath_Amult21_Cmult58} {iopath_Amult21_Cmult59} {iopath_Amult21_Cmult60} {iopath_Amult21_Cmult61} {iopath_Amult21_Cmult62} {iopath_Amult21_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult22_Cmult22} {iopath_Amult22_Cmult23} {iopath_Amult22_Cmult24} {iopath_Amult22_Cmult25} {iopath_Amult22_Cmult26} {iopath_Amult22_Cmult27} {iopath_Amult22_Cmult28} {iopath_Amult22_Cmult29} {iopath_Amult22_Cmult30} {iopath_Amult22_Cmult31} {iopath_Amult22_Cmult32} {iopath_Amult22_Cmult33} {iopath_Amult22_Cmult34} {iopath_Amult22_Cmult35} {iopath_Amult22_Cmult36} {iopath_Amult22_Cmult37} {iopath_Amult22_Cmult38} {iopath_Amult22_Cmult39} {iopath_Amult22_Cmult40} {iopath_Amult22_Cmult41} {iopath_Amult22_Cmult42} {iopath_Amult22_Cmult43} {iopath_Amult22_Cmult44} {iopath_Amult22_Cmult45} {iopath_Amult22_Cmult46} {iopath_Amult22_Cmult47} {iopath_Amult22_Cmult48} {iopath_Amult22_Cmult49} {iopath_Amult22_Cmult50} {iopath_Amult22_Cmult51} {iopath_Amult22_Cmult52} {iopath_Amult22_Cmult53} {iopath_Amult22_Cmult54} {iopath_Amult22_Cmult55} {iopath_Amult22_Cmult56} {iopath_Amult22_Cmult57} {iopath_Amult22_Cmult58} {iopath_Amult22_Cmult59} {iopath_Amult22_Cmult60} {iopath_Amult22_Cmult61} {iopath_Amult22_Cmult62} {iopath_Amult22_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult23_Cmult23} {iopath_Amult23_Cmult24} {iopath_Amult23_Cmult25} {iopath_Amult23_Cmult26} {iopath_Amult23_Cmult27} {iopath_Amult23_Cmult28} {iopath_Amult23_Cmult29} {iopath_Amult23_Cmult30} {iopath_Amult23_Cmult31} {iopath_Amult23_Cmult32} {iopath_Amult23_Cmult33} {iopath_Amult23_Cmult34} {iopath_Amult23_Cmult35} {iopath_Amult23_Cmult36} {iopath_Amult23_Cmult37} {iopath_Amult23_Cmult38} {iopath_Amult23_Cmult39} {iopath_Amult23_Cmult40} {iopath_Amult23_Cmult41} {iopath_Amult23_Cmult42} {iopath_Amult23_Cmult43} {iopath_Amult23_Cmult44} {iopath_Amult23_Cmult45} {iopath_Amult23_Cmult46} {iopath_Amult23_Cmult47} {iopath_Amult23_Cmult48} {iopath_Amult23_Cmult49} {iopath_Amult23_Cmult50} {iopath_Amult23_Cmult51} {iopath_Amult23_Cmult52} {iopath_Amult23_Cmult53} {iopath_Amult23_Cmult54} {iopath_Amult23_Cmult55} {iopath_Amult23_Cmult56} {iopath_Amult23_Cmult57} {iopath_Amult23_Cmult58} {iopath_Amult23_Cmult59} {iopath_Amult23_Cmult60} {iopath_Amult23_Cmult61} {iopath_Amult23_Cmult62} {iopath_Amult23_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult24_Cmult24} {iopath_Amult24_Cmult25} {iopath_Amult24_Cmult26} {iopath_Amult24_Cmult27} {iopath_Amult24_Cmult28} {iopath_Amult24_Cmult29} {iopath_Amult24_Cmult30} {iopath_Amult24_Cmult31} {iopath_Amult24_Cmult32} {iopath_Amult24_Cmult33} {iopath_Amult24_Cmult34} {iopath_Amult24_Cmult35} {iopath_Amult24_Cmult36} {iopath_Amult24_Cmult37} {iopath_Amult24_Cmult38} {iopath_Amult24_Cmult39} {iopath_Amult24_Cmult40} {iopath_Amult24_Cmult41} {iopath_Amult24_Cmult42} {iopath_Amult24_Cmult43} {iopath_Amult24_Cmult44} {iopath_Amult24_Cmult45} {iopath_Amult24_Cmult46} {iopath_Amult24_Cmult47} {iopath_Amult24_Cmult48} {iopath_Amult24_Cmult49} {iopath_Amult24_Cmult50} {iopath_Amult24_Cmult51} {iopath_Amult24_Cmult52} {iopath_Amult24_Cmult53} {iopath_Amult24_Cmult54} {iopath_Amult24_Cmult55} {iopath_Amult24_Cmult56} {iopath_Amult24_Cmult57} {iopath_Amult24_Cmult58} {iopath_Amult24_Cmult59} {iopath_Amult24_Cmult60} {iopath_Amult24_Cmult61} {iopath_Amult24_Cmult62} {iopath_Amult24_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult25_Cmult25} {iopath_Amult25_Cmult26} {iopath_Amult25_Cmult27} {iopath_Amult25_Cmult28} {iopath_Amult25_Cmult29} {iopath_Amult25_Cmult30} {iopath_Amult25_Cmult31} {iopath_Amult25_Cmult32} {iopath_Amult25_Cmult33} {iopath_Amult25_Cmult34} {iopath_Amult25_Cmult35} {iopath_Amult25_Cmult36} {iopath_Amult25_Cmult37} {iopath_Amult25_Cmult38} {iopath_Amult25_Cmult39} {iopath_Amult25_Cmult40} {iopath_Amult25_Cmult41} {iopath_Amult25_Cmult42} {iopath_Amult25_Cmult43} {iopath_Amult25_Cmult44} {iopath_Amult25_Cmult45} {iopath_Amult25_Cmult46} {iopath_Amult25_Cmult47} {iopath_Amult25_Cmult48} {iopath_Amult25_Cmult49} {iopath_Amult25_Cmult50} {iopath_Amult25_Cmult51} {iopath_Amult25_Cmult52} {iopath_Amult25_Cmult53} {iopath_Amult25_Cmult54} {iopath_Amult25_Cmult55} {iopath_Amult25_Cmult56} {iopath_Amult25_Cmult57} {iopath_Amult25_Cmult58} {iopath_Amult25_Cmult59} {iopath_Amult25_Cmult60} {iopath_Amult25_Cmult61} {iopath_Amult25_Cmult62} {iopath_Amult25_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult26_Cmult26} {iopath_Amult26_Cmult27} {iopath_Amult26_Cmult28} {iopath_Amult26_Cmult29} {iopath_Amult26_Cmult30} {iopath_Amult26_Cmult31} {iopath_Amult26_Cmult32} {iopath_Amult26_Cmult33} {iopath_Amult26_Cmult34} {iopath_Amult26_Cmult35} {iopath_Amult26_Cmult36} {iopath_Amult26_Cmult37} {iopath_Amult26_Cmult38} {iopath_Amult26_Cmult39} {iopath_Amult26_Cmult40} {iopath_Amult26_Cmult41} {iopath_Amult26_Cmult42} {iopath_Amult26_Cmult43} {iopath_Amult26_Cmult44} {iopath_Amult26_Cmult45} {iopath_Amult26_Cmult46} {iopath_Amult26_Cmult47} {iopath_Amult26_Cmult48} {iopath_Amult26_Cmult49} {iopath_Amult26_Cmult50} {iopath_Amult26_Cmult51} {iopath_Amult26_Cmult52} {iopath_Amult26_Cmult53} {iopath_Amult26_Cmult54} {iopath_Amult26_Cmult55} {iopath_Amult26_Cmult56} {iopath_Amult26_Cmult57} {iopath_Amult26_Cmult58} {iopath_Amult26_Cmult59} {iopath_Amult26_Cmult60} {iopath_Amult26_Cmult61} {iopath_Amult26_Cmult62} {iopath_Amult26_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult27_Cmult27} {iopath_Amult27_Cmult28} {iopath_Amult27_Cmult29} {iopath_Amult27_Cmult30} {iopath_Amult27_Cmult31} {iopath_Amult27_Cmult32} {iopath_Amult27_Cmult33} {iopath_Amult27_Cmult34} {iopath_Amult27_Cmult35} {iopath_Amult27_Cmult36} {iopath_Amult27_Cmult37} {iopath_Amult27_Cmult38} {iopath_Amult27_Cmult39} {iopath_Amult27_Cmult40} {iopath_Amult27_Cmult41} {iopath_Amult27_Cmult42} {iopath_Amult27_Cmult43} {iopath_Amult27_Cmult44} {iopath_Amult27_Cmult45} {iopath_Amult27_Cmult46} {iopath_Amult27_Cmult47} {iopath_Amult27_Cmult48} {iopath_Amult27_Cmult49} {iopath_Amult27_Cmult50} {iopath_Amult27_Cmult51} {iopath_Amult27_Cmult52} {iopath_Amult27_Cmult53} {iopath_Amult27_Cmult54} {iopath_Amult27_Cmult55} {iopath_Amult27_Cmult56} {iopath_Amult27_Cmult57} {iopath_Amult27_Cmult58} {iopath_Amult27_Cmult59} {iopath_Amult27_Cmult60} {iopath_Amult27_Cmult61} {iopath_Amult27_Cmult62} {iopath_Amult27_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult28_Cmult28} {iopath_Amult28_Cmult29} {iopath_Amult28_Cmult30} {iopath_Amult28_Cmult31} {iopath_Amult28_Cmult32} {iopath_Amult28_Cmult33} {iopath_Amult28_Cmult34} {iopath_Amult28_Cmult35} {iopath_Amult28_Cmult36} {iopath_Amult28_Cmult37} {iopath_Amult28_Cmult38} {iopath_Amult28_Cmult39} {iopath_Amult28_Cmult40} {iopath_Amult28_Cmult41} {iopath_Amult28_Cmult42} {iopath_Amult28_Cmult43} {iopath_Amult28_Cmult44} {iopath_Amult28_Cmult45} {iopath_Amult28_Cmult46} {iopath_Amult28_Cmult47} {iopath_Amult28_Cmult48} {iopath_Amult28_Cmult49} {iopath_Amult28_Cmult50} {iopath_Amult28_Cmult51} {iopath_Amult28_Cmult52} {iopath_Amult28_Cmult53} {iopath_Amult28_Cmult54} {iopath_Amult28_Cmult55} {iopath_Amult28_Cmult56} {iopath_Amult28_Cmult57} {iopath_Amult28_Cmult58} {iopath_Amult28_Cmult59} {iopath_Amult28_Cmult60} {iopath_Amult28_Cmult61} {iopath_Amult28_Cmult62} {iopath_Amult28_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult29_Cmult29} {iopath_Amult29_Cmult30} {iopath_Amult29_Cmult31} {iopath_Amult29_Cmult32} {iopath_Amult29_Cmult33} {iopath_Amult29_Cmult34} {iopath_Amult29_Cmult35} {iopath_Amult29_Cmult36} {iopath_Amult29_Cmult37} {iopath_Amult29_Cmult38} {iopath_Amult29_Cmult39} {iopath_Amult29_Cmult40} {iopath_Amult29_Cmult41} {iopath_Amult29_Cmult42} {iopath_Amult29_Cmult43} {iopath_Amult29_Cmult44} {iopath_Amult29_Cmult45} {iopath_Amult29_Cmult46} {iopath_Amult29_Cmult47} {iopath_Amult29_Cmult48} {iopath_Amult29_Cmult49} {iopath_Amult29_Cmult50} {iopath_Amult29_Cmult51} {iopath_Amult29_Cmult52} {iopath_Amult29_Cmult53} {iopath_Amult29_Cmult54} {iopath_Amult29_Cmult55} {iopath_Amult29_Cmult56} {iopath_Amult29_Cmult57} {iopath_Amult29_Cmult58} {iopath_Amult29_Cmult59} {iopath_Amult29_Cmult60} {iopath_Amult29_Cmult61} {iopath_Amult29_Cmult62} {iopath_Amult29_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult30_Cmult30} {iopath_Amult30_Cmult31} {iopath_Amult30_Cmult32} {iopath_Amult30_Cmult33} {iopath_Amult30_Cmult34} {iopath_Amult30_Cmult35} {iopath_Amult30_Cmult36} {iopath_Amult30_Cmult37} {iopath_Amult30_Cmult38} {iopath_Amult30_Cmult39} {iopath_Amult30_Cmult40} {iopath_Amult30_Cmult41} {iopath_Amult30_Cmult42} {iopath_Amult30_Cmult43} {iopath_Amult30_Cmult44} {iopath_Amult30_Cmult45} {iopath_Amult30_Cmult46} {iopath_Amult30_Cmult47} {iopath_Amult30_Cmult48} {iopath_Amult30_Cmult49} {iopath_Amult30_Cmult50} {iopath_Amult30_Cmult51} {iopath_Amult30_Cmult52} {iopath_Amult30_Cmult53} {iopath_Amult30_Cmult54} {iopath_Amult30_Cmult55} {iopath_Amult30_Cmult56} {iopath_Amult30_Cmult57} {iopath_Amult30_Cmult58} {iopath_Amult30_Cmult59} {iopath_Amult30_Cmult60} {iopath_Amult30_Cmult61} {iopath_Amult30_Cmult62} {iopath_Amult30_Cmult63} 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 {iopath_Amult31_Cmult31} {iopath_Amult31_Cmult32} {iopath_Amult31_Cmult33} {iopath_Amult31_Cmult34} {iopath_Amult31_Cmult35} {iopath_Amult31_Cmult36} {iopath_Amult31_Cmult37} {iopath_Amult31_Cmult38} {iopath_Amult31_Cmult39} {iopath_Amult31_Cmult40} {iopath_Amult31_Cmult41} {iopath_Amult31_Cmult42} {iopath_Amult31_Cmult43} {iopath_Amult31_Cmult44} {iopath_Amult31_Cmult45} {iopath_Amult31_Cmult46} {iopath_Amult31_Cmult47} {iopath_Amult31_Cmult48} {iopath_Amult31_Cmult49} {iopath_Amult31_Cmult50} {iopath_Amult31_Cmult51} {iopath_Amult31_Cmult52} {iopath_Amult31_Cmult53} {iopath_Amult31_Cmult54} {iopath_Amult31_Cmult55} {iopath_Amult31_Cmult56} {iopath_Amult31_Cmult57} {iopath_Amult31_Cmult58} {iopath_Amult31_Cmult59} {iopath_Amult31_Cmult60} {iopath_Amult31_Cmult61} {iopath_Amult31_Cmult62} {iopath_Amult31_Cmult63} "*)
	(* DELAY_MATRIX_Valid_mult="1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 "*)
	(* DELAY_MATRIX_sel_mul_32x32="1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 1e-10 "*)
	output reg  [63:0] Cmult;
	input wire         sel_mul_32x32;

`ifdef GSIM
    specify
		(Amult[0]  => Cmult[0]) = "";
		(Amult[1]  => Cmult[0]) = "";
		(Amult[2]  => Cmult[0]) = "";
		(Amult[3]  => Cmult[0]) = "";
		(Amult[4]  => Cmult[0]) = "";
		(Amult[5]  => Cmult[0]) = "";
		(Amult[6]  => Cmult[0]) = "";
		(Amult[7]  => Cmult[0]) = "";
		(Amult[8]  => Cmult[0]) = "";
		(Amult[9]  => Cmult[0]) = "";
		(Amult[10] => Cmult[0]) = "";
		(Amult[11] => Cmult[0]) = "";
		(Amult[12] => Cmult[0]) = "";
		(Amult[13] => Cmult[0]) = "";
		(Amult[14] => Cmult[0]) = "";
		(Amult[15] => Cmult[0]) = "";
		(Amult[16] => Cmult[0]) = "";
		(Amult[17] => Cmult[0]) = "";
		(Amult[18] => Cmult[0]) = "";
		(Amult[19] => Cmult[0]) = "";
		(Amult[20] => Cmult[0]) = "";
		(Amult[21] => Cmult[0]) = "";
		(Amult[22] => Cmult[0]) = "";
		(Amult[23] => Cmult[0]) = "";
		(Amult[24] => Cmult[0]) = "";
		(Amult[25] => Cmult[0]) = "";
		(Amult[26] => Cmult[0]) = "";
		(Amult[27] => Cmult[0]) = "";
		(Amult[28] => Cmult[0]) = "";
		(Amult[29] => Cmult[0]) = "";
		(Amult[30] => Cmult[0]) = "";
		(Amult[31] => Cmult[0]) = "";
		(Bmult[0]  => Cmult[0]) = "";
		(Bmult[1]  => Cmult[0]) = "";
		(Bmult[2]  => Cmult[0]) = "";
		(Bmult[3]  => Cmult[0]) = "";
		(Bmult[4]  => Cmult[0]) = "";
		(Bmult[5]  => Cmult[0]) = "";
		(Bmult[6]  => Cmult[0]) = "";
		(Bmult[7]  => Cmult[0]) = "";
		(Bmult[8]  => Cmult[0]) = "";
		(Bmult[9]  => Cmult[0]) = "";
		(Bmult[10] => Cmult[0]) = "";
		(Bmult[11] => Cmult[0]) = "";
		(Bmult[12] => Cmult[0]) = "";
		(Bmult[13] => Cmult[0]) = "";
		(Bmult[14] => Cmult[0]) = "";
		(Bmult[15] => Cmult[0]) = "";
		(Bmult[16] => Cmult[0]) = "";
		(Bmult[17] => Cmult[0]) = "";
		(Bmult[18] => Cmult[0]) = "";
		(Bmult[19] => Cmult[0]) = "";
		(Bmult[20] => Cmult[0]) = "";
		(Bmult[21] => Cmult[0]) = "";
		(Bmult[22] => Cmult[0]) = "";
		(Bmult[23] => Cmult[0]) = "";
		(Bmult[24] => Cmult[0]) = "";
		(Bmult[25] => Cmult[0]) = "";
		(Bmult[26] => Cmult[0]) = "";
		(Bmult[27] => Cmult[0]) = "";
		(Bmult[28] => Cmult[0]) = "";
		(Bmult[29] => Cmult[0]) = "";
		(Bmult[30] => Cmult[0]) = "";
		(Bmult[31] => Cmult[0]) = "";		
		(Valid_mult[0] => Cmult[0]) = "";
		(Valid_mult[1] => Cmult[0]) = "";
		(sel_mul_32x32 => Cmult[0]) = "";
		(Amult[0]  => Cmult[1]) = "";
		(Amult[1]  => Cmult[1]) = "";
		(Amult[2]  => Cmult[1]) = "";
		(Amult[3]  => Cmult[1]) = "";
		(Amult[4]  => Cmult[1]) = "";
		(Amult[5]  => Cmult[1]) = "";
		(Amult[6]  => Cmult[1]) = "";
		(Amult[7]  => Cmult[1]) = "";
		(Amult[8]  => Cmult[1]) = "";
		(Amult[9]  => Cmult[1]) = "";
		(Amult[10] => Cmult[1]) = "";
		(Amult[11] => Cmult[1]) = "";
		(Amult[12] => Cmult[1]) = "";
		(Amult[13] => Cmult[1]) = "";
		(Amult[14] => Cmult[1]) = "";
		(Amult[15] => Cmult[1]) = "";
		(Amult[16] => Cmult[1]) = "";
		(Amult[17] => Cmult[1]) = "";
		(Amult[18] => Cmult[1]) = "";
		(Amult[19] => Cmult[1]) = "";
		(Amult[20] => Cmult[1]) = "";
		(Amult[21] => Cmult[1]) = "";
		(Amult[22] => Cmult[1]) = "";
		(Amult[23] => Cmult[1]) = "";
		(Amult[24] => Cmult[1]) = "";
		(Amult[25] => Cmult[1]) = "";
		(Amult[26] => Cmult[1]) = "";
		(Amult[27] => Cmult[1]) = "";
		(Amult[28] => Cmult[1]) = "";
		(Amult[29] => Cmult[1]) = "";
		(Amult[30] => Cmult[1]) = "";
		(Amult[31] => Cmult[1]) = "";
		(Bmult[0]  => Cmult[1]) = "";
		(Bmult[1]  => Cmult[1]) = "";
		(Bmult[2]  => Cmult[1]) = "";
		(Bmult[3]  => Cmult[1]) = "";
		(Bmult[4]  => Cmult[1]) = "";
		(Bmult[5]  => Cmult[1]) = "";
		(Bmult[6]  => Cmult[1]) = "";
		(Bmult[7]  => Cmult[1]) = "";
		(Bmult[8]  => Cmult[1]) = "";
		(Bmult[9]  => Cmult[1]) = "";
		(Bmult[10] => Cmult[1]) = "";
		(Bmult[11] => Cmult[1]) = "";
		(Bmult[12] => Cmult[1]) = "";
		(Bmult[13] => Cmult[1]) = "";
		(Bmult[14] => Cmult[1]) = "";
		(Bmult[15] => Cmult[1]) = "";
		(Bmult[16] => Cmult[1]) = "";
		(Bmult[17] => Cmult[1]) = "";
		(Bmult[18] => Cmult[1]) = "";
		(Bmult[19] => Cmult[1]) = "";
		(Bmult[20] => Cmult[1]) = "";
		(Bmult[21] => Cmult[1]) = "";
		(Bmult[22] => Cmult[1]) = "";
		(Bmult[23] => Cmult[1]) = "";
		(Bmult[24] => Cmult[1]) = "";
		(Bmult[25] => Cmult[1]) = "";
		(Bmult[26] => Cmult[1]) = "";
		(Bmult[27] => Cmult[1]) = "";
		(Bmult[28] => Cmult[1]) = "";
		(Bmult[29] => Cmult[1]) = "";
		(Bmult[30] => Cmult[1]) = "";
		(Bmult[31] => Cmult[1]) = "";		
		(Valid_mult[0] => Cmult[1]) = "";
		(Valid_mult[1] => Cmult[1]) = "";
		(sel_mul_32x32 => Cmult[1]) = "";
		(Amult[0]  => Cmult[2]) = "";
		(Amult[1]  => Cmult[2]) = "";
		(Amult[2]  => Cmult[2]) = "";
		(Amult[3]  => Cmult[2]) = "";
		(Amult[4]  => Cmult[2]) = "";
		(Amult[5]  => Cmult[2]) = "";
		(Amult[6]  => Cmult[2]) = "";
		(Amult[7]  => Cmult[2]) = "";
		(Amult[8]  => Cmult[2]) = "";
		(Amult[9]  => Cmult[2]) = "";
		(Amult[10] => Cmult[2]) = "";
		(Amult[11] => Cmult[2]) = "";
		(Amult[12] => Cmult[2]) = "";
		(Amult[13] => Cmult[2]) = "";
		(Amult[14] => Cmult[2]) = "";
		(Amult[15] => Cmult[2]) = "";
		(Amult[16] => Cmult[2]) = "";
		(Amult[17] => Cmult[2]) = "";
		(Amult[18] => Cmult[2]) = "";
		(Amult[19] => Cmult[2]) = "";
		(Amult[20] => Cmult[2]) = "";
		(Amult[21] => Cmult[2]) = "";
		(Amult[22] => Cmult[2]) = "";
		(Amult[23] => Cmult[2]) = "";
		(Amult[24] => Cmult[2]) = "";
		(Amult[25] => Cmult[2]) = "";
		(Amult[26] => Cmult[2]) = "";
		(Amult[27] => Cmult[2]) = "";
		(Amult[28] => Cmult[2]) = "";
		(Amult[29] => Cmult[2]) = "";
		(Amult[30] => Cmult[2]) = "";
		(Amult[31] => Cmult[2]) = "";
		(Bmult[0]  => Cmult[2]) = "";
		(Bmult[1]  => Cmult[2]) = "";
		(Bmult[2]  => Cmult[2]) = "";
		(Bmult[3]  => Cmult[2]) = "";
		(Bmult[4]  => Cmult[2]) = "";
		(Bmult[5]  => Cmult[2]) = "";
		(Bmult[6]  => Cmult[2]) = "";
		(Bmult[7]  => Cmult[2]) = "";
		(Bmult[8]  => Cmult[2]) = "";
		(Bmult[9]  => Cmult[2]) = "";
		(Bmult[10] => Cmult[2]) = "";
		(Bmult[11] => Cmult[2]) = "";
		(Bmult[12] => Cmult[2]) = "";
		(Bmult[13] => Cmult[2]) = "";
		(Bmult[14] => Cmult[2]) = "";
		(Bmult[15] => Cmult[2]) = "";
		(Bmult[16] => Cmult[2]) = "";
		(Bmult[17] => Cmult[2]) = "";
		(Bmult[18] => Cmult[2]) = "";
		(Bmult[19] => Cmult[2]) = "";
		(Bmult[20] => Cmult[2]) = "";
		(Bmult[21] => Cmult[2]) = "";
		(Bmult[22] => Cmult[2]) = "";
		(Bmult[23] => Cmult[2]) = "";
		(Bmult[24] => Cmult[2]) = "";
		(Bmult[25] => Cmult[2]) = "";
		(Bmult[26] => Cmult[2]) = "";
		(Bmult[27] => Cmult[2]) = "";
		(Bmult[28] => Cmult[2]) = "";
		(Bmult[29] => Cmult[2]) = "";
		(Bmult[30] => Cmult[2]) = "";
		(Bmult[31] => Cmult[2]) = "";		
		(Valid_mult[0] => Cmult[2]) = "";
		(Valid_mult[1] => Cmult[2]) = "";
		(sel_mul_32x32 => Cmult[2]) = "";
		(Amult[0]  => Cmult[3]) = "";
		(Amult[1]  => Cmult[3]) = "";
		(Amult[2]  => Cmult[3]) = "";
		(Amult[3]  => Cmult[3]) = "";
		(Amult[4]  => Cmult[3]) = "";
		(Amult[5]  => Cmult[3]) = "";
		(Amult[6]  => Cmult[3]) = "";
		(Amult[7]  => Cmult[3]) = "";
		(Amult[8]  => Cmult[3]) = "";
		(Amult[9]  => Cmult[3]) = "";
		(Amult[10] => Cmult[3]) = "";
		(Amult[11] => Cmult[3]) = "";
		(Amult[12] => Cmult[3]) = "";
		(Amult[13] => Cmult[3]) = "";
		(Amult[14] => Cmult[3]) = "";
		(Amult[15] => Cmult[3]) = "";
		(Amult[16] => Cmult[3]) = "";
		(Amult[17] => Cmult[3]) = "";
		(Amult[18] => Cmult[3]) = "";
		(Amult[19] => Cmult[3]) = "";
		(Amult[20] => Cmult[3]) = "";
		(Amult[21] => Cmult[3]) = "";
		(Amult[22] => Cmult[3]) = "";
		(Amult[23] => Cmult[3]) = "";
		(Amult[24] => Cmult[3]) = "";
		(Amult[25] => Cmult[3]) = "";
		(Amult[26] => Cmult[3]) = "";
		(Amult[27] => Cmult[3]) = "";
		(Amult[28] => Cmult[3]) = "";
		(Amult[29] => Cmult[3]) = "";
		(Amult[30] => Cmult[3]) = "";
		(Amult[31] => Cmult[3]) = "";
		(Bmult[0]  => Cmult[3]) = "";
		(Bmult[1]  => Cmult[3]) = "";
		(Bmult[2]  => Cmult[3]) = "";
		(Bmult[3]  => Cmult[3]) = "";
		(Bmult[4]  => Cmult[3]) = "";
		(Bmult[5]  => Cmult[3]) = "";
		(Bmult[6]  => Cmult[3]) = "";
		(Bmult[7]  => Cmult[3]) = "";
		(Bmult[8]  => Cmult[3]) = "";
		(Bmult[9]  => Cmult[3]) = "";
		(Bmult[10] => Cmult[3]) = "";
		(Bmult[11] => Cmult[3]) = "";
		(Bmult[12] => Cmult[3]) = "";
		(Bmult[13] => Cmult[3]) = "";
		(Bmult[14] => Cmult[3]) = "";
		(Bmult[15] => Cmult[3]) = "";
		(Bmult[16] => Cmult[3]) = "";
		(Bmult[17] => Cmult[3]) = "";
		(Bmult[18] => Cmult[3]) = "";
		(Bmult[19] => Cmult[3]) = "";
		(Bmult[20] => Cmult[3]) = "";
		(Bmult[21] => Cmult[3]) = "";
		(Bmult[22] => Cmult[3]) = "";
		(Bmult[23] => Cmult[3]) = "";
		(Bmult[24] => Cmult[3]) = "";
		(Bmult[25] => Cmult[3]) = "";
		(Bmult[26] => Cmult[3]) = "";
		(Bmult[27] => Cmult[3]) = "";
		(Bmult[28] => Cmult[3]) = "";
		(Bmult[29] => Cmult[3]) = "";
		(Bmult[30] => Cmult[3]) = "";
		(Bmult[31] => Cmult[3]) = "";		
		(Valid_mult[0] => Cmult[3]) = "";
		(Valid_mult[1] => Cmult[3]) = "";
		(sel_mul_32x32 => Cmult[3]) = "";
		(Amult[0]  => Cmult[4]) = "";
		(Amult[1]  => Cmult[4]) = "";
		(Amult[2]  => Cmult[4]) = "";
		(Amult[3]  => Cmult[4]) = "";
		(Amult[4]  => Cmult[4]) = "";
		(Amult[5]  => Cmult[4]) = "";
		(Amult[6]  => Cmult[4]) = "";
		(Amult[7]  => Cmult[4]) = "";
		(Amult[8]  => Cmult[4]) = "";
		(Amult[9]  => Cmult[4]) = "";
		(Amult[10] => Cmult[4]) = "";
		(Amult[11] => Cmult[4]) = "";
		(Amult[12] => Cmult[4]) = "";
		(Amult[13] => Cmult[4]) = "";
		(Amult[14] => Cmult[4]) = "";
		(Amult[15] => Cmult[4]) = "";
		(Amult[16] => Cmult[4]) = "";
		(Amult[17] => Cmult[4]) = "";
		(Amult[18] => Cmult[4]) = "";
		(Amult[19] => Cmult[4]) = "";
		(Amult[20] => Cmult[4]) = "";
		(Amult[21] => Cmult[4]) = "";
		(Amult[22] => Cmult[4]) = "";
		(Amult[23] => Cmult[4]) = "";
		(Amult[24] => Cmult[4]) = "";
		(Amult[25] => Cmult[4]) = "";
		(Amult[26] => Cmult[4]) = "";
		(Amult[27] => Cmult[4]) = "";
		(Amult[28] => Cmult[4]) = "";
		(Amult[29] => Cmult[4]) = "";
		(Amult[30] => Cmult[4]) = "";
		(Amult[31] => Cmult[4]) = "";
		(Bmult[0]  => Cmult[4]) = "";
		(Bmult[1]  => Cmult[4]) = "";
		(Bmult[2]  => Cmult[4]) = "";
		(Bmult[3]  => Cmult[4]) = "";
		(Bmult[4]  => Cmult[4]) = "";
		(Bmult[5]  => Cmult[4]) = "";
		(Bmult[6]  => Cmult[4]) = "";
		(Bmult[7]  => Cmult[4]) = "";
		(Bmult[8]  => Cmult[4]) = "";
		(Bmult[9]  => Cmult[4]) = "";
		(Bmult[10] => Cmult[4]) = "";
		(Bmult[11] => Cmult[4]) = "";
		(Bmult[12] => Cmult[4]) = "";
		(Bmult[13] => Cmult[4]) = "";
		(Bmult[14] => Cmult[4]) = "";
		(Bmult[15] => Cmult[4]) = "";
		(Bmult[16] => Cmult[4]) = "";
		(Bmult[17] => Cmult[4]) = "";
		(Bmult[18] => Cmult[4]) = "";
		(Bmult[19] => Cmult[4]) = "";
		(Bmult[20] => Cmult[4]) = "";
		(Bmult[21] => Cmult[4]) = "";
		(Bmult[22] => Cmult[4]) = "";
		(Bmult[23] => Cmult[4]) = "";
		(Bmult[24] => Cmult[4]) = "";
		(Bmult[25] => Cmult[4]) = "";
		(Bmult[26] => Cmult[4]) = "";
		(Bmult[27] => Cmult[4]) = "";
		(Bmult[28] => Cmult[4]) = "";
		(Bmult[29] => Cmult[4]) = "";
		(Bmult[30] => Cmult[4]) = "";
		(Bmult[31] => Cmult[4]) = "";		
		(Valid_mult[0] => Cmult[4]) = "";
		(Valid_mult[1] => Cmult[4]) = "";
		(sel_mul_32x32 => Cmult[4]) = "";
		(Amult[0]  => Cmult[5]) = "";
		(Amult[1]  => Cmult[5]) = "";
		(Amult[2]  => Cmult[5]) = "";
		(Amult[3]  => Cmult[5]) = "";
		(Amult[4]  => Cmult[5]) = "";
		(Amult[5]  => Cmult[5]) = "";
		(Amult[6]  => Cmult[5]) = "";
		(Amult[7]  => Cmult[5]) = "";
		(Amult[8]  => Cmult[5]) = "";
		(Amult[9]  => Cmult[5]) = "";
		(Amult[10] => Cmult[5]) = "";
		(Amult[11] => Cmult[5]) = "";
		(Amult[12] => Cmult[5]) = "";
		(Amult[13] => Cmult[5]) = "";
		(Amult[14] => Cmult[5]) = "";
		(Amult[15] => Cmult[5]) = "";
		(Amult[16] => Cmult[5]) = "";
		(Amult[17] => Cmult[5]) = "";
		(Amult[18] => Cmult[5]) = "";
		(Amult[19] => Cmult[5]) = "";
		(Amult[20] => Cmult[5]) = "";
		(Amult[21] => Cmult[5]) = "";
		(Amult[22] => Cmult[5]) = "";
		(Amult[23] => Cmult[5]) = "";
		(Amult[24] => Cmult[5]) = "";
		(Amult[25] => Cmult[5]) = "";
		(Amult[26] => Cmult[5]) = "";
		(Amult[27] => Cmult[5]) = "";
		(Amult[28] => Cmult[5]) = "";
		(Amult[29] => Cmult[5]) = "";
		(Amult[30] => Cmult[5]) = "";
		(Amult[31] => Cmult[5]) = "";
		(Bmult[0]  => Cmult[5]) = "";
		(Bmult[1]  => Cmult[5]) = "";
		(Bmult[2]  => Cmult[5]) = "";
		(Bmult[3]  => Cmult[5]) = "";
		(Bmult[4]  => Cmult[5]) = "";
		(Bmult[5]  => Cmult[5]) = "";
		(Bmult[6]  => Cmult[5]) = "";
		(Bmult[7]  => Cmult[5]) = "";
		(Bmult[8]  => Cmult[5]) = "";
		(Bmult[9]  => Cmult[5]) = "";
		(Bmult[10] => Cmult[5]) = "";
		(Bmult[11] => Cmult[5]) = "";
		(Bmult[12] => Cmult[5]) = "";
		(Bmult[13] => Cmult[5]) = "";
		(Bmult[14] => Cmult[5]) = "";
		(Bmult[15] => Cmult[5]) = "";
		(Bmult[16] => Cmult[5]) = "";
		(Bmult[17] => Cmult[5]) = "";
		(Bmult[18] => Cmult[5]) = "";
		(Bmult[19] => Cmult[5]) = "";
		(Bmult[20] => Cmult[5]) = "";
		(Bmult[21] => Cmult[5]) = "";
		(Bmult[22] => Cmult[5]) = "";
		(Bmult[23] => Cmult[5]) = "";
		(Bmult[24] => Cmult[5]) = "";
		(Bmult[25] => Cmult[5]) = "";
		(Bmult[26] => Cmult[5]) = "";
		(Bmult[27] => Cmult[5]) = "";
		(Bmult[28] => Cmult[5]) = "";
		(Bmult[29] => Cmult[5]) = "";
		(Bmult[30] => Cmult[5]) = "";
		(Bmult[31] => Cmult[5]) = "";		
		(Valid_mult[0] => Cmult[5]) = "";
		(Valid_mult[1] => Cmult[5]) = "";
		(sel_mul_32x32 => Cmult[5]) = "";
		(Amult[0]  => Cmult[6]) = "";
		(Amult[1]  => Cmult[6]) = "";
		(Amult[2]  => Cmult[6]) = "";
		(Amult[3]  => Cmult[6]) = "";
		(Amult[4]  => Cmult[6]) = "";
		(Amult[5]  => Cmult[6]) = "";
		(Amult[6]  => Cmult[6]) = "";
		(Amult[7]  => Cmult[6]) = "";
		(Amult[8]  => Cmult[6]) = "";
		(Amult[9]  => Cmult[6]) = "";
		(Amult[10] => Cmult[6]) = "";
		(Amult[11] => Cmult[6]) = "";
		(Amult[12] => Cmult[6]) = "";
		(Amult[13] => Cmult[6]) = "";
		(Amult[14] => Cmult[6]) = "";
		(Amult[15] => Cmult[6]) = "";
		(Amult[16] => Cmult[6]) = "";
		(Amult[17] => Cmult[6]) = "";
		(Amult[18] => Cmult[6]) = "";
		(Amult[19] => Cmult[6]) = "";
		(Amult[20] => Cmult[6]) = "";
		(Amult[21] => Cmult[6]) = "";
		(Amult[22] => Cmult[6]) = "";
		(Amult[23] => Cmult[6]) = "";
		(Amult[24] => Cmult[6]) = "";
		(Amult[25] => Cmult[6]) = "";
		(Amult[26] => Cmult[6]) = "";
		(Amult[27] => Cmult[6]) = "";
		(Amult[28] => Cmult[6]) = "";
		(Amult[29] => Cmult[6]) = "";
		(Amult[30] => Cmult[6]) = "";
		(Amult[31] => Cmult[6]) = "";
		(Bmult[0]  => Cmult[6]) = "";
		(Bmult[1]  => Cmult[6]) = "";
		(Bmult[2]  => Cmult[6]) = "";
		(Bmult[3]  => Cmult[6]) = "";
		(Bmult[4]  => Cmult[6]) = "";
		(Bmult[5]  => Cmult[6]) = "";
		(Bmult[6]  => Cmult[6]) = "";
		(Bmult[7]  => Cmult[6]) = "";
		(Bmult[8]  => Cmult[6]) = "";
		(Bmult[9]  => Cmult[6]) = "";
		(Bmult[10] => Cmult[6]) = "";
		(Bmult[11] => Cmult[6]) = "";
		(Bmult[12] => Cmult[6]) = "";
		(Bmult[13] => Cmult[6]) = "";
		(Bmult[14] => Cmult[6]) = "";
		(Bmult[15] => Cmult[6]) = "";
		(Bmult[16] => Cmult[6]) = "";
		(Bmult[17] => Cmult[6]) = "";
		(Bmult[18] => Cmult[6]) = "";
		(Bmult[19] => Cmult[6]) = "";
		(Bmult[20] => Cmult[6]) = "";
		(Bmult[21] => Cmult[6]) = "";
		(Bmult[22] => Cmult[6]) = "";
		(Bmult[23] => Cmult[6]) = "";
		(Bmult[24] => Cmult[6]) = "";
		(Bmult[25] => Cmult[6]) = "";
		(Bmult[26] => Cmult[6]) = "";
		(Bmult[27] => Cmult[6]) = "";
		(Bmult[28] => Cmult[6]) = "";
		(Bmult[29] => Cmult[6]) = "";
		(Bmult[30] => Cmult[6]) = "";
		(Bmult[31] => Cmult[6]) = "";		
		(Valid_mult[0] => Cmult[6]) = "";
		(Valid_mult[1] => Cmult[6]) = "";
		(sel_mul_32x32 => Cmult[6]) = "";
		(Amult[0]  => Cmult[7]) = "";
		(Amult[1]  => Cmult[7]) = "";
		(Amult[2]  => Cmult[7]) = "";
		(Amult[3]  => Cmult[7]) = "";
		(Amult[4]  => Cmult[7]) = "";
		(Amult[5]  => Cmult[7]) = "";
		(Amult[6]  => Cmult[7]) = "";
		(Amult[7]  => Cmult[7]) = "";
		(Amult[8]  => Cmult[7]) = "";
		(Amult[9]  => Cmult[7]) = "";
		(Amult[10] => Cmult[7]) = "";
		(Amult[11] => Cmult[7]) = "";
		(Amult[12] => Cmult[7]) = "";
		(Amult[13] => Cmult[7]) = "";
		(Amult[14] => Cmult[7]) = "";
		(Amult[15] => Cmult[7]) = "";
		(Amult[16] => Cmult[7]) = "";
		(Amult[17] => Cmult[7]) = "";
		(Amult[18] => Cmult[7]) = "";
		(Amult[19] => Cmult[7]) = "";
		(Amult[20] => Cmult[7]) = "";
		(Amult[21] => Cmult[7]) = "";
		(Amult[22] => Cmult[7]) = "";
		(Amult[23] => Cmult[7]) = "";
		(Amult[24] => Cmult[7]) = "";
		(Amult[25] => Cmult[7]) = "";
		(Amult[26] => Cmult[7]) = "";
		(Amult[27] => Cmult[7]) = "";
		(Amult[28] => Cmult[7]) = "";
		(Amult[29] => Cmult[7]) = "";
		(Amult[30] => Cmult[7]) = "";
		(Amult[31] => Cmult[7]) = "";
		(Bmult[0]  => Cmult[7]) = "";
		(Bmult[1]  => Cmult[7]) = "";
		(Bmult[2]  => Cmult[7]) = "";
		(Bmult[3]  => Cmult[7]) = "";
		(Bmult[4]  => Cmult[7]) = "";
		(Bmult[5]  => Cmult[7]) = "";
		(Bmult[6]  => Cmult[7]) = "";
		(Bmult[7]  => Cmult[7]) = "";
		(Bmult[8]  => Cmult[7]) = "";
		(Bmult[9]  => Cmult[7]) = "";
		(Bmult[10] => Cmult[7]) = "";
		(Bmult[11] => Cmult[7]) = "";
		(Bmult[12] => Cmult[7]) = "";
		(Bmult[13] => Cmult[7]) = "";
		(Bmult[14] => Cmult[7]) = "";
		(Bmult[15] => Cmult[7]) = "";
		(Bmult[16] => Cmult[7]) = "";
		(Bmult[17] => Cmult[7]) = "";
		(Bmult[18] => Cmult[7]) = "";
		(Bmult[19] => Cmult[7]) = "";
		(Bmult[20] => Cmult[7]) = "";
		(Bmult[21] => Cmult[7]) = "";
		(Bmult[22] => Cmult[7]) = "";
		(Bmult[23] => Cmult[7]) = "";
		(Bmult[24] => Cmult[7]) = "";
		(Bmult[25] => Cmult[7]) = "";
		(Bmult[26] => Cmult[7]) = "";
		(Bmult[27] => Cmult[7]) = "";
		(Bmult[28] => Cmult[7]) = "";
		(Bmult[29] => Cmult[7]) = "";
		(Bmult[30] => Cmult[7]) = "";
		(Bmult[31] => Cmult[7]) = "";		
		(Valid_mult[0] => Cmult[7]) = "";
		(Valid_mult[1] => Cmult[7]) = "";
		(sel_mul_32x32 => Cmult[7]) = "";
		(Amult[0]  => Cmult[8]) = "";
		(Amult[1]  => Cmult[8]) = "";
		(Amult[2]  => Cmult[8]) = "";
		(Amult[3]  => Cmult[8]) = "";
		(Amult[4]  => Cmult[8]) = "";
		(Amult[5]  => Cmult[8]) = "";
		(Amult[6]  => Cmult[8]) = "";
		(Amult[7]  => Cmult[8]) = "";
		(Amult[8]  => Cmult[8]) = "";
		(Amult[9]  => Cmult[8]) = "";
		(Amult[10] => Cmult[8]) = "";
		(Amult[11] => Cmult[8]) = "";
		(Amult[12] => Cmult[8]) = "";
		(Amult[13] => Cmult[8]) = "";
		(Amult[14] => Cmult[8]) = "";
		(Amult[15] => Cmult[8]) = "";
		(Amult[16] => Cmult[8]) = "";
		(Amult[17] => Cmult[8]) = "";
		(Amult[18] => Cmult[8]) = "";
		(Amult[19] => Cmult[8]) = "";
		(Amult[20] => Cmult[8]) = "";
		(Amult[21] => Cmult[8]) = "";
		(Amult[22] => Cmult[8]) = "";
		(Amult[23] => Cmult[8]) = "";
		(Amult[24] => Cmult[8]) = "";
		(Amult[25] => Cmult[8]) = "";
		(Amult[26] => Cmult[8]) = "";
		(Amult[27] => Cmult[8]) = "";
		(Amult[28] => Cmult[8]) = "";
		(Amult[29] => Cmult[8]) = "";
		(Amult[30] => Cmult[8]) = "";
		(Amult[31] => Cmult[8]) = "";
		(Bmult[0]  => Cmult[8]) = "";
		(Bmult[1]  => Cmult[8]) = "";
		(Bmult[2]  => Cmult[8]) = "";
		(Bmult[3]  => Cmult[8]) = "";
		(Bmult[4]  => Cmult[8]) = "";
		(Bmult[5]  => Cmult[8]) = "";
		(Bmult[6]  => Cmult[8]) = "";
		(Bmult[7]  => Cmult[8]) = "";
		(Bmult[8]  => Cmult[8]) = "";
		(Bmult[9]  => Cmult[8]) = "";
		(Bmult[10] => Cmult[8]) = "";
		(Bmult[11] => Cmult[8]) = "";
		(Bmult[12] => Cmult[8]) = "";
		(Bmult[13] => Cmult[8]) = "";
		(Bmult[14] => Cmult[8]) = "";
		(Bmult[15] => Cmult[8]) = "";
		(Bmult[16] => Cmult[8]) = "";
		(Bmult[17] => Cmult[8]) = "";
		(Bmult[18] => Cmult[8]) = "";
		(Bmult[19] => Cmult[8]) = "";
		(Bmult[20] => Cmult[8]) = "";
		(Bmult[21] => Cmult[8]) = "";
		(Bmult[22] => Cmult[8]) = "";
		(Bmult[23] => Cmult[8]) = "";
		(Bmult[24] => Cmult[8]) = "";
		(Bmult[25] => Cmult[8]) = "";
		(Bmult[26] => Cmult[8]) = "";
		(Bmult[27] => Cmult[8]) = "";
		(Bmult[28] => Cmult[8]) = "";
		(Bmult[29] => Cmult[8]) = "";
		(Bmult[30] => Cmult[8]) = "";
		(Bmult[31] => Cmult[8]) = "";		
		(Valid_mult[0] => Cmult[8]) = "";
		(Valid_mult[1] => Cmult[8]) = "";
		(sel_mul_32x32 => Cmult[8]) = "";	
		(Amult[0]  => Cmult[9]) = "";
		(Amult[1]  => Cmult[9]) = "";
		(Amult[2]  => Cmult[9]) = "";
		(Amult[3]  => Cmult[9]) = "";
		(Amult[4]  => Cmult[9]) = "";
		(Amult[5]  => Cmult[9]) = "";
		(Amult[6]  => Cmult[9]) = "";
		(Amult[7]  => Cmult[9]) = "";
		(Amult[8]  => Cmult[9]) = "";
		(Amult[9]  => Cmult[9]) = "";
		(Amult[10] => Cmult[9]) = "";
		(Amult[11] => Cmult[9]) = "";
		(Amult[12] => Cmult[9]) = "";
		(Amult[13] => Cmult[9]) = "";
		(Amult[14] => Cmult[9]) = "";
		(Amult[15] => Cmult[9]) = "";
		(Amult[16] => Cmult[9]) = "";
		(Amult[17] => Cmult[9]) = "";
		(Amult[18] => Cmult[9]) = "";
		(Amult[19] => Cmult[9]) = "";
		(Amult[20] => Cmult[9]) = "";
		(Amult[21] => Cmult[9]) = "";
		(Amult[22] => Cmult[9]) = "";
		(Amult[23] => Cmult[9]) = "";
		(Amult[24] => Cmult[9]) = "";
		(Amult[25] => Cmult[9]) = "";
		(Amult[26] => Cmult[9]) = "";
		(Amult[27] => Cmult[9]) = "";
		(Amult[28] => Cmult[9]) = "";
		(Amult[29] => Cmult[9]) = "";
		(Amult[30] => Cmult[9]) = "";
		(Amult[31] => Cmult[9]) = "";
		(Bmult[0]  => Cmult[9]) = "";
		(Bmult[1]  => Cmult[9]) = "";
		(Bmult[2]  => Cmult[9]) = "";
		(Bmult[3]  => Cmult[9]) = "";
		(Bmult[4]  => Cmult[9]) = "";
		(Bmult[5]  => Cmult[9]) = "";
		(Bmult[6]  => Cmult[9]) = "";
		(Bmult[7]  => Cmult[9]) = "";
		(Bmult[8]  => Cmult[9]) = "";
		(Bmult[9]  => Cmult[9]) = "";
		(Bmult[10] => Cmult[9]) = "";
		(Bmult[11] => Cmult[9]) = "";
		(Bmult[12] => Cmult[9]) = "";
		(Bmult[13] => Cmult[9]) = "";
		(Bmult[14] => Cmult[9]) = "";
		(Bmult[15] => Cmult[9]) = "";
		(Bmult[16] => Cmult[9]) = "";
		(Bmult[17] => Cmult[9]) = "";
		(Bmult[18] => Cmult[9]) = "";
		(Bmult[19] => Cmult[9]) = "";
		(Bmult[20] => Cmult[9]) = "";
		(Bmult[21] => Cmult[9]) = "";
		(Bmult[22] => Cmult[9]) = "";
		(Bmult[23] => Cmult[9]) = "";
		(Bmult[24] => Cmult[9]) = "";
		(Bmult[25] => Cmult[9]) = "";
		(Bmult[26] => Cmult[9]) = "";
		(Bmult[27] => Cmult[9]) = "";
		(Bmult[28] => Cmult[9]) = "";
		(Bmult[29] => Cmult[9]) = "";
		(Bmult[30] => Cmult[9]) = "";
		(Bmult[31] => Cmult[9]) = "";		
		(Valid_mult[0] => Cmult[9]) = "";
		(Valid_mult[1] => Cmult[9]) = "";
		(sel_mul_32x32 => Cmult[9]) = "";	
		(Amult[0]  => Cmult[10]) = "";
		(Amult[1]  => Cmult[10]) = "";
		(Amult[2]  => Cmult[10]) = "";
		(Amult[3]  => Cmult[10]) = "";
		(Amult[4]  => Cmult[10]) = "";
		(Amult[5]  => Cmult[10]) = "";
		(Amult[6]  => Cmult[10]) = "";
		(Amult[7]  => Cmult[10]) = "";
		(Amult[8]  => Cmult[10]) = "";
		(Amult[9]  => Cmult[10]) = "";
		(Amult[10] => Cmult[10]) = "";
		(Amult[11] => Cmult[10]) = "";
		(Amult[12] => Cmult[10]) = "";
		(Amult[13] => Cmult[10]) = "";
		(Amult[14] => Cmult[10]) = "";
		(Amult[15] => Cmult[10]) = "";
		(Amult[16] => Cmult[10]) = "";
		(Amult[17] => Cmult[10]) = "";
		(Amult[18] => Cmult[10]) = "";
		(Amult[19] => Cmult[10]) = "";
		(Amult[20] => Cmult[10]) = "";
		(Amult[21] => Cmult[10]) = "";
		(Amult[22] => Cmult[10]) = "";
		(Amult[23] => Cmult[10]) = "";
		(Amult[24] => Cmult[10]) = "";
		(Amult[25] => Cmult[10]) = "";
		(Amult[26] => Cmult[10]) = "";
		(Amult[27] => Cmult[10]) = "";
		(Amult[28] => Cmult[10]) = "";
		(Amult[29] => Cmult[10]) = "";
		(Amult[30] => Cmult[10]) = "";
		(Amult[31] => Cmult[10]) = "";
		(Bmult[0]  => Cmult[10]) = "";
		(Bmult[1]  => Cmult[10]) = "";
		(Bmult[2]  => Cmult[10]) = "";
		(Bmult[3]  => Cmult[10]) = "";
		(Bmult[4]  => Cmult[10]) = "";
		(Bmult[5]  => Cmult[10]) = "";
		(Bmult[6]  => Cmult[10]) = "";
		(Bmult[7]  => Cmult[10]) = "";
		(Bmult[8]  => Cmult[10]) = "";
		(Bmult[9]  => Cmult[10]) = "";
		(Bmult[10] => Cmult[10]) = "";
		(Bmult[11] => Cmult[10]) = "";
		(Bmult[12] => Cmult[10]) = "";
		(Bmult[13] => Cmult[10]) = "";
		(Bmult[14] => Cmult[10]) = "";
		(Bmult[15] => Cmult[10]) = "";
		(Bmult[16] => Cmult[10]) = "";
		(Bmult[17] => Cmult[10]) = "";
		(Bmult[18] => Cmult[10]) = "";
		(Bmult[19] => Cmult[10]) = "";
		(Bmult[20] => Cmult[10]) = "";
		(Bmult[21] => Cmult[10]) = "";
		(Bmult[22] => Cmult[10]) = "";
		(Bmult[23] => Cmult[10]) = "";
		(Bmult[24] => Cmult[10]) = "";
		(Bmult[25] => Cmult[10]) = "";
		(Bmult[26] => Cmult[10]) = "";
		(Bmult[27] => Cmult[10]) = "";
		(Bmult[28] => Cmult[10]) = "";
		(Bmult[29] => Cmult[10]) = "";
		(Bmult[30] => Cmult[10]) = "";
		(Bmult[31] => Cmult[10]) = "";		
		(Valid_mult[0] => Cmult[10]) = "";
		(Valid_mult[1] => Cmult[10]) = "";
		(sel_mul_32x32 => Cmult[10]) = "";
		(Amult[0]  => Cmult[11]) = "";
		(Amult[1]  => Cmult[11]) = "";
		(Amult[2]  => Cmult[11]) = "";
		(Amult[3]  => Cmult[11]) = "";
		(Amult[4]  => Cmult[11]) = "";
		(Amult[5]  => Cmult[11]) = "";
		(Amult[6]  => Cmult[11]) = "";
		(Amult[7]  => Cmult[11]) = "";
		(Amult[8]  => Cmult[11]) = "";
		(Amult[9]  => Cmult[11]) = "";
		(Amult[10] => Cmult[11]) = "";
		(Amult[11] => Cmult[11]) = "";
		(Amult[12] => Cmult[11]) = "";
		(Amult[13] => Cmult[11]) = "";
		(Amult[14] => Cmult[11]) = "";
		(Amult[15] => Cmult[11]) = "";
		(Amult[16] => Cmult[11]) = "";
		(Amult[17] => Cmult[11]) = "";
		(Amult[18] => Cmult[11]) = "";
		(Amult[19] => Cmult[11]) = "";
		(Amult[20] => Cmult[11]) = "";
		(Amult[21] => Cmult[11]) = "";
		(Amult[22] => Cmult[11]) = "";
		(Amult[23] => Cmult[11]) = "";
		(Amult[24] => Cmult[11]) = "";
		(Amult[25] => Cmult[11]) = "";
		(Amult[26] => Cmult[11]) = "";
		(Amult[27] => Cmult[11]) = "";
		(Amult[28] => Cmult[11]) = "";
		(Amult[29] => Cmult[11]) = "";
		(Amult[30] => Cmult[11]) = "";
		(Amult[31] => Cmult[11]) = "";
		(Bmult[0]  => Cmult[11]) = "";
		(Bmult[1]  => Cmult[11]) = "";
		(Bmult[2]  => Cmult[11]) = "";
		(Bmult[3]  => Cmult[11]) = "";
		(Bmult[4]  => Cmult[11]) = "";
		(Bmult[5]  => Cmult[11]) = "";
		(Bmult[6]  => Cmult[11]) = "";
		(Bmult[7]  => Cmult[11]) = "";
		(Bmult[8]  => Cmult[11]) = "";
		(Bmult[9]  => Cmult[11]) = "";
		(Bmult[10] => Cmult[11]) = "";
		(Bmult[11] => Cmult[11]) = "";
		(Bmult[12] => Cmult[11]) = "";
		(Bmult[13] => Cmult[11]) = "";
		(Bmult[14] => Cmult[11]) = "";
		(Bmult[15] => Cmult[11]) = "";
		(Bmult[16] => Cmult[11]) = "";
		(Bmult[17] => Cmult[11]) = "";
		(Bmult[18] => Cmult[11]) = "";
		(Bmult[19] => Cmult[11]) = "";
		(Bmult[20] => Cmult[11]) = "";
		(Bmult[21] => Cmult[11]) = "";
		(Bmult[22] => Cmult[11]) = "";
		(Bmult[23] => Cmult[11]) = "";
		(Bmult[24] => Cmult[11]) = "";
		(Bmult[25] => Cmult[11]) = "";
		(Bmult[26] => Cmult[11]) = "";
		(Bmult[27] => Cmult[11]) = "";
		(Bmult[28] => Cmult[11]) = "";
		(Bmult[29] => Cmult[11]) = "";
		(Bmult[30] => Cmult[11]) = "";
		(Bmult[31] => Cmult[11]) = "";		
		(Valid_mult[0] => Cmult[11]) = "";
		(Valid_mult[1] => Cmult[11]) = "";
		(sel_mul_32x32 => Cmult[11]) = "";
		(Amult[0]  => Cmult[12]) = "";
		(Amult[1]  => Cmult[12]) = "";
		(Amult[2]  => Cmult[12]) = "";
		(Amult[3]  => Cmult[12]) = "";
		(Amult[4]  => Cmult[12]) = "";
		(Amult[5]  => Cmult[12]) = "";
		(Amult[6]  => Cmult[12]) = "";
		(Amult[7]  => Cmult[12]) = "";
		(Amult[8]  => Cmult[12]) = "";
		(Amult[9]  => Cmult[12]) = "";
		(Amult[10] => Cmult[12]) = "";
		(Amult[11] => Cmult[12]) = "";
		(Amult[12] => Cmult[12]) = "";
		(Amult[13] => Cmult[12]) = "";
		(Amult[14] => Cmult[12]) = "";
		(Amult[15] => Cmult[12]) = "";
		(Amult[16] => Cmult[12]) = "";
		(Amult[17] => Cmult[12]) = "";
		(Amult[18] => Cmult[12]) = "";
		(Amult[19] => Cmult[12]) = "";
		(Amult[20] => Cmult[12]) = "";
		(Amult[21] => Cmult[12]) = "";
		(Amult[22] => Cmult[12]) = "";
		(Amult[23] => Cmult[12]) = "";
		(Amult[24] => Cmult[12]) = "";
		(Amult[25] => Cmult[12]) = "";
		(Amult[26] => Cmult[12]) = "";
		(Amult[27] => Cmult[12]) = "";
		(Amult[28] => Cmult[12]) = "";
		(Amult[29] => Cmult[12]) = "";
		(Amult[30] => Cmult[12]) = "";
		(Amult[31] => Cmult[12]) = "";
		(Bmult[0]  => Cmult[12]) = "";
		(Bmult[1]  => Cmult[12]) = "";
		(Bmult[2]  => Cmult[12]) = "";
		(Bmult[3]  => Cmult[12]) = "";
		(Bmult[4]  => Cmult[12]) = "";
		(Bmult[5]  => Cmult[12]) = "";
		(Bmult[6]  => Cmult[12]) = "";
		(Bmult[7]  => Cmult[12]) = "";
		(Bmult[8]  => Cmult[12]) = "";
		(Bmult[9]  => Cmult[12]) = "";
		(Bmult[10] => Cmult[12]) = "";
		(Bmult[11] => Cmult[12]) = "";
		(Bmult[12] => Cmult[12]) = "";
		(Bmult[13] => Cmult[12]) = "";
		(Bmult[14] => Cmult[12]) = "";
		(Bmult[15] => Cmult[12]) = "";
		(Bmult[16] => Cmult[12]) = "";
		(Bmult[17] => Cmult[12]) = "";
		(Bmult[18] => Cmult[12]) = "";
		(Bmult[19] => Cmult[12]) = "";
		(Bmult[20] => Cmult[12]) = "";
		(Bmult[21] => Cmult[12]) = "";
		(Bmult[22] => Cmult[12]) = "";
		(Bmult[23] => Cmult[12]) = "";
		(Bmult[24] => Cmult[12]) = "";
		(Bmult[25] => Cmult[12]) = "";
		(Bmult[26] => Cmult[12]) = "";
		(Bmult[27] => Cmult[12]) = "";
		(Bmult[28] => Cmult[12]) = "";
		(Bmult[29] => Cmult[12]) = "";
		(Bmult[30] => Cmult[12]) = "";
		(Bmult[31] => Cmult[12]) = "";		
		(Valid_mult[0] => Cmult[12]) = "";
		(Valid_mult[1] => Cmult[12]) = "";
		(sel_mul_32x32 => Cmult[12]) = "";
		(Amult[0]  => Cmult[13]) = "";
		(Amult[1]  => Cmult[13]) = "";
		(Amult[2]  => Cmult[13]) = "";
		(Amult[3]  => Cmult[13]) = "";
		(Amult[4]  => Cmult[13]) = "";
		(Amult[5]  => Cmult[13]) = "";
		(Amult[6]  => Cmult[13]) = "";
		(Amult[7]  => Cmult[13]) = "";
		(Amult[8]  => Cmult[13]) = "";
		(Amult[9]  => Cmult[13]) = "";
		(Amult[10] => Cmult[13]) = "";
		(Amult[11] => Cmult[13]) = "";
		(Amult[12] => Cmult[13]) = "";
		(Amult[13] => Cmult[13]) = "";
		(Amult[14] => Cmult[13]) = "";
		(Amult[15] => Cmult[13]) = "";
		(Amult[16] => Cmult[13]) = "";
		(Amult[17] => Cmult[13]) = "";
		(Amult[18] => Cmult[13]) = "";
		(Amult[19] => Cmult[13]) = "";
		(Amult[20] => Cmult[13]) = "";
		(Amult[21] => Cmult[13]) = "";
		(Amult[22] => Cmult[13]) = "";
		(Amult[23] => Cmult[13]) = "";
		(Amult[24] => Cmult[13]) = "";
		(Amult[25] => Cmult[13]) = "";
		(Amult[26] => Cmult[13]) = "";
		(Amult[27] => Cmult[13]) = "";
		(Amult[28] => Cmult[13]) = "";
		(Amult[29] => Cmult[13]) = "";
		(Amult[30] => Cmult[13]) = "";
		(Amult[31] => Cmult[13]) = "";
		(Bmult[0]  => Cmult[13]) = "";
		(Bmult[1]  => Cmult[13]) = "";
		(Bmult[2]  => Cmult[13]) = "";
		(Bmult[3]  => Cmult[13]) = "";
		(Bmult[4]  => Cmult[13]) = "";
		(Bmult[5]  => Cmult[13]) = "";
		(Bmult[6]  => Cmult[13]) = "";
		(Bmult[7]  => Cmult[13]) = "";
		(Bmult[8]  => Cmult[13]) = "";
		(Bmult[9]  => Cmult[13]) = "";
		(Bmult[10] => Cmult[13]) = "";
		(Bmult[11] => Cmult[13]) = "";
		(Bmult[12] => Cmult[13]) = "";
		(Bmult[13] => Cmult[13]) = "";
		(Bmult[14] => Cmult[13]) = "";
		(Bmult[15] => Cmult[13]) = "";
		(Bmult[16] => Cmult[13]) = "";
		(Bmult[17] => Cmult[13]) = "";
		(Bmult[18] => Cmult[13]) = "";
		(Bmult[19] => Cmult[13]) = "";
		(Bmult[20] => Cmult[13]) = "";
		(Bmult[21] => Cmult[13]) = "";
		(Bmult[22] => Cmult[13]) = "";
		(Bmult[23] => Cmult[13]) = "";
		(Bmult[24] => Cmult[13]) = "";
		(Bmult[25] => Cmult[13]) = "";
		(Bmult[26] => Cmult[13]) = "";
		(Bmult[27] => Cmult[13]) = "";
		(Bmult[28] => Cmult[13]) = "";
		(Bmult[29] => Cmult[13]) = "";
		(Bmult[30] => Cmult[13]) = "";
		(Bmult[31] => Cmult[13]) = "";		
		(Valid_mult[0] => Cmult[13]) = "";
		(Valid_mult[1] => Cmult[13]) = "";
		(sel_mul_32x32 => Cmult[13]) = "";
		(Amult[0]  => Cmult[14]) = "";
		(Amult[1]  => Cmult[14]) = "";
		(Amult[2]  => Cmult[14]) = "";
		(Amult[3]  => Cmult[14]) = "";
		(Amult[4]  => Cmult[14]) = "";
		(Amult[5]  => Cmult[14]) = "";
		(Amult[6]  => Cmult[14]) = "";
		(Amult[7]  => Cmult[14]) = "";
		(Amult[8]  => Cmult[14]) = "";
		(Amult[9]  => Cmult[14]) = "";
		(Amult[10] => Cmult[14]) = "";
		(Amult[11] => Cmult[14]) = "";
		(Amult[12] => Cmult[14]) = "";
		(Amult[13] => Cmult[14]) = "";
		(Amult[14] => Cmult[14]) = "";
		(Amult[15] => Cmult[14]) = "";
		(Amult[16] => Cmult[14]) = "";
		(Amult[17] => Cmult[14]) = "";
		(Amult[18] => Cmult[14]) = "";
		(Amult[19] => Cmult[14]) = "";
		(Amult[20] => Cmult[14]) = "";
		(Amult[21] => Cmult[14]) = "";
		(Amult[22] => Cmult[14]) = "";
		(Amult[23] => Cmult[14]) = "";
		(Amult[24] => Cmult[14]) = "";
		(Amult[25] => Cmult[14]) = "";
		(Amult[26] => Cmult[14]) = "";
		(Amult[27] => Cmult[14]) = "";
		(Amult[28] => Cmult[14]) = "";
		(Amult[29] => Cmult[14]) = "";
		(Amult[30] => Cmult[14]) = "";
		(Amult[31] => Cmult[14]) = "";
		(Bmult[0]  => Cmult[14]) = "";
		(Bmult[1]  => Cmult[14]) = "";
		(Bmult[2]  => Cmult[14]) = "";
		(Bmult[3]  => Cmult[14]) = "";
		(Bmult[4]  => Cmult[14]) = "";
		(Bmult[5]  => Cmult[14]) = "";
		(Bmult[6]  => Cmult[14]) = "";
		(Bmult[7]  => Cmult[14]) = "";
		(Bmult[8]  => Cmult[14]) = "";
		(Bmult[9]  => Cmult[14]) = "";
		(Bmult[10] => Cmult[14]) = "";
		(Bmult[11] => Cmult[14]) = "";
		(Bmult[12] => Cmult[14]) = "";
		(Bmult[13] => Cmult[14]) = "";
		(Bmult[14] => Cmult[14]) = "";
		(Bmult[15] => Cmult[14]) = "";
		(Bmult[16] => Cmult[14]) = "";
		(Bmult[17] => Cmult[14]) = "";
		(Bmult[18] => Cmult[14]) = "";
		(Bmult[19] => Cmult[14]) = "";
		(Bmult[20] => Cmult[14]) = "";
		(Bmult[21] => Cmult[14]) = "";
		(Bmult[22] => Cmult[14]) = "";
		(Bmult[23] => Cmult[14]) = "";
		(Bmult[24] => Cmult[14]) = "";
		(Bmult[25] => Cmult[14]) = "";
		(Bmult[26] => Cmult[14]) = "";
		(Bmult[27] => Cmult[14]) = "";
		(Bmult[28] => Cmult[14]) = "";
		(Bmult[29] => Cmult[14]) = "";
		(Bmult[30] => Cmult[14]) = "";
		(Bmult[31] => Cmult[14]) = "";		
		(Valid_mult[0] => Cmult[14]) = "";
		(Valid_mult[1] => Cmult[14]) = "";
		(sel_mul_32x32 => Cmult[14]) = "";
		(Amult[0]  => Cmult[15]) = "";
		(Amult[1]  => Cmult[15]) = "";
		(Amult[2]  => Cmult[15]) = "";
		(Amult[3]  => Cmult[15]) = "";
		(Amult[4]  => Cmult[15]) = "";
		(Amult[5]  => Cmult[15]) = "";
		(Amult[6]  => Cmult[15]) = "";
		(Amult[7]  => Cmult[15]) = "";
		(Amult[8]  => Cmult[15]) = "";
		(Amult[9]  => Cmult[15]) = "";
		(Amult[10] => Cmult[15]) = "";
		(Amult[11] => Cmult[15]) = "";
		(Amult[12] => Cmult[15]) = "";
		(Amult[13] => Cmult[15]) = "";
		(Amult[14] => Cmult[15]) = "";
		(Amult[15] => Cmult[15]) = "";
		(Amult[16] => Cmult[15]) = "";
		(Amult[17] => Cmult[15]) = "";
		(Amult[18] => Cmult[15]) = "";
		(Amult[19] => Cmult[15]) = "";
		(Amult[20] => Cmult[15]) = "";
		(Amult[21] => Cmult[15]) = "";
		(Amult[22] => Cmult[15]) = "";
		(Amult[23] => Cmult[15]) = "";
		(Amult[24] => Cmult[15]) = "";
		(Amult[25] => Cmult[15]) = "";
		(Amult[26] => Cmult[15]) = "";
		(Amult[27] => Cmult[15]) = "";
		(Amult[28] => Cmult[15]) = "";
		(Amult[29] => Cmult[15]) = "";
		(Amult[30] => Cmult[15]) = "";
		(Amult[31] => Cmult[15]) = "";
		(Bmult[0]  => Cmult[15]) = "";
		(Bmult[1]  => Cmult[15]) = "";
		(Bmult[2]  => Cmult[15]) = "";
		(Bmult[3]  => Cmult[15]) = "";
		(Bmult[4]  => Cmult[15]) = "";
		(Bmult[5]  => Cmult[15]) = "";
		(Bmult[6]  => Cmult[15]) = "";
		(Bmult[7]  => Cmult[15]) = "";
		(Bmult[8]  => Cmult[15]) = "";
		(Bmult[9]  => Cmult[15]) = "";
		(Bmult[10] => Cmult[15]) = "";
		(Bmult[11] => Cmult[15]) = "";
		(Bmult[12] => Cmult[15]) = "";
		(Bmult[13] => Cmult[15]) = "";
		(Bmult[14] => Cmult[15]) = "";
		(Bmult[15] => Cmult[15]) = "";
		(Bmult[16] => Cmult[15]) = "";
		(Bmult[17] => Cmult[15]) = "";
		(Bmult[18] => Cmult[15]) = "";
		(Bmult[19] => Cmult[15]) = "";
		(Bmult[20] => Cmult[15]) = "";
		(Bmult[21] => Cmult[15]) = "";
		(Bmult[22] => Cmult[15]) = "";
		(Bmult[23] => Cmult[15]) = "";
		(Bmult[24] => Cmult[15]) = "";
		(Bmult[25] => Cmult[15]) = "";
		(Bmult[26] => Cmult[15]) = "";
		(Bmult[27] => Cmult[15]) = "";
		(Bmult[28] => Cmult[15]) = "";
		(Bmult[29] => Cmult[15]) = "";
		(Bmult[30] => Cmult[15]) = "";
		(Bmult[31] => Cmult[15]) = "";		
		(Valid_mult[0] => Cmult[15]) = "";
		(Valid_mult[1] => Cmult[15]) = "";
		(sel_mul_32x32 => Cmult[15]) = "";
		(Amult[0]  => Cmult[16]) = "";
		(Amult[1]  => Cmult[16]) = "";
		(Amult[2]  => Cmult[16]) = "";
		(Amult[3]  => Cmult[16]) = "";
		(Amult[4]  => Cmult[16]) = "";
		(Amult[5]  => Cmult[16]) = "";
		(Amult[6]  => Cmult[16]) = "";
		(Amult[7]  => Cmult[16]) = "";
		(Amult[8]  => Cmult[16]) = "";
		(Amult[9]  => Cmult[16]) = "";
		(Amult[10] => Cmult[16]) = "";
		(Amult[11] => Cmult[16]) = "";
		(Amult[12] => Cmult[16]) = "";
		(Amult[13] => Cmult[16]) = "";
		(Amult[14] => Cmult[16]) = "";
		(Amult[15] => Cmult[16]) = "";
		(Amult[16] => Cmult[16]) = "";
		(Amult[17] => Cmult[16]) = "";
		(Amult[18] => Cmult[16]) = "";
		(Amult[19] => Cmult[16]) = "";
		(Amult[20] => Cmult[16]) = "";
		(Amult[21] => Cmult[16]) = "";
		(Amult[22] => Cmult[16]) = "";
		(Amult[23] => Cmult[16]) = "";
		(Amult[24] => Cmult[16]) = "";
		(Amult[25] => Cmult[16]) = "";
		(Amult[26] => Cmult[16]) = "";
		(Amult[27] => Cmult[16]) = "";
		(Amult[28] => Cmult[16]) = "";
		(Amult[29] => Cmult[16]) = "";
		(Amult[30] => Cmult[16]) = "";
		(Amult[31] => Cmult[16]) = "";
		(Bmult[0]  => Cmult[16]) = "";
		(Bmult[1]  => Cmult[16]) = "";
		(Bmult[2]  => Cmult[16]) = "";
		(Bmult[3]  => Cmult[16]) = "";
		(Bmult[4]  => Cmult[16]) = "";
		(Bmult[5]  => Cmult[16]) = "";
		(Bmult[6]  => Cmult[16]) = "";
		(Bmult[7]  => Cmult[16]) = "";
		(Bmult[8]  => Cmult[16]) = "";
		(Bmult[9]  => Cmult[16]) = "";
		(Bmult[10] => Cmult[16]) = "";
		(Bmult[11] => Cmult[16]) = "";
		(Bmult[12] => Cmult[16]) = "";
		(Bmult[13] => Cmult[16]) = "";
		(Bmult[14] => Cmult[16]) = "";
		(Bmult[15] => Cmult[16]) = "";
		(Bmult[16] => Cmult[16]) = "";
		(Bmult[17] => Cmult[16]) = "";
		(Bmult[18] => Cmult[16]) = "";
		(Bmult[19] => Cmult[16]) = "";
		(Bmult[20] => Cmult[16]) = "";
		(Bmult[21] => Cmult[16]) = "";
		(Bmult[22] => Cmult[16]) = "";
		(Bmult[23] => Cmult[16]) = "";
		(Bmult[24] => Cmult[16]) = "";
		(Bmult[25] => Cmult[16]) = "";
		(Bmult[26] => Cmult[16]) = "";
		(Bmult[27] => Cmult[16]) = "";
		(Bmult[28] => Cmult[16]) = "";
		(Bmult[29] => Cmult[16]) = "";
		(Bmult[30] => Cmult[16]) = "";
		(Bmult[31] => Cmult[16]) = "";		
		(Valid_mult[0] => Cmult[16]) = "";
		(Valid_mult[1] => Cmult[16]) = "";
		(sel_mul_32x32 => Cmult[16]) = "";
		(Amult[0]  => Cmult[17]) = "";
		(Amult[1]  => Cmult[17]) = "";
		(Amult[2]  => Cmult[17]) = "";
		(Amult[3]  => Cmult[17]) = "";
		(Amult[4]  => Cmult[17]) = "";
		(Amult[5]  => Cmult[17]) = "";
		(Amult[6]  => Cmult[17]) = "";
		(Amult[7]  => Cmult[17]) = "";
		(Amult[8]  => Cmult[17]) = "";
		(Amult[9]  => Cmult[17]) = "";
		(Amult[10] => Cmult[17]) = "";
		(Amult[11] => Cmult[17]) = "";
		(Amult[12] => Cmult[17]) = "";
		(Amult[13] => Cmult[17]) = "";
		(Amult[14] => Cmult[17]) = "";
		(Amult[15] => Cmult[17]) = "";
		(Amult[16] => Cmult[17]) = "";
		(Amult[17] => Cmult[17]) = "";
		(Amult[18] => Cmult[17]) = "";
		(Amult[19] => Cmult[17]) = "";
		(Amult[20] => Cmult[17]) = "";
		(Amult[21] => Cmult[17]) = "";
		(Amult[22] => Cmult[17]) = "";
		(Amult[23] => Cmult[17]) = "";
		(Amult[24] => Cmult[17]) = "";
		(Amult[25] => Cmult[17]) = "";
		(Amult[26] => Cmult[17]) = "";
		(Amult[27] => Cmult[17]) = "";
		(Amult[28] => Cmult[17]) = "";
		(Amult[29] => Cmult[17]) = "";
		(Amult[30] => Cmult[17]) = "";
		(Amult[31] => Cmult[17]) = "";
		(Bmult[0]  => Cmult[17]) = "";
		(Bmult[1]  => Cmult[17]) = "";
		(Bmult[2]  => Cmult[17]) = "";
		(Bmult[3]  => Cmult[17]) = "";
		(Bmult[4]  => Cmult[17]) = "";
		(Bmult[5]  => Cmult[17]) = "";
		(Bmult[6]  => Cmult[17]) = "";
		(Bmult[7]  => Cmult[17]) = "";
		(Bmult[8]  => Cmult[17]) = "";
		(Bmult[9]  => Cmult[17]) = "";
		(Bmult[10] => Cmult[17]) = "";
		(Bmult[11] => Cmult[17]) = "";
		(Bmult[12] => Cmult[17]) = "";
		(Bmult[13] => Cmult[17]) = "";
		(Bmult[14] => Cmult[17]) = "";
		(Bmult[15] => Cmult[17]) = "";
		(Bmult[16] => Cmult[17]) = "";
		(Bmult[17] => Cmult[17]) = "";
		(Bmult[18] => Cmult[17]) = "";
		(Bmult[19] => Cmult[17]) = "";
		(Bmult[20] => Cmult[17]) = "";
		(Bmult[21] => Cmult[17]) = "";
		(Bmult[22] => Cmult[17]) = "";
		(Bmult[23] => Cmult[17]) = "";
		(Bmult[24] => Cmult[17]) = "";
		(Bmult[25] => Cmult[17]) = "";
		(Bmult[26] => Cmult[17]) = "";
		(Bmult[27] => Cmult[17]) = "";
		(Bmult[28] => Cmult[17]) = "";
		(Bmult[29] => Cmult[17]) = "";
		(Bmult[30] => Cmult[17]) = "";
		(Bmult[31] => Cmult[17]) = "";		
		(Valid_mult[0] => Cmult[17]) = "";
		(Valid_mult[1] => Cmult[17]) = "";
		(sel_mul_32x32 => Cmult[17]) = "";
		(Amult[0]  => Cmult[18]) = "";
		(Amult[1]  => Cmult[18]) = "";
		(Amult[2]  => Cmult[18]) = "";
		(Amult[3]  => Cmult[18]) = "";
		(Amult[4]  => Cmult[18]) = "";
		(Amult[5]  => Cmult[18]) = "";
		(Amult[6]  => Cmult[18]) = "";
		(Amult[7]  => Cmult[18]) = "";
		(Amult[8]  => Cmult[18]) = "";
		(Amult[9]  => Cmult[18]) = "";
		(Amult[10] => Cmult[18]) = "";
		(Amult[11] => Cmult[18]) = "";
		(Amult[12] => Cmult[18]) = "";
		(Amult[13] => Cmult[18]) = "";
		(Amult[14] => Cmult[18]) = "";
		(Amult[15] => Cmult[18]) = "";
		(Amult[16] => Cmult[18]) = "";
		(Amult[17] => Cmult[18]) = "";
		(Amult[18] => Cmult[18]) = "";
		(Amult[19] => Cmult[18]) = "";
		(Amult[20] => Cmult[18]) = "";
		(Amult[21] => Cmult[18]) = "";
		(Amult[22] => Cmult[18]) = "";
		(Amult[23] => Cmult[18]) = "";
		(Amult[24] => Cmult[18]) = "";
		(Amult[25] => Cmult[18]) = "";
		(Amult[26] => Cmult[18]) = "";
		(Amult[27] => Cmult[18]) = "";
		(Amult[28] => Cmult[18]) = "";
		(Amult[29] => Cmult[18]) = "";
		(Amult[30] => Cmult[18]) = "";
		(Amult[31] => Cmult[18]) = "";
		(Bmult[0]  => Cmult[18]) = "";
		(Bmult[1]  => Cmult[18]) = "";
		(Bmult[2]  => Cmult[18]) = "";
		(Bmult[3]  => Cmult[18]) = "";
		(Bmult[4]  => Cmult[18]) = "";
		(Bmult[5]  => Cmult[18]) = "";
		(Bmult[6]  => Cmult[18]) = "";
		(Bmult[7]  => Cmult[18]) = "";
		(Bmult[8]  => Cmult[18]) = "";
		(Bmult[9]  => Cmult[18]) = "";
		(Bmult[10] => Cmult[18]) = "";
		(Bmult[11] => Cmult[18]) = "";
		(Bmult[12] => Cmult[18]) = "";
		(Bmult[13] => Cmult[18]) = "";
		(Bmult[14] => Cmult[18]) = "";
		(Bmult[15] => Cmult[18]) = "";
		(Bmult[16] => Cmult[18]) = "";
		(Bmult[17] => Cmult[18]) = "";
		(Bmult[18] => Cmult[18]) = "";
		(Bmult[19] => Cmult[18]) = "";
		(Bmult[20] => Cmult[18]) = "";
		(Bmult[21] => Cmult[18]) = "";
		(Bmult[22] => Cmult[18]) = "";
		(Bmult[23] => Cmult[18]) = "";
		(Bmult[24] => Cmult[18]) = "";
		(Bmult[25] => Cmult[18]) = "";
		(Bmult[26] => Cmult[18]) = "";
		(Bmult[27] => Cmult[18]) = "";
		(Bmult[28] => Cmult[18]) = "";
		(Bmult[29] => Cmult[18]) = "";
		(Bmult[30] => Cmult[18]) = "";
		(Bmult[31] => Cmult[18]) = "";		
		(Valid_mult[0] => Cmult[18]) = "";
		(Valid_mult[1] => Cmult[18]) = "";
		(sel_mul_32x32 => Cmult[18]) = "";	
		(Amult[0]  => Cmult[19]) = "";
		(Amult[1]  => Cmult[19]) = "";
		(Amult[2]  => Cmult[19]) = "";
		(Amult[3]  => Cmult[19]) = "";
		(Amult[4]  => Cmult[19]) = "";
		(Amult[5]  => Cmult[19]) = "";
		(Amult[6]  => Cmult[19]) = "";
		(Amult[7]  => Cmult[19]) = "";
		(Amult[8]  => Cmult[19]) = "";
		(Amult[9]  => Cmult[19]) = "";
		(Amult[10] => Cmult[19]) = "";
		(Amult[11] => Cmult[19]) = "";
		(Amult[12] => Cmult[19]) = "";
		(Amult[13] => Cmult[19]) = "";
		(Amult[14] => Cmult[19]) = "";
		(Amult[15] => Cmult[19]) = "";
		(Amult[16] => Cmult[19]) = "";
		(Amult[17] => Cmult[19]) = "";
		(Amult[18] => Cmult[19]) = "";
		(Amult[19] => Cmult[19]) = "";
		(Amult[20] => Cmult[19]) = "";
		(Amult[21] => Cmult[19]) = "";
		(Amult[22] => Cmult[19]) = "";
		(Amult[23] => Cmult[19]) = "";
		(Amult[24] => Cmult[19]) = "";
		(Amult[25] => Cmult[19]) = "";
		(Amult[26] => Cmult[19]) = "";
		(Amult[27] => Cmult[19]) = "";
		(Amult[28] => Cmult[19]) = "";
		(Amult[29] => Cmult[19]) = "";
		(Amult[30] => Cmult[19]) = "";
		(Amult[31] => Cmult[19]) = "";
		(Bmult[0]  => Cmult[19]) = "";
		(Bmult[1]  => Cmult[19]) = "";
		(Bmult[2]  => Cmult[19]) = "";
		(Bmult[3]  => Cmult[19]) = "";
		(Bmult[4]  => Cmult[19]) = "";
		(Bmult[5]  => Cmult[19]) = "";
		(Bmult[6]  => Cmult[19]) = "";
		(Bmult[7]  => Cmult[19]) = "";
		(Bmult[8]  => Cmult[19]) = "";
		(Bmult[9]  => Cmult[19]) = "";
		(Bmult[10] => Cmult[19]) = "";
		(Bmult[11] => Cmult[19]) = "";
		(Bmult[12] => Cmult[19]) = "";
		(Bmult[13] => Cmult[19]) = "";
		(Bmult[14] => Cmult[19]) = "";
		(Bmult[15] => Cmult[19]) = "";
		(Bmult[16] => Cmult[19]) = "";
		(Bmult[17] => Cmult[19]) = "";
		(Bmult[18] => Cmult[19]) = "";
		(Bmult[19] => Cmult[19]) = "";
		(Bmult[20] => Cmult[19]) = "";
		(Bmult[21] => Cmult[19]) = "";
		(Bmult[22] => Cmult[19]) = "";
		(Bmult[23] => Cmult[19]) = "";
		(Bmult[24] => Cmult[19]) = "";
		(Bmult[25] => Cmult[19]) = "";
		(Bmult[26] => Cmult[19]) = "";
		(Bmult[27] => Cmult[19]) = "";
		(Bmult[28] => Cmult[19]) = "";
		(Bmult[29] => Cmult[19]) = "";
		(Bmult[30] => Cmult[19]) = "";
		(Bmult[31] => Cmult[19]) = "";		
		(Valid_mult[0] => Cmult[19]) = "";
		(Valid_mult[1] => Cmult[19]) = "";
		(sel_mul_32x32 => Cmult[19]) = "";
		(Amult[0]  => Cmult[20]) = "";
		(Amult[1]  => Cmult[20]) = "";
		(Amult[2]  => Cmult[20]) = "";
		(Amult[3]  => Cmult[20]) = "";
		(Amult[4]  => Cmult[20]) = "";
		(Amult[5]  => Cmult[20]) = "";
		(Amult[6]  => Cmult[20]) = "";
		(Amult[7]  => Cmult[20]) = "";
		(Amult[8]  => Cmult[20]) = "";
		(Amult[9]  => Cmult[20]) = "";
		(Amult[10] => Cmult[20]) = "";
		(Amult[11] => Cmult[20]) = "";
		(Amult[12] => Cmult[20]) = "";
		(Amult[13] => Cmult[20]) = "";
		(Amult[14] => Cmult[20]) = "";
		(Amult[15] => Cmult[20]) = "";
		(Amult[16] => Cmult[20]) = "";
		(Amult[17] => Cmult[20]) = "";
		(Amult[18] => Cmult[20]) = "";
		(Amult[19] => Cmult[20]) = "";
		(Amult[20] => Cmult[20]) = "";
		(Amult[21] => Cmult[20]) = "";
		(Amult[22] => Cmult[20]) = "";
		(Amult[23] => Cmult[20]) = "";
		(Amult[24] => Cmult[20]) = "";
		(Amult[25] => Cmult[20]) = "";
		(Amult[26] => Cmult[20]) = "";
		(Amult[27] => Cmult[20]) = "";
		(Amult[28] => Cmult[20]) = "";
		(Amult[29] => Cmult[20]) = "";
		(Amult[30] => Cmult[20]) = "";
		(Amult[31] => Cmult[20]) = "";
		(Bmult[0]  => Cmult[20]) = "";
		(Bmult[1]  => Cmult[20]) = "";
		(Bmult[2]  => Cmult[20]) = "";
		(Bmult[3]  => Cmult[20]) = "";
		(Bmult[4]  => Cmult[20]) = "";
		(Bmult[5]  => Cmult[20]) = "";
		(Bmult[6]  => Cmult[20]) = "";
		(Bmult[7]  => Cmult[20]) = "";
		(Bmult[8]  => Cmult[20]) = "";
		(Bmult[9]  => Cmult[20]) = "";
		(Bmult[10] => Cmult[20]) = "";
		(Bmult[11] => Cmult[20]) = "";
		(Bmult[12] => Cmult[20]) = "";
		(Bmult[13] => Cmult[20]) = "";
		(Bmult[14] => Cmult[20]) = "";
		(Bmult[15] => Cmult[20]) = "";
		(Bmult[16] => Cmult[20]) = "";
		(Bmult[17] => Cmult[20]) = "";
		(Bmult[18] => Cmult[20]) = "";
		(Bmult[19] => Cmult[20]) = "";
		(Bmult[20] => Cmult[20]) = "";
		(Bmult[21] => Cmult[20]) = "";
		(Bmult[22] => Cmult[20]) = "";
		(Bmult[23] => Cmult[20]) = "";
		(Bmult[24] => Cmult[20]) = "";
		(Bmult[25] => Cmult[20]) = "";
		(Bmult[26] => Cmult[20]) = "";
		(Bmult[27] => Cmult[20]) = "";
		(Bmult[28] => Cmult[20]) = "";
		(Bmult[29] => Cmult[20]) = "";
		(Bmult[30] => Cmult[20]) = "";
		(Bmult[31] => Cmult[20]) = "";		
		(Valid_mult[0] => Cmult[20]) = "";
		(Valid_mult[1] => Cmult[20]) = "";
		(sel_mul_32x32 => Cmult[20]) = "";
		(Amult[0]  => Cmult[21]) = "";
		(Amult[1]  => Cmult[21]) = "";
		(Amult[2]  => Cmult[21]) = "";
		(Amult[3]  => Cmult[21]) = "";
		(Amult[4]  => Cmult[21]) = "";
		(Amult[5]  => Cmult[21]) = "";
		(Amult[6]  => Cmult[21]) = "";
		(Amult[7]  => Cmult[21]) = "";
		(Amult[8]  => Cmult[21]) = "";
		(Amult[9]  => Cmult[21]) = "";
		(Amult[10] => Cmult[21]) = "";
		(Amult[11] => Cmult[21]) = "";
		(Amult[12] => Cmult[21]) = "";
		(Amult[13] => Cmult[21]) = "";
		(Amult[14] => Cmult[21]) = "";
		(Amult[15] => Cmult[21]) = "";
		(Amult[16] => Cmult[21]) = "";
		(Amult[17] => Cmult[21]) = "";
		(Amult[18] => Cmult[21]) = "";
		(Amult[19] => Cmult[21]) = "";
		(Amult[20] => Cmult[21]) = "";
		(Amult[21] => Cmult[21]) = "";
		(Amult[22] => Cmult[21]) = "";
		(Amult[23] => Cmult[21]) = "";
		(Amult[24] => Cmult[21]) = "";
		(Amult[25] => Cmult[21]) = "";
		(Amult[26] => Cmult[21]) = "";
		(Amult[27] => Cmult[21]) = "";
		(Amult[28] => Cmult[21]) = "";
		(Amult[29] => Cmult[21]) = "";
		(Amult[30] => Cmult[21]) = "";
		(Amult[31] => Cmult[21]) = "";
		(Bmult[0]  => Cmult[21]) = "";
		(Bmult[1]  => Cmult[21]) = "";
		(Bmult[2]  => Cmult[21]) = "";
		(Bmult[3]  => Cmult[21]) = "";
		(Bmult[4]  => Cmult[21]) = "";
		(Bmult[5]  => Cmult[21]) = "";
		(Bmult[6]  => Cmult[21]) = "";
		(Bmult[7]  => Cmult[21]) = "";
		(Bmult[8]  => Cmult[21]) = "";
		(Bmult[9]  => Cmult[21]) = "";
		(Bmult[10] => Cmult[21]) = "";
		(Bmult[11] => Cmult[21]) = "";
		(Bmult[12] => Cmult[21]) = "";
		(Bmult[13] => Cmult[21]) = "";
		(Bmult[14] => Cmult[21]) = "";
		(Bmult[15] => Cmult[21]) = "";
		(Bmult[16] => Cmult[21]) = "";
		(Bmult[17] => Cmult[21]) = "";
		(Bmult[18] => Cmult[21]) = "";
		(Bmult[19] => Cmult[21]) = "";
		(Bmult[20] => Cmult[21]) = "";
		(Bmult[21] => Cmult[21]) = "";
		(Bmult[22] => Cmult[21]) = "";
		(Bmult[23] => Cmult[21]) = "";
		(Bmult[24] => Cmult[21]) = "";
		(Bmult[25] => Cmult[21]) = "";
		(Bmult[26] => Cmult[21]) = "";
		(Bmult[27] => Cmult[21]) = "";
		(Bmult[28] => Cmult[21]) = "";
		(Bmult[29] => Cmult[21]) = "";
		(Bmult[30] => Cmult[21]) = "";
		(Bmult[31] => Cmult[21]) = "";		
		(Valid_mult[0] => Cmult[21]) = "";
		(Valid_mult[1] => Cmult[21]) = "";
		(sel_mul_32x32 => Cmult[21]) = "";
		(Amult[0]  => Cmult[22]) = "";
		(Amult[1]  => Cmult[22]) = "";
		(Amult[2]  => Cmult[22]) = "";
		(Amult[3]  => Cmult[22]) = "";
		(Amult[4]  => Cmult[22]) = "";
		(Amult[5]  => Cmult[22]) = "";
		(Amult[6]  => Cmult[22]) = "";
		(Amult[7]  => Cmult[22]) = "";
		(Amult[8]  => Cmult[22]) = "";
		(Amult[9]  => Cmult[22]) = "";
		(Amult[10] => Cmult[22]) = "";
		(Amult[11] => Cmult[22]) = "";
		(Amult[12] => Cmult[22]) = "";
		(Amult[13] => Cmult[22]) = "";
		(Amult[14] => Cmult[22]) = "";
		(Amult[15] => Cmult[22]) = "";
		(Amult[16] => Cmult[22]) = "";
		(Amult[17] => Cmult[22]) = "";
		(Amult[18] => Cmult[22]) = "";
		(Amult[19] => Cmult[22]) = "";
		(Amult[20] => Cmult[22]) = "";
		(Amult[21] => Cmult[22]) = "";
		(Amult[22] => Cmult[22]) = "";
		(Amult[23] => Cmult[22]) = "";
		(Amult[24] => Cmult[22]) = "";
		(Amult[25] => Cmult[22]) = "";
		(Amult[26] => Cmult[22]) = "";
		(Amult[27] => Cmult[22]) = "";
		(Amult[28] => Cmult[22]) = "";
		(Amult[29] => Cmult[22]) = "";
		(Amult[30] => Cmult[22]) = "";
		(Amult[31] => Cmult[22]) = "";
		(Bmult[0]  => Cmult[22]) = "";
		(Bmult[1]  => Cmult[22]) = "";
		(Bmult[2]  => Cmult[22]) = "";
		(Bmult[3]  => Cmult[22]) = "";
		(Bmult[4]  => Cmult[22]) = "";
		(Bmult[5]  => Cmult[22]) = "";
		(Bmult[6]  => Cmult[22]) = "";
		(Bmult[7]  => Cmult[22]) = "";
		(Bmult[8]  => Cmult[22]) = "";
		(Bmult[9]  => Cmult[22]) = "";
		(Bmult[10] => Cmult[22]) = "";
		(Bmult[11] => Cmult[22]) = "";
		(Bmult[12] => Cmult[22]) = "";
		(Bmult[13] => Cmult[22]) = "";
		(Bmult[14] => Cmult[22]) = "";
		(Bmult[15] => Cmult[22]) = "";
		(Bmult[16] => Cmult[22]) = "";
		(Bmult[17] => Cmult[22]) = "";
		(Bmult[18] => Cmult[22]) = "";
		(Bmult[19] => Cmult[22]) = "";
		(Bmult[20] => Cmult[22]) = "";
		(Bmult[21] => Cmult[22]) = "";
		(Bmult[22] => Cmult[22]) = "";
		(Bmult[23] => Cmult[22]) = "";
		(Bmult[24] => Cmult[22]) = "";
		(Bmult[25] => Cmult[22]) = "";
		(Bmult[26] => Cmult[22]) = "";
		(Bmult[27] => Cmult[22]) = "";
		(Bmult[28] => Cmult[22]) = "";
		(Bmult[29] => Cmult[22]) = "";
		(Bmult[30] => Cmult[22]) = "";
		(Bmult[31] => Cmult[22]) = "";		
		(Valid_mult[0] => Cmult[22]) = "";
		(Valid_mult[1] => Cmult[22]) = "";
		(sel_mul_32x32 => Cmult[22]) = "";
		(Amult[0]  => Cmult[23]) = "";
		(Amult[1]  => Cmult[23]) = "";
		(Amult[2]  => Cmult[23]) = "";
		(Amult[3]  => Cmult[23]) = "";
		(Amult[4]  => Cmult[23]) = "";
		(Amult[5]  => Cmult[23]) = "";
		(Amult[6]  => Cmult[23]) = "";
		(Amult[7]  => Cmult[23]) = "";
		(Amult[8]  => Cmult[23]) = "";
		(Amult[9]  => Cmult[23]) = "";
		(Amult[10] => Cmult[23]) = "";
		(Amult[11] => Cmult[23]) = "";
		(Amult[12] => Cmult[23]) = "";
		(Amult[13] => Cmult[23]) = "";
		(Amult[14] => Cmult[23]) = "";
		(Amult[15] => Cmult[23]) = "";
		(Amult[16] => Cmult[23]) = "";
		(Amult[17] => Cmult[23]) = "";
		(Amult[18] => Cmult[23]) = "";
		(Amult[19] => Cmult[23]) = "";
		(Amult[20] => Cmult[23]) = "";
		(Amult[21] => Cmult[23]) = "";
		(Amult[22] => Cmult[23]) = "";
		(Amult[23] => Cmult[23]) = "";
		(Amult[24] => Cmult[23]) = "";
		(Amult[25] => Cmult[23]) = "";
		(Amult[26] => Cmult[23]) = "";
		(Amult[27] => Cmult[23]) = "";
		(Amult[28] => Cmult[23]) = "";
		(Amult[29] => Cmult[23]) = "";
		(Amult[30] => Cmult[23]) = "";
		(Amult[31] => Cmult[23]) = "";
		(Bmult[0]  => Cmult[23]) = "";
		(Bmult[1]  => Cmult[23]) = "";
		(Bmult[2]  => Cmult[23]) = "";
		(Bmult[3]  => Cmult[23]) = "";
		(Bmult[4]  => Cmult[23]) = "";
		(Bmult[5]  => Cmult[23]) = "";
		(Bmult[6]  => Cmult[23]) = "";
		(Bmult[7]  => Cmult[23]) = "";
		(Bmult[8]  => Cmult[23]) = "";
		(Bmult[9]  => Cmult[23]) = "";
		(Bmult[10] => Cmult[23]) = "";
		(Bmult[11] => Cmult[23]) = "";
		(Bmult[12] => Cmult[23]) = "";
		(Bmult[13] => Cmult[23]) = "";
		(Bmult[14] => Cmult[23]) = "";
		(Bmult[15] => Cmult[23]) = "";
		(Bmult[16] => Cmult[23]) = "";
		(Bmult[17] => Cmult[23]) = "";
		(Bmult[18] => Cmult[23]) = "";
		(Bmult[19] => Cmult[23]) = "";
		(Bmult[20] => Cmult[23]) = "";
		(Bmult[21] => Cmult[23]) = "";
		(Bmult[22] => Cmult[23]) = "";
		(Bmult[23] => Cmult[23]) = "";
		(Bmult[24] => Cmult[23]) = "";
		(Bmult[25] => Cmult[23]) = "";
		(Bmult[26] => Cmult[23]) = "";
		(Bmult[27] => Cmult[23]) = "";
		(Bmult[28] => Cmult[23]) = "";
		(Bmult[29] => Cmult[23]) = "";
		(Bmult[30] => Cmult[23]) = "";
		(Bmult[31] => Cmult[23]) = "";		
		(Valid_mult[0] => Cmult[23]) = "";
		(Valid_mult[1] => Cmult[23]) = "";
		(sel_mul_32x32 => Cmult[23]) = "";
		(Amult[0]  => Cmult[24]) = "";
		(Amult[1]  => Cmult[24]) = "";
		(Amult[2]  => Cmult[24]) = "";
		(Amult[3]  => Cmult[24]) = "";
		(Amult[4]  => Cmult[24]) = "";
		(Amult[5]  => Cmult[24]) = "";
		(Amult[6]  => Cmult[24]) = "";
		(Amult[7]  => Cmult[24]) = "";
		(Amult[8]  => Cmult[24]) = "";
		(Amult[9]  => Cmult[24]) = "";
		(Amult[10] => Cmult[24]) = "";
		(Amult[11] => Cmult[24]) = "";
		(Amult[12] => Cmult[24]) = "";
		(Amult[13] => Cmult[24]) = "";
		(Amult[14] => Cmult[24]) = "";
		(Amult[15] => Cmult[24]) = "";
		(Amult[16] => Cmult[24]) = "";
		(Amult[17] => Cmult[24]) = "";
		(Amult[18] => Cmult[24]) = "";
		(Amult[19] => Cmult[24]) = "";
		(Amult[20] => Cmult[24]) = "";
		(Amult[21] => Cmult[24]) = "";
		(Amult[22] => Cmult[24]) = "";
		(Amult[23] => Cmult[24]) = "";
		(Amult[24] => Cmult[24]) = "";
		(Amult[25] => Cmult[24]) = "";
		(Amult[26] => Cmult[24]) = "";
		(Amult[27] => Cmult[24]) = "";
		(Amult[28] => Cmult[24]) = "";
		(Amult[29] => Cmult[24]) = "";
		(Amult[30] => Cmult[24]) = "";
		(Amult[31] => Cmult[24]) = "";
		(Bmult[0]  => Cmult[24]) = "";
		(Bmult[1]  => Cmult[24]) = "";
		(Bmult[2]  => Cmult[24]) = "";
		(Bmult[3]  => Cmult[24]) = "";
		(Bmult[4]  => Cmult[24]) = "";
		(Bmult[5]  => Cmult[24]) = "";
		(Bmult[6]  => Cmult[24]) = "";
		(Bmult[7]  => Cmult[24]) = "";
		(Bmult[8]  => Cmult[24]) = "";
		(Bmult[9]  => Cmult[24]) = "";
		(Bmult[10] => Cmult[24]) = "";
		(Bmult[11] => Cmult[24]) = "";
		(Bmult[12] => Cmult[24]) = "";
		(Bmult[13] => Cmult[24]) = "";
		(Bmult[14] => Cmult[24]) = "";
		(Bmult[15] => Cmult[24]) = "";
		(Bmult[16] => Cmult[24]) = "";
		(Bmult[17] => Cmult[24]) = "";
		(Bmult[18] => Cmult[24]) = "";
		(Bmult[19] => Cmult[24]) = "";
		(Bmult[20] => Cmult[24]) = "";
		(Bmult[21] => Cmult[24]) = "";
		(Bmult[22] => Cmult[24]) = "";
		(Bmult[23] => Cmult[24]) = "";
		(Bmult[24] => Cmult[24]) = "";
		(Bmult[25] => Cmult[24]) = "";
		(Bmult[26] => Cmult[24]) = "";
		(Bmult[27] => Cmult[24]) = "";
		(Bmult[28] => Cmult[24]) = "";
		(Bmult[29] => Cmult[24]) = "";
		(Bmult[30] => Cmult[24]) = "";
		(Bmult[31] => Cmult[24]) = "";		
		(Valid_mult[0] => Cmult[24]) = "";
		(Valid_mult[1] => Cmult[24]) = "";
		(sel_mul_32x32 => Cmult[24]) = "";
		(Amult[0]  => Cmult[25]) = "";
		(Amult[1]  => Cmult[25]) = "";
		(Amult[2]  => Cmult[25]) = "";
		(Amult[3]  => Cmult[25]) = "";
		(Amult[4]  => Cmult[25]) = "";
		(Amult[5]  => Cmult[25]) = "";
		(Amult[6]  => Cmult[25]) = "";
		(Amult[7]  => Cmult[25]) = "";
		(Amult[8]  => Cmult[25]) = "";
		(Amult[9]  => Cmult[25]) = "";
		(Amult[10] => Cmult[25]) = "";
		(Amult[11] => Cmult[25]) = "";
		(Amult[12] => Cmult[25]) = "";
		(Amult[13] => Cmult[25]) = "";
		(Amult[14] => Cmult[25]) = "";
		(Amult[15] => Cmult[25]) = "";
		(Amult[16] => Cmult[25]) = "";
		(Amult[17] => Cmult[25]) = "";
		(Amult[18] => Cmult[25]) = "";
		(Amult[19] => Cmult[25]) = "";
		(Amult[20] => Cmult[25]) = "";
		(Amult[21] => Cmult[25]) = "";
		(Amult[22] => Cmult[25]) = "";
		(Amult[23] => Cmult[25]) = "";
		(Amult[24] => Cmult[25]) = "";
		(Amult[25] => Cmult[25]) = "";
		(Amult[26] => Cmult[25]) = "";
		(Amult[27] => Cmult[25]) = "";
		(Amult[28] => Cmult[25]) = "";
		(Amult[29] => Cmult[25]) = "";
		(Amult[30] => Cmult[25]) = "";
		(Amult[31] => Cmult[25]) = "";
		(Bmult[0]  => Cmult[25]) = "";
		(Bmult[1]  => Cmult[25]) = "";
		(Bmult[2]  => Cmult[25]) = "";
		(Bmult[3]  => Cmult[25]) = "";
		(Bmult[4]  => Cmult[25]) = "";
		(Bmult[5]  => Cmult[25]) = "";
		(Bmult[6]  => Cmult[25]) = "";
		(Bmult[7]  => Cmult[25]) = "";
		(Bmult[8]  => Cmult[25]) = "";
		(Bmult[9]  => Cmult[25]) = "";
		(Bmult[10] => Cmult[25]) = "";
		(Bmult[11] => Cmult[25]) = "";
		(Bmult[12] => Cmult[25]) = "";
		(Bmult[13] => Cmult[25]) = "";
		(Bmult[14] => Cmult[25]) = "";
		(Bmult[15] => Cmult[25]) = "";
		(Bmult[16] => Cmult[25]) = "";
		(Bmult[17] => Cmult[25]) = "";
		(Bmult[18] => Cmult[25]) = "";
		(Bmult[19] => Cmult[25]) = "";
		(Bmult[20] => Cmult[25]) = "";
		(Bmult[21] => Cmult[25]) = "";
		(Bmult[22] => Cmult[25]) = "";
		(Bmult[23] => Cmult[25]) = "";
		(Bmult[24] => Cmult[25]) = "";
		(Bmult[25] => Cmult[25]) = "";
		(Bmult[26] => Cmult[25]) = "";
		(Bmult[27] => Cmult[25]) = "";
		(Bmult[28] => Cmult[25]) = "";
		(Bmult[29] => Cmult[25]) = "";
		(Bmult[30] => Cmult[25]) = "";
		(Bmult[31] => Cmult[25]) = "";		
		(Valid_mult[0] => Cmult[25]) = "";
		(Valid_mult[1] => Cmult[25]) = "";
		(sel_mul_32x32 => Cmult[25]) = "";
		(Amult[0]  => Cmult[26]) = "";
		(Amult[1]  => Cmult[26]) = "";
		(Amult[2]  => Cmult[26]) = "";
		(Amult[3]  => Cmult[26]) = "";
		(Amult[4]  => Cmult[26]) = "";
		(Amult[5]  => Cmult[26]) = "";
		(Amult[6]  => Cmult[26]) = "";
		(Amult[7]  => Cmult[26]) = "";
		(Amult[8]  => Cmult[26]) = "";
		(Amult[9]  => Cmult[26]) = "";
		(Amult[10] => Cmult[26]) = "";
		(Amult[11] => Cmult[26]) = "";
		(Amult[12] => Cmult[26]) = "";
		(Amult[13] => Cmult[26]) = "";
		(Amult[14] => Cmult[26]) = "";
		(Amult[15] => Cmult[26]) = "";
		(Amult[16] => Cmult[26]) = "";
		(Amult[17] => Cmult[26]) = "";
		(Amult[18] => Cmult[26]) = "";
		(Amult[19] => Cmult[26]) = "";
		(Amult[20] => Cmult[26]) = "";
		(Amult[21] => Cmult[26]) = "";
		(Amult[22] => Cmult[26]) = "";
		(Amult[23] => Cmult[26]) = "";
		(Amult[24] => Cmult[26]) = "";
		(Amult[25] => Cmult[26]) = "";
		(Amult[26] => Cmult[26]) = "";
		(Amult[27] => Cmult[26]) = "";
		(Amult[28] => Cmult[26]) = "";
		(Amult[29] => Cmult[26]) = "";
		(Amult[30] => Cmult[26]) = "";
		(Amult[31] => Cmult[26]) = "";
		(Bmult[0]  => Cmult[26]) = "";
		(Bmult[1]  => Cmult[26]) = "";
		(Bmult[2]  => Cmult[26]) = "";
		(Bmult[3]  => Cmult[26]) = "";
		(Bmult[4]  => Cmult[26]) = "";
		(Bmult[5]  => Cmult[26]) = "";
		(Bmult[6]  => Cmult[26]) = "";
		(Bmult[7]  => Cmult[26]) = "";
		(Bmult[8]  => Cmult[26]) = "";
		(Bmult[9]  => Cmult[26]) = "";
		(Bmult[10] => Cmult[26]) = "";
		(Bmult[11] => Cmult[26]) = "";
		(Bmult[12] => Cmult[26]) = "";
		(Bmult[13] => Cmult[26]) = "";
		(Bmult[14] => Cmult[26]) = "";
		(Bmult[15] => Cmult[26]) = "";
		(Bmult[16] => Cmult[26]) = "";
		(Bmult[17] => Cmult[26]) = "";
		(Bmult[18] => Cmult[26]) = "";
		(Bmult[19] => Cmult[26]) = "";
		(Bmult[20] => Cmult[26]) = "";
		(Bmult[21] => Cmult[26]) = "";
		(Bmult[22] => Cmult[26]) = "";
		(Bmult[23] => Cmult[26]) = "";
		(Bmult[24] => Cmult[26]) = "";
		(Bmult[25] => Cmult[26]) = "";
		(Bmult[26] => Cmult[26]) = "";
		(Bmult[27] => Cmult[26]) = "";
		(Bmult[28] => Cmult[26]) = "";
		(Bmult[29] => Cmult[26]) = "";
		(Bmult[30] => Cmult[26]) = "";
		(Bmult[31] => Cmult[26]) = "";		
		(Valid_mult[0] => Cmult[26]) = "";
		(Valid_mult[1] => Cmult[26]) = "";
		(sel_mul_32x32 => Cmult[26]) = "";
		(Amult[0]  => Cmult[27]) = "";
		(Amult[1]  => Cmult[27]) = "";
		(Amult[2]  => Cmult[27]) = "";
		(Amult[3]  => Cmult[27]) = "";
		(Amult[4]  => Cmult[27]) = "";
		(Amult[5]  => Cmult[27]) = "";
		(Amult[6]  => Cmult[27]) = "";
		(Amult[7]  => Cmult[27]) = "";
		(Amult[8]  => Cmult[27]) = "";
		(Amult[9]  => Cmult[27]) = "";
		(Amult[10] => Cmult[27]) = "";
		(Amult[11] => Cmult[27]) = "";
		(Amult[12] => Cmult[27]) = "";
		(Amult[13] => Cmult[27]) = "";
		(Amult[14] => Cmult[27]) = "";
		(Amult[15] => Cmult[27]) = "";
		(Amult[16] => Cmult[27]) = "";
		(Amult[17] => Cmult[27]) = "";
		(Amult[18] => Cmult[27]) = "";
		(Amult[19] => Cmult[27]) = "";
		(Amult[20] => Cmult[27]) = "";
		(Amult[21] => Cmult[27]) = "";
		(Amult[22] => Cmult[27]) = "";
		(Amult[23] => Cmult[27]) = "";
		(Amult[24] => Cmult[27]) = "";
		(Amult[25] => Cmult[27]) = "";
		(Amult[26] => Cmult[27]) = "";
		(Amult[27] => Cmult[27]) = "";
		(Amult[28] => Cmult[27]) = "";
		(Amult[29] => Cmult[27]) = "";
		(Amult[30] => Cmult[27]) = "";
		(Amult[31] => Cmult[27]) = "";
		(Bmult[0]  => Cmult[27]) = "";
		(Bmult[1]  => Cmult[27]) = "";
		(Bmult[2]  => Cmult[27]) = "";
		(Bmult[3]  => Cmult[27]) = "";
		(Bmult[4]  => Cmult[27]) = "";
		(Bmult[5]  => Cmult[27]) = "";
		(Bmult[6]  => Cmult[27]) = "";
		(Bmult[7]  => Cmult[27]) = "";
		(Bmult[8]  => Cmult[27]) = "";
		(Bmult[9]  => Cmult[27]) = "";
		(Bmult[10] => Cmult[27]) = "";
		(Bmult[11] => Cmult[27]) = "";
		(Bmult[12] => Cmult[27]) = "";
		(Bmult[13] => Cmult[27]) = "";
		(Bmult[14] => Cmult[27]) = "";
		(Bmult[15] => Cmult[27]) = "";
		(Bmult[16] => Cmult[27]) = "";
		(Bmult[17] => Cmult[27]) = "";
		(Bmult[18] => Cmult[27]) = "";
		(Bmult[19] => Cmult[27]) = "";
		(Bmult[20] => Cmult[27]) = "";
		(Bmult[21] => Cmult[27]) = "";
		(Bmult[22] => Cmult[27]) = "";
		(Bmult[23] => Cmult[27]) = "";
		(Bmult[24] => Cmult[27]) = "";
		(Bmult[25] => Cmult[27]) = "";
		(Bmult[26] => Cmult[27]) = "";
		(Bmult[27] => Cmult[27]) = "";
		(Bmult[28] => Cmult[27]) = "";
		(Bmult[29] => Cmult[27]) = "";
		(Bmult[30] => Cmult[27]) = "";
		(Bmult[31] => Cmult[27]) = "";		
		(Valid_mult[0] => Cmult[27]) = "";
		(Valid_mult[1] => Cmult[27]) = "";
		(sel_mul_32x32 => Cmult[27]) = "";
		(Amult[0]  => Cmult[28]) = "";
		(Amult[1]  => Cmult[28]) = "";
		(Amult[2]  => Cmult[28]) = "";
		(Amult[3]  => Cmult[28]) = "";
		(Amult[4]  => Cmult[28]) = "";
		(Amult[5]  => Cmult[28]) = "";
		(Amult[6]  => Cmult[28]) = "";
		(Amult[7]  => Cmult[28]) = "";
		(Amult[8]  => Cmult[28]) = "";
		(Amult[9]  => Cmult[28]) = "";
		(Amult[10] => Cmult[28]) = "";
		(Amult[11] => Cmult[28]) = "";
		(Amult[12] => Cmult[28]) = "";
		(Amult[13] => Cmult[28]) = "";
		(Amult[14] => Cmult[28]) = "";
		(Amult[15] => Cmult[28]) = "";
		(Amult[16] => Cmult[28]) = "";
		(Amult[17] => Cmult[28]) = "";
		(Amult[18] => Cmult[28]) = "";
		(Amult[19] => Cmult[28]) = "";
		(Amult[20] => Cmult[28]) = "";
		(Amult[21] => Cmult[28]) = "";
		(Amult[22] => Cmult[28]) = "";
		(Amult[23] => Cmult[28]) = "";
		(Amult[24] => Cmult[28]) = "";
		(Amult[25] => Cmult[28]) = "";
		(Amult[26] => Cmult[28]) = "";
		(Amult[27] => Cmult[28]) = "";
		(Amult[28] => Cmult[28]) = "";
		(Amult[29] => Cmult[28]) = "";
		(Amult[30] => Cmult[28]) = "";
		(Amult[31] => Cmult[28]) = "";
		(Bmult[0]  => Cmult[28]) = "";
		(Bmult[1]  => Cmult[28]) = "";
		(Bmult[2]  => Cmult[28]) = "";
		(Bmult[3]  => Cmult[28]) = "";
		(Bmult[4]  => Cmult[28]) = "";
		(Bmult[5]  => Cmult[28]) = "";
		(Bmult[6]  => Cmult[28]) = "";
		(Bmult[7]  => Cmult[28]) = "";
		(Bmult[8]  => Cmult[28]) = "";
		(Bmult[9]  => Cmult[28]) = "";
		(Bmult[10] => Cmult[28]) = "";
		(Bmult[11] => Cmult[28]) = "";
		(Bmult[12] => Cmult[28]) = "";
		(Bmult[13] => Cmult[28]) = "";
		(Bmult[14] => Cmult[28]) = "";
		(Bmult[15] => Cmult[28]) = "";
		(Bmult[16] => Cmult[28]) = "";
		(Bmult[17] => Cmult[28]) = "";
		(Bmult[18] => Cmult[28]) = "";
		(Bmult[19] => Cmult[28]) = "";
		(Bmult[20] => Cmult[28]) = "";
		(Bmult[21] => Cmult[28]) = "";
		(Bmult[22] => Cmult[28]) = "";
		(Bmult[23] => Cmult[28]) = "";
		(Bmult[24] => Cmult[28]) = "";
		(Bmult[25] => Cmult[28]) = "";
		(Bmult[26] => Cmult[28]) = "";
		(Bmult[27] => Cmult[28]) = "";
		(Bmult[28] => Cmult[28]) = "";
		(Bmult[29] => Cmult[28]) = "";
		(Bmult[30] => Cmult[28]) = "";
		(Bmult[31] => Cmult[28]) = "";		
		(Valid_mult[0] => Cmult[28]) = "";
		(Valid_mult[1] => Cmult[28]) = "";
		(sel_mul_32x32 => Cmult[28]) = "";	
		(Amult[0]  => Cmult[29]) = "";
		(Amult[1]  => Cmult[29]) = "";
		(Amult[2]  => Cmult[29]) = "";
		(Amult[3]  => Cmult[29]) = "";
		(Amult[4]  => Cmult[29]) = "";
		(Amult[5]  => Cmult[29]) = "";
		(Amult[6]  => Cmult[29]) = "";
		(Amult[7]  => Cmult[29]) = "";
		(Amult[8]  => Cmult[29]) = "";
		(Amult[9]  => Cmult[29]) = "";
		(Amult[10] => Cmult[29]) = "";
		(Amult[11] => Cmult[29]) = "";
		(Amult[12] => Cmult[29]) = "";
		(Amult[13] => Cmult[29]) = "";
		(Amult[14] => Cmult[29]) = "";
		(Amult[15] => Cmult[29]) = "";
		(Amult[16] => Cmult[29]) = "";
		(Amult[17] => Cmult[29]) = "";
		(Amult[18] => Cmult[29]) = "";
		(Amult[19] => Cmult[29]) = "";
		(Amult[20] => Cmult[29]) = "";
		(Amult[21] => Cmult[29]) = "";
		(Amult[22] => Cmult[29]) = "";
		(Amult[23] => Cmult[29]) = "";
		(Amult[24] => Cmult[29]) = "";
		(Amult[25] => Cmult[29]) = "";
		(Amult[26] => Cmult[29]) = "";
		(Amult[27] => Cmult[29]) = "";
		(Amult[28] => Cmult[29]) = "";
		(Amult[29] => Cmult[29]) = "";
		(Amult[30] => Cmult[29]) = "";
		(Amult[31] => Cmult[29]) = "";
		(Bmult[0]  => Cmult[29]) = "";
		(Bmult[1]  => Cmult[29]) = "";
		(Bmult[2]  => Cmult[29]) = "";
		(Bmult[3]  => Cmult[29]) = "";
		(Bmult[4]  => Cmult[29]) = "";
		(Bmult[5]  => Cmult[29]) = "";
		(Bmult[6]  => Cmult[29]) = "";
		(Bmult[7]  => Cmult[29]) = "";
		(Bmult[8]  => Cmult[29]) = "";
		(Bmult[9]  => Cmult[29]) = "";
		(Bmult[10] => Cmult[29]) = "";
		(Bmult[11] => Cmult[29]) = "";
		(Bmult[12] => Cmult[29]) = "";
		(Bmult[13] => Cmult[29]) = "";
		(Bmult[14] => Cmult[29]) = "";
		(Bmult[15] => Cmult[29]) = "";
		(Bmult[16] => Cmult[29]) = "";
		(Bmult[17] => Cmult[29]) = "";
		(Bmult[18] => Cmult[29]) = "";
		(Bmult[19] => Cmult[29]) = "";
		(Bmult[20] => Cmult[29]) = "";
		(Bmult[21] => Cmult[29]) = "";
		(Bmult[22] => Cmult[29]) = "";
		(Bmult[23] => Cmult[29]) = "";
		(Bmult[24] => Cmult[29]) = "";
		(Bmult[25] => Cmult[29]) = "";
		(Bmult[26] => Cmult[29]) = "";
		(Bmult[27] => Cmult[29]) = "";
		(Bmult[28] => Cmult[29]) = "";
		(Bmult[29] => Cmult[29]) = "";
		(Bmult[30] => Cmult[29]) = "";
		(Bmult[31] => Cmult[29]) = "";		
		(Valid_mult[0] => Cmult[29]) = "";
		(Valid_mult[1] => Cmult[29]) = "";
		(sel_mul_32x32 => Cmult[29]) = "";	
		(Amult[0]  => Cmult[30]) = "";
		(Amult[1]  => Cmult[30]) = "";
		(Amult[2]  => Cmult[30]) = "";
		(Amult[3]  => Cmult[30]) = "";
		(Amult[4]  => Cmult[30]) = "";
		(Amult[5]  => Cmult[30]) = "";
		(Amult[6]  => Cmult[30]) = "";
		(Amult[7]  => Cmult[30]) = "";
		(Amult[8]  => Cmult[30]) = "";
		(Amult[9]  => Cmult[30]) = "";
		(Amult[10] => Cmult[30]) = "";
		(Amult[11] => Cmult[30]) = "";
		(Amult[12] => Cmult[30]) = "";
		(Amult[13] => Cmult[30]) = "";
		(Amult[14] => Cmult[30]) = "";
		(Amult[15] => Cmult[30]) = "";
		(Amult[16] => Cmult[30]) = "";
		(Amult[17] => Cmult[30]) = "";
		(Amult[18] => Cmult[30]) = "";
		(Amult[19] => Cmult[30]) = "";
		(Amult[20] => Cmult[30]) = "";
		(Amult[21] => Cmult[30]) = "";
		(Amult[22] => Cmult[30]) = "";
		(Amult[23] => Cmult[30]) = "";
		(Amult[24] => Cmult[30]) = "";
		(Amult[25] => Cmult[30]) = "";
		(Amult[26] => Cmult[30]) = "";
		(Amult[27] => Cmult[30]) = "";
		(Amult[28] => Cmult[30]) = "";
		(Amult[29] => Cmult[30]) = "";
		(Amult[30] => Cmult[30]) = "";
		(Amult[31] => Cmult[30]) = "";
		(Bmult[0]  => Cmult[30]) = "";
		(Bmult[1]  => Cmult[30]) = "";
		(Bmult[2]  => Cmult[30]) = "";
		(Bmult[3]  => Cmult[30]) = "";
		(Bmult[4]  => Cmult[30]) = "";
		(Bmult[5]  => Cmult[30]) = "";
		(Bmult[6]  => Cmult[30]) = "";
		(Bmult[7]  => Cmult[30]) = "";
		(Bmult[8]  => Cmult[30]) = "";
		(Bmult[9]  => Cmult[30]) = "";
		(Bmult[10] => Cmult[30]) = "";
		(Bmult[11] => Cmult[30]) = "";
		(Bmult[12] => Cmult[30]) = "";
		(Bmult[13] => Cmult[30]) = "";
		(Bmult[14] => Cmult[30]) = "";
		(Bmult[15] => Cmult[30]) = "";
		(Bmult[16] => Cmult[30]) = "";
		(Bmult[17] => Cmult[30]) = "";
		(Bmult[18] => Cmult[30]) = "";
		(Bmult[19] => Cmult[30]) = "";
		(Bmult[20] => Cmult[30]) = "";
		(Bmult[21] => Cmult[30]) = "";
		(Bmult[22] => Cmult[30]) = "";
		(Bmult[23] => Cmult[30]) = "";
		(Bmult[24] => Cmult[30]) = "";
		(Bmult[25] => Cmult[30]) = "";
		(Bmult[26] => Cmult[30]) = "";
		(Bmult[27] => Cmult[30]) = "";
		(Bmult[28] => Cmult[30]) = "";
		(Bmult[29] => Cmult[30]) = "";
		(Bmult[30] => Cmult[30]) = "";
		(Bmult[31] => Cmult[30]) = "";		
		(Valid_mult[0] => Cmult[30]) = "";
		(Valid_mult[1] => Cmult[30]) = "";
		(sel_mul_32x32 => Cmult[30]) = "";
		(Amult[0]  => Cmult[31]) = "";
		(Amult[1]  => Cmult[31]) = "";
		(Amult[2]  => Cmult[31]) = "";
		(Amult[3]  => Cmult[31]) = "";
		(Amult[4]  => Cmult[31]) = "";
		(Amult[5]  => Cmult[31]) = "";
		(Amult[6]  => Cmult[31]) = "";
		(Amult[7]  => Cmult[31]) = "";
		(Amult[8]  => Cmult[31]) = "";
		(Amult[9]  => Cmult[31]) = "";
		(Amult[10] => Cmult[31]) = "";
		(Amult[11] => Cmult[31]) = "";
		(Amult[12] => Cmult[31]) = "";
		(Amult[13] => Cmult[31]) = "";
		(Amult[14] => Cmult[31]) = "";
		(Amult[15] => Cmult[31]) = "";
		(Amult[16] => Cmult[31]) = "";
		(Amult[17] => Cmult[31]) = "";
		(Amult[18] => Cmult[31]) = "";
		(Amult[19] => Cmult[31]) = "";
		(Amult[20] => Cmult[31]) = "";
		(Amult[21] => Cmult[31]) = "";
		(Amult[22] => Cmult[31]) = "";
		(Amult[23] => Cmult[31]) = "";
		(Amult[24] => Cmult[31]) = "";
		(Amult[25] => Cmult[31]) = "";
		(Amult[26] => Cmult[31]) = "";
		(Amult[27] => Cmult[31]) = "";
		(Amult[28] => Cmult[31]) = "";
		(Amult[29] => Cmult[31]) = "";
		(Amult[30] => Cmult[31]) = "";
		(Amult[31] => Cmult[31]) = "";
		(Bmult[0]  => Cmult[31]) = "";
		(Bmult[1]  => Cmult[31]) = "";
		(Bmult[2]  => Cmult[31]) = "";
		(Bmult[3]  => Cmult[31]) = "";
		(Bmult[4]  => Cmult[31]) = "";
		(Bmult[5]  => Cmult[31]) = "";
		(Bmult[6]  => Cmult[31]) = "";
		(Bmult[7]  => Cmult[31]) = "";
		(Bmult[8]  => Cmult[31]) = "";
		(Bmult[9]  => Cmult[31]) = "";
		(Bmult[10] => Cmult[31]) = "";
		(Bmult[11] => Cmult[31]) = "";
		(Bmult[12] => Cmult[31]) = "";
		(Bmult[13] => Cmult[31]) = "";
		(Bmult[14] => Cmult[31]) = "";
		(Bmult[15] => Cmult[31]) = "";
		(Bmult[16] => Cmult[31]) = "";
		(Bmult[17] => Cmult[31]) = "";
		(Bmult[18] => Cmult[31]) = "";
		(Bmult[19] => Cmult[31]) = "";
		(Bmult[20] => Cmult[31]) = "";
		(Bmult[21] => Cmult[31]) = "";
		(Bmult[22] => Cmult[31]) = "";
		(Bmult[23] => Cmult[31]) = "";
		(Bmult[24] => Cmult[31]) = "";
		(Bmult[25] => Cmult[31]) = "";
		(Bmult[26] => Cmult[31]) = "";
		(Bmult[27] => Cmult[31]) = "";
		(Bmult[28] => Cmult[31]) = "";
		(Bmult[29] => Cmult[31]) = "";
		(Bmult[30] => Cmult[31]) = "";
		(Bmult[31] => Cmult[31]) = "";		
		(Valid_mult[0] => Cmult[31]) = "";
		(Valid_mult[1] => Cmult[31]) = "";
		(sel_mul_32x32 => Cmult[31]) = "";
		(Amult[0]  => Cmult[32]) = "";
		(Amult[1]  => Cmult[32]) = "";
		(Amult[2]  => Cmult[32]) = "";
		(Amult[3]  => Cmult[32]) = "";
		(Amult[4]  => Cmult[32]) = "";
		(Amult[5]  => Cmult[32]) = "";
		(Amult[6]  => Cmult[32]) = "";
		(Amult[7]  => Cmult[32]) = "";
		(Amult[8]  => Cmult[32]) = "";
		(Amult[9]  => Cmult[32]) = "";
		(Amult[10] => Cmult[32]) = "";
		(Amult[11] => Cmult[32]) = "";
		(Amult[12] => Cmult[32]) = "";
		(Amult[13] => Cmult[32]) = "";
		(Amult[14] => Cmult[32]) = "";
		(Amult[15] => Cmult[32]) = "";
		(Amult[16] => Cmult[32]) = "";
		(Amult[17] => Cmult[32]) = "";
		(Amult[18] => Cmult[32]) = "";
		(Amult[19] => Cmult[32]) = "";
		(Amult[20] => Cmult[32]) = "";
		(Amult[21] => Cmult[32]) = "";
		(Amult[22] => Cmult[32]) = "";
		(Amult[23] => Cmult[32]) = "";
		(Amult[24] => Cmult[32]) = "";
		(Amult[25] => Cmult[32]) = "";
		(Amult[26] => Cmult[32]) = "";
		(Amult[27] => Cmult[32]) = "";
		(Amult[28] => Cmult[32]) = "";
		(Amult[29] => Cmult[32]) = "";
		(Amult[30] => Cmult[32]) = "";
		(Amult[31] => Cmult[32]) = "";
		(Bmult[0]  => Cmult[32]) = "";
		(Bmult[1]  => Cmult[32]) = "";
		(Bmult[2]  => Cmult[32]) = "";
		(Bmult[3]  => Cmult[32]) = "";
		(Bmult[4]  => Cmult[32]) = "";
		(Bmult[5]  => Cmult[32]) = "";
		(Bmult[6]  => Cmult[32]) = "";
		(Bmult[7]  => Cmult[32]) = "";
		(Bmult[8]  => Cmult[32]) = "";
		(Bmult[9]  => Cmult[32]) = "";
		(Bmult[10] => Cmult[32]) = "";
		(Bmult[11] => Cmult[32]) = "";
		(Bmult[12] => Cmult[32]) = "";
		(Bmult[13] => Cmult[32]) = "";
		(Bmult[14] => Cmult[32]) = "";
		(Bmult[15] => Cmult[32]) = "";
		(Bmult[16] => Cmult[32]) = "";
		(Bmult[17] => Cmult[32]) = "";
		(Bmult[18] => Cmult[32]) = "";
		(Bmult[19] => Cmult[32]) = "";
		(Bmult[20] => Cmult[32]) = "";
		(Bmult[21] => Cmult[32]) = "";
		(Bmult[22] => Cmult[32]) = "";
		(Bmult[23] => Cmult[32]) = "";
		(Bmult[24] => Cmult[32]) = "";
		(Bmult[25] => Cmult[32]) = "";
		(Bmult[26] => Cmult[32]) = "";
		(Bmult[27] => Cmult[32]) = "";
		(Bmult[28] => Cmult[32]) = "";
		(Bmult[29] => Cmult[32]) = "";
		(Bmult[30] => Cmult[32]) = "";
		(Bmult[31] => Cmult[32]) = "";		
		(Valid_mult[0] => Cmult[32]) = "";
		(Valid_mult[1] => Cmult[32]) = "";
		(sel_mul_32x32 => Cmult[32]) = "";
		(Amult[0]  => Cmult[33]) = "";
		(Amult[1]  => Cmult[33]) = "";
		(Amult[2]  => Cmult[33]) = "";
		(Amult[3]  => Cmult[33]) = "";
		(Amult[4]  => Cmult[33]) = "";
		(Amult[5]  => Cmult[33]) = "";
		(Amult[6]  => Cmult[33]) = "";
		(Amult[7]  => Cmult[33]) = "";
		(Amult[8]  => Cmult[33]) = "";
		(Amult[9]  => Cmult[33]) = "";
		(Amult[10] => Cmult[33]) = "";
		(Amult[11] => Cmult[33]) = "";
		(Amult[12] => Cmult[33]) = "";
		(Amult[13] => Cmult[33]) = "";
		(Amult[14] => Cmult[33]) = "";
		(Amult[15] => Cmult[33]) = "";
		(Amult[16] => Cmult[33]) = "";
		(Amult[17] => Cmult[33]) = "";
		(Amult[18] => Cmult[33]) = "";
		(Amult[19] => Cmult[33]) = "";
		(Amult[20] => Cmult[33]) = "";
		(Amult[21] => Cmult[33]) = "";
		(Amult[22] => Cmult[33]) = "";
		(Amult[23] => Cmult[33]) = "";
		(Amult[24] => Cmult[33]) = "";
		(Amult[25] => Cmult[33]) = "";
		(Amult[26] => Cmult[33]) = "";
		(Amult[27] => Cmult[33]) = "";
		(Amult[28] => Cmult[33]) = "";
		(Amult[29] => Cmult[33]) = "";
		(Amult[30] => Cmult[33]) = "";
		(Amult[31] => Cmult[33]) = "";
		(Bmult[0]  => Cmult[33]) = "";
		(Bmult[1]  => Cmult[33]) = "";
		(Bmult[2]  => Cmult[33]) = "";
		(Bmult[3]  => Cmult[33]) = "";
		(Bmult[4]  => Cmult[33]) = "";
		(Bmult[5]  => Cmult[33]) = "";
		(Bmult[6]  => Cmult[33]) = "";
		(Bmult[7]  => Cmult[33]) = "";
		(Bmult[8]  => Cmult[33]) = "";
		(Bmult[9]  => Cmult[33]) = "";
		(Bmult[10] => Cmult[33]) = "";
		(Bmult[11] => Cmult[33]) = "";
		(Bmult[12] => Cmult[33]) = "";
		(Bmult[13] => Cmult[33]) = "";
		(Bmult[14] => Cmult[33]) = "";
		(Bmult[15] => Cmult[33]) = "";
		(Bmult[16] => Cmult[33]) = "";
		(Bmult[17] => Cmult[33]) = "";
		(Bmult[18] => Cmult[33]) = "";
		(Bmult[19] => Cmult[33]) = "";
		(Bmult[20] => Cmult[33]) = "";
		(Bmult[21] => Cmult[33]) = "";
		(Bmult[22] => Cmult[33]) = "";
		(Bmult[23] => Cmult[33]) = "";
		(Bmult[24] => Cmult[33]) = "";
		(Bmult[25] => Cmult[33]) = "";
		(Bmult[26] => Cmult[33]) = "";
		(Bmult[27] => Cmult[33]) = "";
		(Bmult[28] => Cmult[33]) = "";
		(Bmult[29] => Cmult[33]) = "";
		(Bmult[30] => Cmult[33]) = "";
		(Bmult[31] => Cmult[33]) = "";		
		(Valid_mult[0] => Cmult[33]) = "";
		(Valid_mult[1] => Cmult[33]) = "";
		(sel_mul_32x32 => Cmult[33]) = "";
		(Amult[0]  => Cmult[34]) = "";
		(Amult[1]  => Cmult[34]) = "";
		(Amult[2]  => Cmult[34]) = "";
		(Amult[3]  => Cmult[34]) = "";
		(Amult[4]  => Cmult[34]) = "";
		(Amult[5]  => Cmult[34]) = "";
		(Amult[6]  => Cmult[34]) = "";
		(Amult[7]  => Cmult[34]) = "";
		(Amult[8]  => Cmult[34]) = "";
		(Amult[9]  => Cmult[34]) = "";
		(Amult[10] => Cmult[34]) = "";
		(Amult[11] => Cmult[34]) = "";
		(Amult[12] => Cmult[34]) = "";
		(Amult[13] => Cmult[34]) = "";
		(Amult[14] => Cmult[34]) = "";
		(Amult[15] => Cmult[34]) = "";
		(Amult[16] => Cmult[34]) = "";
		(Amult[17] => Cmult[34]) = "";
		(Amult[18] => Cmult[34]) = "";
		(Amult[19] => Cmult[34]) = "";
		(Amult[20] => Cmult[34]) = "";
		(Amult[21] => Cmult[34]) = "";
		(Amult[22] => Cmult[34]) = "";
		(Amult[23] => Cmult[34]) = "";
		(Amult[24] => Cmult[34]) = "";
		(Amult[25] => Cmult[34]) = "";
		(Amult[26] => Cmult[34]) = "";
		(Amult[27] => Cmult[34]) = "";
		(Amult[28] => Cmult[34]) = "";
		(Amult[29] => Cmult[34]) = "";
		(Amult[30] => Cmult[34]) = "";
		(Amult[31] => Cmult[34]) = "";
		(Bmult[0]  => Cmult[34]) = "";
		(Bmult[1]  => Cmult[34]) = "";
		(Bmult[2]  => Cmult[34]) = "";
		(Bmult[3]  => Cmult[34]) = "";
		(Bmult[4]  => Cmult[34]) = "";
		(Bmult[5]  => Cmult[34]) = "";
		(Bmult[6]  => Cmult[34]) = "";
		(Bmult[7]  => Cmult[34]) = "";
		(Bmult[8]  => Cmult[34]) = "";
		(Bmult[9]  => Cmult[34]) = "";
		(Bmult[10] => Cmult[34]) = "";
		(Bmult[11] => Cmult[34]) = "";
		(Bmult[12] => Cmult[34]) = "";
		(Bmult[13] => Cmult[34]) = "";
		(Bmult[14] => Cmult[34]) = "";
		(Bmult[15] => Cmult[34]) = "";
		(Bmult[16] => Cmult[34]) = "";
		(Bmult[17] => Cmult[34]) = "";
		(Bmult[18] => Cmult[34]) = "";
		(Bmult[19] => Cmult[34]) = "";
		(Bmult[20] => Cmult[34]) = "";
		(Bmult[21] => Cmult[34]) = "";
		(Bmult[22] => Cmult[34]) = "";
		(Bmult[23] => Cmult[34]) = "";
		(Bmult[24] => Cmult[34]) = "";
		(Bmult[25] => Cmult[34]) = "";
		(Bmult[26] => Cmult[34]) = "";
		(Bmult[27] => Cmult[34]) = "";
		(Bmult[28] => Cmult[34]) = "";
		(Bmult[29] => Cmult[34]) = "";
		(Bmult[30] => Cmult[34]) = "";
		(Bmult[31] => Cmult[34]) = "";		
		(Valid_mult[0] => Cmult[34]) = "";
		(Valid_mult[1] => Cmult[34]) = "";
		(sel_mul_32x32 => Cmult[34]) = "";
		(Amult[0]  => Cmult[35]) = "";
		(Amult[1]  => Cmult[35]) = "";
		(Amult[2]  => Cmult[35]) = "";
		(Amult[3]  => Cmult[35]) = "";
		(Amult[4]  => Cmult[35]) = "";
		(Amult[5]  => Cmult[35]) = "";
		(Amult[6]  => Cmult[35]) = "";
		(Amult[7]  => Cmult[35]) = "";
		(Amult[8]  => Cmult[35]) = "";
		(Amult[9]  => Cmult[35]) = "";
		(Amult[10] => Cmult[35]) = "";
		(Amult[11] => Cmult[35]) = "";
		(Amult[12] => Cmult[35]) = "";
		(Amult[13] => Cmult[35]) = "";
		(Amult[14] => Cmult[35]) = "";
		(Amult[15] => Cmult[35]) = "";
		(Amult[16] => Cmult[35]) = "";
		(Amult[17] => Cmult[35]) = "";
		(Amult[18] => Cmult[35]) = "";
		(Amult[19] => Cmult[35]) = "";
		(Amult[20] => Cmult[35]) = "";
		(Amult[21] => Cmult[35]) = "";
		(Amult[22] => Cmult[35]) = "";
		(Amult[23] => Cmult[35]) = "";
		(Amult[24] => Cmult[35]) = "";
		(Amult[25] => Cmult[35]) = "";
		(Amult[26] => Cmult[35]) = "";
		(Amult[27] => Cmult[35]) = "";
		(Amult[28] => Cmult[35]) = "";
		(Amult[29] => Cmult[35]) = "";
		(Amult[30] => Cmult[35]) = "";
		(Amult[31] => Cmult[35]) = "";
		(Bmult[0]  => Cmult[35]) = "";
		(Bmult[1]  => Cmult[35]) = "";
		(Bmult[2]  => Cmult[35]) = "";
		(Bmult[3]  => Cmult[35]) = "";
		(Bmult[4]  => Cmult[35]) = "";
		(Bmult[5]  => Cmult[35]) = "";
		(Bmult[6]  => Cmult[35]) = "";
		(Bmult[7]  => Cmult[35]) = "";
		(Bmult[8]  => Cmult[35]) = "";
		(Bmult[9]  => Cmult[35]) = "";
		(Bmult[10] => Cmult[35]) = "";
		(Bmult[11] => Cmult[35]) = "";
		(Bmult[12] => Cmult[35]) = "";
		(Bmult[13] => Cmult[35]) = "";
		(Bmult[14] => Cmult[35]) = "";
		(Bmult[15] => Cmult[35]) = "";
		(Bmult[16] => Cmult[35]) = "";
		(Bmult[17] => Cmult[35]) = "";
		(Bmult[18] => Cmult[35]) = "";
		(Bmult[19] => Cmult[35]) = "";
		(Bmult[20] => Cmult[35]) = "";
		(Bmult[21] => Cmult[35]) = "";
		(Bmult[22] => Cmult[35]) = "";
		(Bmult[23] => Cmult[35]) = "";
		(Bmult[24] => Cmult[35]) = "";
		(Bmult[25] => Cmult[35]) = "";
		(Bmult[26] => Cmult[35]) = "";
		(Bmult[27] => Cmult[35]) = "";
		(Bmult[28] => Cmult[35]) = "";
		(Bmult[29] => Cmult[35]) = "";
		(Bmult[30] => Cmult[35]) = "";
		(Bmult[31] => Cmult[35]) = "";		
		(Valid_mult[0] => Cmult[35]) = "";
		(Valid_mult[1] => Cmult[35]) = "";
		(sel_mul_32x32 => Cmult[35]) = "";
		(Amult[0]  => Cmult[36]) = "";
		(Amult[1]  => Cmult[36]) = "";
		(Amult[2]  => Cmult[36]) = "";
		(Amult[3]  => Cmult[36]) = "";
		(Amult[4]  => Cmult[36]) = "";
		(Amult[5]  => Cmult[36]) = "";
		(Amult[6]  => Cmult[36]) = "";
		(Amult[7]  => Cmult[36]) = "";
		(Amult[8]  => Cmult[36]) = "";
		(Amult[9]  => Cmult[36]) = "";
		(Amult[10] => Cmult[36]) = "";
		(Amult[11] => Cmult[36]) = "";
		(Amult[12] => Cmult[36]) = "";
		(Amult[13] => Cmult[36]) = "";
		(Amult[14] => Cmult[36]) = "";
		(Amult[15] => Cmult[36]) = "";
		(Amult[16] => Cmult[36]) = "";
		(Amult[17] => Cmult[36]) = "";
		(Amult[18] => Cmult[36]) = "";
		(Amult[19] => Cmult[36]) = "";
		(Amult[20] => Cmult[36]) = "";
		(Amult[21] => Cmult[36]) = "";
		(Amult[22] => Cmult[36]) = "";
		(Amult[23] => Cmult[36]) = "";
		(Amult[24] => Cmult[36]) = "";
		(Amult[25] => Cmult[36]) = "";
		(Amult[26] => Cmult[36]) = "";
		(Amult[27] => Cmult[36]) = "";
		(Amult[28] => Cmult[36]) = "";
		(Amult[29] => Cmult[36]) = "";
		(Amult[30] => Cmult[36]) = "";
		(Amult[31] => Cmult[36]) = "";
		(Bmult[0]  => Cmult[36]) = "";
		(Bmult[1]  => Cmult[36]) = "";
		(Bmult[2]  => Cmult[36]) = "";
		(Bmult[3]  => Cmult[36]) = "";
		(Bmult[4]  => Cmult[36]) = "";
		(Bmult[5]  => Cmult[36]) = "";
		(Bmult[6]  => Cmult[36]) = "";
		(Bmult[7]  => Cmult[36]) = "";
		(Bmult[8]  => Cmult[36]) = "";
		(Bmult[9]  => Cmult[36]) = "";
		(Bmult[10] => Cmult[36]) = "";
		(Bmult[11] => Cmult[36]) = "";
		(Bmult[12] => Cmult[36]) = "";
		(Bmult[13] => Cmult[36]) = "";
		(Bmult[14] => Cmult[36]) = "";
		(Bmult[15] => Cmult[36]) = "";
		(Bmult[16] => Cmult[36]) = "";
		(Bmult[17] => Cmult[36]) = "";
		(Bmult[18] => Cmult[36]) = "";
		(Bmult[19] => Cmult[36]) = "";
		(Bmult[20] => Cmult[36]) = "";
		(Bmult[21] => Cmult[36]) = "";
		(Bmult[22] => Cmult[36]) = "";
		(Bmult[23] => Cmult[36]) = "";
		(Bmult[24] => Cmult[36]) = "";
		(Bmult[25] => Cmult[36]) = "";
		(Bmult[26] => Cmult[36]) = "";
		(Bmult[27] => Cmult[36]) = "";
		(Bmult[28] => Cmult[36]) = "";
		(Bmult[29] => Cmult[36]) = "";
		(Bmult[30] => Cmult[36]) = "";
		(Bmult[31] => Cmult[36]) = "";		
		(Valid_mult[0] => Cmult[36]) = "";
		(Valid_mult[1] => Cmult[36]) = "";
		(sel_mul_32x32 => Cmult[36]) = "";
		(Amult[0]  => Cmult[37]) = "";
		(Amult[1]  => Cmult[37]) = "";
		(Amult[2]  => Cmult[37]) = "";
		(Amult[3]  => Cmult[37]) = "";
		(Amult[4]  => Cmult[37]) = "";
		(Amult[5]  => Cmult[37]) = "";
		(Amult[6]  => Cmult[37]) = "";
		(Amult[7]  => Cmult[37]) = "";
		(Amult[8]  => Cmult[37]) = "";
		(Amult[9]  => Cmult[37]) = "";
		(Amult[10] => Cmult[37]) = "";
		(Amult[11] => Cmult[37]) = "";
		(Amult[12] => Cmult[37]) = "";
		(Amult[13] => Cmult[37]) = "";
		(Amult[14] => Cmult[37]) = "";
		(Amult[15] => Cmult[37]) = "";
		(Amult[16] => Cmult[37]) = "";
		(Amult[17] => Cmult[37]) = "";
		(Amult[18] => Cmult[37]) = "";
		(Amult[19] => Cmult[37]) = "";
		(Amult[20] => Cmult[37]) = "";
		(Amult[21] => Cmult[37]) = "";
		(Amult[22] => Cmult[37]) = "";
		(Amult[23] => Cmult[37]) = "";
		(Amult[24] => Cmult[37]) = "";
		(Amult[25] => Cmult[37]) = "";
		(Amult[26] => Cmult[37]) = "";
		(Amult[27] => Cmult[37]) = "";
		(Amult[28] => Cmult[37]) = "";
		(Amult[29] => Cmult[37]) = "";
		(Amult[30] => Cmult[37]) = "";
		(Amult[31] => Cmult[37]) = "";
		(Bmult[0]  => Cmult[37]) = "";
		(Bmult[1]  => Cmult[37]) = "";
		(Bmult[2]  => Cmult[37]) = "";
		(Bmult[3]  => Cmult[37]) = "";
		(Bmult[4]  => Cmult[37]) = "";
		(Bmult[5]  => Cmult[37]) = "";
		(Bmult[6]  => Cmult[37]) = "";
		(Bmult[7]  => Cmult[37]) = "";
		(Bmult[8]  => Cmult[37]) = "";
		(Bmult[9]  => Cmult[37]) = "";
		(Bmult[10] => Cmult[37]) = "";
		(Bmult[11] => Cmult[37]) = "";
		(Bmult[12] => Cmult[37]) = "";
		(Bmult[13] => Cmult[37]) = "";
		(Bmult[14] => Cmult[37]) = "";
		(Bmult[15] => Cmult[37]) = "";
		(Bmult[16] => Cmult[37]) = "";
		(Bmult[17] => Cmult[37]) = "";
		(Bmult[18] => Cmult[37]) = "";
		(Bmult[19] => Cmult[37]) = "";
		(Bmult[20] => Cmult[37]) = "";
		(Bmult[21] => Cmult[37]) = "";
		(Bmult[22] => Cmult[37]) = "";
		(Bmult[23] => Cmult[37]) = "";
		(Bmult[24] => Cmult[37]) = "";
		(Bmult[25] => Cmult[37]) = "";
		(Bmult[26] => Cmult[37]) = "";
		(Bmult[27] => Cmult[37]) = "";
		(Bmult[28] => Cmult[37]) = "";
		(Bmult[29] => Cmult[37]) = "";
		(Bmult[30] => Cmult[37]) = "";
		(Bmult[31] => Cmult[37]) = "";		
		(Valid_mult[0] => Cmult[37]) = "";
		(Valid_mult[1] => Cmult[37]) = "";
		(sel_mul_32x32 => Cmult[37]) = "";
		(Amult[0]  => Cmult[38]) = "";
		(Amult[1]  => Cmult[38]) = "";
		(Amult[2]  => Cmult[38]) = "";
		(Amult[3]  => Cmult[38]) = "";
		(Amult[4]  => Cmult[38]) = "";
		(Amult[5]  => Cmult[38]) = "";
		(Amult[6]  => Cmult[38]) = "";
		(Amult[7]  => Cmult[38]) = "";
		(Amult[8]  => Cmult[38]) = "";
		(Amult[9]  => Cmult[38]) = "";
		(Amult[10] => Cmult[38]) = "";
		(Amult[11] => Cmult[38]) = "";
		(Amult[12] => Cmult[38]) = "";
		(Amult[13] => Cmult[38]) = "";
		(Amult[14] => Cmult[38]) = "";
		(Amult[15] => Cmult[38]) = "";
		(Amult[16] => Cmult[38]) = "";
		(Amult[17] => Cmult[38]) = "";
		(Amult[18] => Cmult[38]) = "";
		(Amult[19] => Cmult[38]) = "";
		(Amult[20] => Cmult[38]) = "";
		(Amult[21] => Cmult[38]) = "";
		(Amult[22] => Cmult[38]) = "";
		(Amult[23] => Cmult[38]) = "";
		(Amult[24] => Cmult[38]) = "";
		(Amult[25] => Cmult[38]) = "";
		(Amult[26] => Cmult[38]) = "";
		(Amult[27] => Cmult[38]) = "";
		(Amult[28] => Cmult[38]) = "";
		(Amult[29] => Cmult[38]) = "";
		(Amult[30] => Cmult[38]) = "";
		(Amult[31] => Cmult[38]) = "";
		(Bmult[0]  => Cmult[38]) = "";
		(Bmult[1]  => Cmult[38]) = "";
		(Bmult[2]  => Cmult[38]) = "";
		(Bmult[3]  => Cmult[38]) = "";
		(Bmult[4]  => Cmult[38]) = "";
		(Bmult[5]  => Cmult[38]) = "";
		(Bmult[6]  => Cmult[38]) = "";
		(Bmult[7]  => Cmult[38]) = "";
		(Bmult[8]  => Cmult[38]) = "";
		(Bmult[9]  => Cmult[38]) = "";
		(Bmult[10] => Cmult[38]) = "";
		(Bmult[11] => Cmult[38]) = "";
		(Bmult[12] => Cmult[38]) = "";
		(Bmult[13] => Cmult[38]) = "";
		(Bmult[14] => Cmult[38]) = "";
		(Bmult[15] => Cmult[38]) = "";
		(Bmult[16] => Cmult[38]) = "";
		(Bmult[17] => Cmult[38]) = "";
		(Bmult[18] => Cmult[38]) = "";
		(Bmult[19] => Cmult[38]) = "";
		(Bmult[20] => Cmult[38]) = "";
		(Bmult[21] => Cmult[38]) = "";
		(Bmult[22] => Cmult[38]) = "";
		(Bmult[23] => Cmult[38]) = "";
		(Bmult[24] => Cmult[38]) = "";
		(Bmult[25] => Cmult[38]) = "";
		(Bmult[26] => Cmult[38]) = "";
		(Bmult[27] => Cmult[38]) = "";
		(Bmult[28] => Cmult[38]) = "";
		(Bmult[29] => Cmult[38]) = "";
		(Bmult[30] => Cmult[38]) = "";
		(Bmult[31] => Cmult[38]) = "";		
		(Valid_mult[0] => Cmult[38]) = "";
		(Valid_mult[1] => Cmult[38]) = "";
		(sel_mul_32x32 => Cmult[38]) = "";	
		(Amult[0]  => Cmult[39]) = "";
		(Amult[1]  => Cmult[39]) = "";
		(Amult[2]  => Cmult[39]) = "";
		(Amult[3]  => Cmult[39]) = "";
		(Amult[4]  => Cmult[39]) = "";
		(Amult[5]  => Cmult[39]) = "";
		(Amult[6]  => Cmult[39]) = "";
		(Amult[7]  => Cmult[39]) = "";
		(Amult[8]  => Cmult[39]) = "";
		(Amult[9]  => Cmult[39]) = "";
		(Amult[10] => Cmult[39]) = "";
		(Amult[11] => Cmult[39]) = "";
		(Amult[12] => Cmult[39]) = "";
		(Amult[13] => Cmult[39]) = "";
		(Amult[14] => Cmult[39]) = "";
		(Amult[15] => Cmult[39]) = "";
		(Amult[16] => Cmult[39]) = "";
		(Amult[17] => Cmult[39]) = "";
		(Amult[18] => Cmult[39]) = "";
		(Amult[19] => Cmult[39]) = "";
		(Amult[20] => Cmult[39]) = "";
		(Amult[21] => Cmult[39]) = "";
		(Amult[22] => Cmult[39]) = "";
		(Amult[23] => Cmult[39]) = "";
		(Amult[24] => Cmult[39]) = "";
		(Amult[25] => Cmult[39]) = "";
		(Amult[26] => Cmult[39]) = "";
		(Amult[27] => Cmult[39]) = "";
		(Amult[28] => Cmult[39]) = "";
		(Amult[29] => Cmult[39]) = "";
		(Amult[30] => Cmult[39]) = "";
		(Amult[31] => Cmult[39]) = "";
		(Bmult[0]  => Cmult[39]) = "";
		(Bmult[1]  => Cmult[39]) = "";
		(Bmult[2]  => Cmult[39]) = "";
		(Bmult[3]  => Cmult[39]) = "";
		(Bmult[4]  => Cmult[39]) = "";
		(Bmult[5]  => Cmult[39]) = "";
		(Bmult[6]  => Cmult[39]) = "";
		(Bmult[7]  => Cmult[39]) = "";
		(Bmult[8]  => Cmult[39]) = "";
		(Bmult[9]  => Cmult[39]) = "";
		(Bmult[10] => Cmult[39]) = "";
		(Bmult[11] => Cmult[39]) = "";
		(Bmult[12] => Cmult[39]) = "";
		(Bmult[13] => Cmult[39]) = "";
		(Bmult[14] => Cmult[39]) = "";
		(Bmult[15] => Cmult[39]) = "";
		(Bmult[16] => Cmult[39]) = "";
		(Bmult[17] => Cmult[39]) = "";
		(Bmult[18] => Cmult[39]) = "";
		(Bmult[19] => Cmult[39]) = "";
		(Bmult[20] => Cmult[39]) = "";
		(Bmult[21] => Cmult[39]) = "";
		(Bmult[22] => Cmult[39]) = "";
		(Bmult[23] => Cmult[39]) = "";
		(Bmult[24] => Cmult[39]) = "";
		(Bmult[25] => Cmult[39]) = "";
		(Bmult[26] => Cmult[39]) = "";
		(Bmult[27] => Cmult[39]) = "";
		(Bmult[28] => Cmult[39]) = "";
		(Bmult[29] => Cmult[39]) = "";
		(Bmult[30] => Cmult[39]) = "";
		(Bmult[31] => Cmult[39]) = "";		
		(Valid_mult[0] => Cmult[39]) = "";
		(Valid_mult[1] => Cmult[39]) = "";
		(sel_mul_32x32 => Cmult[39]) = "";
		(Amult[0]  => Cmult[40]) = "";
		(Amult[1]  => Cmult[40]) = "";
		(Amult[2]  => Cmult[40]) = "";
		(Amult[3]  => Cmult[40]) = "";
		(Amult[4]  => Cmult[40]) = "";
		(Amult[5]  => Cmult[40]) = "";
		(Amult[6]  => Cmult[40]) = "";
		(Amult[7]  => Cmult[40]) = "";
		(Amult[8]  => Cmult[40]) = "";
		(Amult[9]  => Cmult[40]) = "";
		(Amult[10] => Cmult[40]) = "";
		(Amult[11] => Cmult[40]) = "";
		(Amult[12] => Cmult[40]) = "";
		(Amult[13] => Cmult[40]) = "";
		(Amult[14] => Cmult[40]) = "";
		(Amult[15] => Cmult[40]) = "";
		(Amult[16] => Cmult[40]) = "";
		(Amult[17] => Cmult[40]) = "";
		(Amult[18] => Cmult[40]) = "";
		(Amult[19] => Cmult[40]) = "";
		(Amult[20] => Cmult[40]) = "";
		(Amult[21] => Cmult[40]) = "";
		(Amult[22] => Cmult[40]) = "";
		(Amult[23] => Cmult[40]) = "";
		(Amult[24] => Cmult[40]) = "";
		(Amult[25] => Cmult[40]) = "";
		(Amult[26] => Cmult[40]) = "";
		(Amult[27] => Cmult[40]) = "";
		(Amult[28] => Cmult[40]) = "";
		(Amult[29] => Cmult[40]) = "";
		(Amult[30] => Cmult[40]) = "";
		(Amult[31] => Cmult[40]) = "";
		(Bmult[0]  => Cmult[40]) = "";
		(Bmult[1]  => Cmult[40]) = "";
		(Bmult[2]  => Cmult[40]) = "";
		(Bmult[3]  => Cmult[40]) = "";
		(Bmult[4]  => Cmult[40]) = "";
		(Bmult[5]  => Cmult[40]) = "";
		(Bmult[6]  => Cmult[40]) = "";
		(Bmult[7]  => Cmult[40]) = "";
		(Bmult[8]  => Cmult[40]) = "";
		(Bmult[9]  => Cmult[40]) = "";
		(Bmult[10] => Cmult[40]) = "";
		(Bmult[11] => Cmult[40]) = "";
		(Bmult[12] => Cmult[40]) = "";
		(Bmult[13] => Cmult[40]) = "";
		(Bmult[14] => Cmult[40]) = "";
		(Bmult[15] => Cmult[40]) = "";
		(Bmult[16] => Cmult[40]) = "";
		(Bmult[17] => Cmult[40]) = "";
		(Bmult[18] => Cmult[40]) = "";
		(Bmult[19] => Cmult[40]) = "";
		(Bmult[20] => Cmult[40]) = "";
		(Bmult[21] => Cmult[40]) = "";
		(Bmult[22] => Cmult[40]) = "";
		(Bmult[23] => Cmult[40]) = "";
		(Bmult[24] => Cmult[40]) = "";
		(Bmult[25] => Cmult[40]) = "";
		(Bmult[26] => Cmult[40]) = "";
		(Bmult[27] => Cmult[40]) = "";
		(Bmult[28] => Cmult[40]) = "";
		(Bmult[29] => Cmult[40]) = "";
		(Bmult[30] => Cmult[40]) = "";
		(Bmult[31] => Cmult[40]) = "";		
		(Valid_mult[0] => Cmult[40]) = "";
		(Valid_mult[1] => Cmult[40]) = "";
		(sel_mul_32x32 => Cmult[40]) = "";
		(Amult[0]  => Cmult[41]) = "";
		(Amult[1]  => Cmult[41]) = "";
		(Amult[2]  => Cmult[41]) = "";
		(Amult[3]  => Cmult[41]) = "";
		(Amult[4]  => Cmult[41]) = "";
		(Amult[5]  => Cmult[41]) = "";
		(Amult[6]  => Cmult[41]) = "";
		(Amult[7]  => Cmult[41]) = "";
		(Amult[8]  => Cmult[41]) = "";
		(Amult[9]  => Cmult[41]) = "";
		(Amult[10] => Cmult[41]) = "";
		(Amult[11] => Cmult[41]) = "";
		(Amult[12] => Cmult[41]) = "";
		(Amult[13] => Cmult[41]) = "";
		(Amult[14] => Cmult[41]) = "";
		(Amult[15] => Cmult[41]) = "";
		(Amult[16] => Cmult[41]) = "";
		(Amult[17] => Cmult[41]) = "";
		(Amult[18] => Cmult[41]) = "";
		(Amult[19] => Cmult[41]) = "";
		(Amult[20] => Cmult[41]) = "";
		(Amult[21] => Cmult[41]) = "";
		(Amult[22] => Cmult[41]) = "";
		(Amult[23] => Cmult[41]) = "";
		(Amult[24] => Cmult[41]) = "";
		(Amult[25] => Cmult[41]) = "";
		(Amult[26] => Cmult[41]) = "";
		(Amult[27] => Cmult[41]) = "";
		(Amult[28] => Cmult[41]) = "";
		(Amult[29] => Cmult[41]) = "";
		(Amult[30] => Cmult[41]) = "";
		(Amult[31] => Cmult[41]) = "";
		(Bmult[0]  => Cmult[41]) = "";
		(Bmult[1]  => Cmult[41]) = "";
		(Bmult[2]  => Cmult[41]) = "";
		(Bmult[3]  => Cmult[41]) = "";
		(Bmult[4]  => Cmult[41]) = "";
		(Bmult[5]  => Cmult[41]) = "";
		(Bmult[6]  => Cmult[41]) = "";
		(Bmult[7]  => Cmult[41]) = "";
		(Bmult[8]  => Cmult[41]) = "";
		(Bmult[9]  => Cmult[41]) = "";
		(Bmult[10] => Cmult[41]) = "";
		(Bmult[11] => Cmult[41]) = "";
		(Bmult[12] => Cmult[41]) = "";
		(Bmult[13] => Cmult[41]) = "";
		(Bmult[14] => Cmult[41]) = "";
		(Bmult[15] => Cmult[41]) = "";
		(Bmult[16] => Cmult[41]) = "";
		(Bmult[17] => Cmult[41]) = "";
		(Bmult[18] => Cmult[41]) = "";
		(Bmult[19] => Cmult[41]) = "";
		(Bmult[20] => Cmult[41]) = "";
		(Bmult[21] => Cmult[41]) = "";
		(Bmult[22] => Cmult[41]) = "";
		(Bmult[23] => Cmult[41]) = "";
		(Bmult[24] => Cmult[41]) = "";
		(Bmult[25] => Cmult[41]) = "";
		(Bmult[26] => Cmult[41]) = "";
		(Bmult[27] => Cmult[41]) = "";
		(Bmult[28] => Cmult[41]) = "";
		(Bmult[29] => Cmult[41]) = "";
		(Bmult[30] => Cmult[41]) = "";
		(Bmult[31] => Cmult[41]) = "";		
		(Valid_mult[0] => Cmult[41]) = "";
		(Valid_mult[1] => Cmult[41]) = "";
		(sel_mul_32x32 => Cmult[41]) = "";
		(Amult[0]  => Cmult[42]) = "";
		(Amult[1]  => Cmult[42]) = "";
		(Amult[2]  => Cmult[42]) = "";
		(Amult[3]  => Cmult[42]) = "";
		(Amult[4]  => Cmult[42]) = "";
		(Amult[5]  => Cmult[42]) = "";
		(Amult[6]  => Cmult[42]) = "";
		(Amult[7]  => Cmult[42]) = "";
		(Amult[8]  => Cmult[42]) = "";
		(Amult[9]  => Cmult[42]) = "";
		(Amult[10] => Cmult[42]) = "";
		(Amult[11] => Cmult[42]) = "";
		(Amult[12] => Cmult[42]) = "";
		(Amult[13] => Cmult[42]) = "";
		(Amult[14] => Cmult[42]) = "";
		(Amult[15] => Cmult[42]) = "";
		(Amult[16] => Cmult[42]) = "";
		(Amult[17] => Cmult[42]) = "";
		(Amult[18] => Cmult[42]) = "";
		(Amult[19] => Cmult[42]) = "";
		(Amult[20] => Cmult[42]) = "";
		(Amult[21] => Cmult[42]) = "";
		(Amult[22] => Cmult[42]) = "";
		(Amult[23] => Cmult[42]) = "";
		(Amult[24] => Cmult[42]) = "";
		(Amult[25] => Cmult[42]) = "";
		(Amult[26] => Cmult[42]) = "";
		(Amult[27] => Cmult[42]) = "";
		(Amult[28] => Cmult[42]) = "";
		(Amult[29] => Cmult[42]) = "";
		(Amult[30] => Cmult[42]) = "";
		(Amult[31] => Cmult[42]) = "";
		(Bmult[0]  => Cmult[42]) = "";
		(Bmult[1]  => Cmult[42]) = "";
		(Bmult[2]  => Cmult[42]) = "";
		(Bmult[3]  => Cmult[42]) = "";
		(Bmult[4]  => Cmult[42]) = "";
		(Bmult[5]  => Cmult[42]) = "";
		(Bmult[6]  => Cmult[42]) = "";
		(Bmult[7]  => Cmult[42]) = "";
		(Bmult[8]  => Cmult[42]) = "";
		(Bmult[9]  => Cmult[42]) = "";
		(Bmult[10] => Cmult[42]) = "";
		(Bmult[11] => Cmult[42]) = "";
		(Bmult[12] => Cmult[42]) = "";
		(Bmult[13] => Cmult[42]) = "";
		(Bmult[14] => Cmult[42]) = "";
		(Bmult[15] => Cmult[42]) = "";
		(Bmult[16] => Cmult[42]) = "";
		(Bmult[17] => Cmult[42]) = "";
		(Bmult[18] => Cmult[42]) = "";
		(Bmult[19] => Cmult[42]) = "";
		(Bmult[20] => Cmult[42]) = "";
		(Bmult[21] => Cmult[42]) = "";
		(Bmult[22] => Cmult[42]) = "";
		(Bmult[23] => Cmult[42]) = "";
		(Bmult[24] => Cmult[42]) = "";
		(Bmult[25] => Cmult[42]) = "";
		(Bmult[26] => Cmult[42]) = "";
		(Bmult[27] => Cmult[42]) = "";
		(Bmult[28] => Cmult[42]) = "";
		(Bmult[29] => Cmult[42]) = "";
		(Bmult[30] => Cmult[42]) = "";
		(Bmult[31] => Cmult[42]) = "";		
		(Valid_mult[0] => Cmult[42]) = "";
		(Valid_mult[1] => Cmult[42]) = "";
		(sel_mul_32x32 => Cmult[42]) = "";
		(Amult[0]  => Cmult[43]) = "";
		(Amult[1]  => Cmult[43]) = "";
		(Amult[2]  => Cmult[43]) = "";
		(Amult[3]  => Cmult[43]) = "";
		(Amult[4]  => Cmult[43]) = "";
		(Amult[5]  => Cmult[43]) = "";
		(Amult[6]  => Cmult[43]) = "";
		(Amult[7]  => Cmult[43]) = "";
		(Amult[8]  => Cmult[43]) = "";
		(Amult[9]  => Cmult[43]) = "";
		(Amult[10] => Cmult[43]) = "";
		(Amult[11] => Cmult[43]) = "";
		(Amult[12] => Cmult[43]) = "";
		(Amult[13] => Cmult[43]) = "";
		(Amult[14] => Cmult[43]) = "";
		(Amult[15] => Cmult[43]) = "";
		(Amult[16] => Cmult[43]) = "";
		(Amult[17] => Cmult[43]) = "";
		(Amult[18] => Cmult[43]) = "";
		(Amult[19] => Cmult[43]) = "";
		(Amult[20] => Cmult[43]) = "";
		(Amult[21] => Cmult[43]) = "";
		(Amult[22] => Cmult[43]) = "";
		(Amult[23] => Cmult[43]) = "";
		(Amult[24] => Cmult[43]) = "";
		(Amult[25] => Cmult[43]) = "";
		(Amult[26] => Cmult[43]) = "";
		(Amult[27] => Cmult[43]) = "";
		(Amult[28] => Cmult[43]) = "";
		(Amult[29] => Cmult[43]) = "";
		(Amult[30] => Cmult[43]) = "";
		(Amult[31] => Cmult[43]) = "";
		(Bmult[0]  => Cmult[43]) = "";
		(Bmult[1]  => Cmult[43]) = "";
		(Bmult[2]  => Cmult[43]) = "";
		(Bmult[3]  => Cmult[43]) = "";
		(Bmult[4]  => Cmult[43]) = "";
		(Bmult[5]  => Cmult[43]) = "";
		(Bmult[6]  => Cmult[43]) = "";
		(Bmult[7]  => Cmult[43]) = "";
		(Bmult[8]  => Cmult[43]) = "";
		(Bmult[9]  => Cmult[43]) = "";
		(Bmult[10] => Cmult[43]) = "";
		(Bmult[11] => Cmult[43]) = "";
		(Bmult[12] => Cmult[43]) = "";
		(Bmult[13] => Cmult[43]) = "";
		(Bmult[14] => Cmult[43]) = "";
		(Bmult[15] => Cmult[43]) = "";
		(Bmult[16] => Cmult[43]) = "";
		(Bmult[17] => Cmult[43]) = "";
		(Bmult[18] => Cmult[43]) = "";
		(Bmult[19] => Cmult[43]) = "";
		(Bmult[20] => Cmult[43]) = "";
		(Bmult[21] => Cmult[43]) = "";
		(Bmult[22] => Cmult[43]) = "";
		(Bmult[23] => Cmult[43]) = "";
		(Bmult[24] => Cmult[43]) = "";
		(Bmult[25] => Cmult[43]) = "";
		(Bmult[26] => Cmult[43]) = "";
		(Bmult[27] => Cmult[43]) = "";
		(Bmult[28] => Cmult[43]) = "";
		(Bmult[29] => Cmult[43]) = "";
		(Bmult[30] => Cmult[43]) = "";
		(Bmult[31] => Cmult[43]) = "";		
		(Valid_mult[0] => Cmult[43]) = "";
		(Valid_mult[1] => Cmult[43]) = "";
		(sel_mul_32x32 => Cmult[43]) = "";
		(Amult[0]  => Cmult[44]) = "";
		(Amult[1]  => Cmult[44]) = "";
		(Amult[2]  => Cmult[44]) = "";
		(Amult[3]  => Cmult[44]) = "";
		(Amult[4]  => Cmult[44]) = "";
		(Amult[5]  => Cmult[44]) = "";
		(Amult[6]  => Cmult[44]) = "";
		(Amult[7]  => Cmult[44]) = "";
		(Amult[8]  => Cmult[44]) = "";
		(Amult[9]  => Cmult[44]) = "";
		(Amult[10] => Cmult[44]) = "";
		(Amult[11] => Cmult[44]) = "";
		(Amult[12] => Cmult[44]) = "";
		(Amult[13] => Cmult[44]) = "";
		(Amult[14] => Cmult[44]) = "";
		(Amult[15] => Cmult[44]) = "";
		(Amult[16] => Cmult[44]) = "";
		(Amult[17] => Cmult[44]) = "";
		(Amult[18] => Cmult[44]) = "";
		(Amult[19] => Cmult[44]) = "";
		(Amult[20] => Cmult[44]) = "";
		(Amult[21] => Cmult[44]) = "";
		(Amult[22] => Cmult[44]) = "";
		(Amult[23] => Cmult[44]) = "";
		(Amult[24] => Cmult[44]) = "";
		(Amult[25] => Cmult[44]) = "";
		(Amult[26] => Cmult[44]) = "";
		(Amult[27] => Cmult[44]) = "";
		(Amult[28] => Cmult[44]) = "";
		(Amult[29] => Cmult[44]) = "";
		(Amult[30] => Cmult[44]) = "";
		(Amult[31] => Cmult[44]) = "";
		(Bmult[0]  => Cmult[44]) = "";
		(Bmult[1]  => Cmult[44]) = "";
		(Bmult[2]  => Cmult[44]) = "";
		(Bmult[3]  => Cmult[44]) = "";
		(Bmult[4]  => Cmult[44]) = "";
		(Bmult[5]  => Cmult[44]) = "";
		(Bmult[6]  => Cmult[44]) = "";
		(Bmult[7]  => Cmult[44]) = "";
		(Bmult[8]  => Cmult[44]) = "";
		(Bmult[9]  => Cmult[44]) = "";
		(Bmult[10] => Cmult[44]) = "";
		(Bmult[11] => Cmult[44]) = "";
		(Bmult[12] => Cmult[44]) = "";
		(Bmult[13] => Cmult[44]) = "";
		(Bmult[14] => Cmult[44]) = "";
		(Bmult[15] => Cmult[44]) = "";
		(Bmult[16] => Cmult[44]) = "";
		(Bmult[17] => Cmult[44]) = "";
		(Bmult[18] => Cmult[44]) = "";
		(Bmult[19] => Cmult[44]) = "";
		(Bmult[20] => Cmult[44]) = "";
		(Bmult[21] => Cmult[44]) = "";
		(Bmult[22] => Cmult[44]) = "";
		(Bmult[23] => Cmult[44]) = "";
		(Bmult[24] => Cmult[44]) = "";
		(Bmult[25] => Cmult[44]) = "";
		(Bmult[26] => Cmult[44]) = "";
		(Bmult[27] => Cmult[44]) = "";
		(Bmult[28] => Cmult[44]) = "";
		(Bmult[29] => Cmult[44]) = "";
		(Bmult[30] => Cmult[44]) = "";
		(Bmult[31] => Cmult[44]) = "";		
		(Valid_mult[0] => Cmult[44]) = "";
		(Valid_mult[1] => Cmult[44]) = "";
		(sel_mul_32x32 => Cmult[44]) = "";
		(Amult[0]  => Cmult[45]) = "";
		(Amult[1]  => Cmult[45]) = "";
		(Amult[2]  => Cmult[45]) = "";
		(Amult[3]  => Cmult[45]) = "";
		(Amult[4]  => Cmult[45]) = "";
		(Amult[5]  => Cmult[45]) = "";
		(Amult[6]  => Cmult[45]) = "";
		(Amult[7]  => Cmult[45]) = "";
		(Amult[8]  => Cmult[45]) = "";
		(Amult[9]  => Cmult[45]) = "";
		(Amult[10] => Cmult[45]) = "";
		(Amult[11] => Cmult[45]) = "";
		(Amult[12] => Cmult[45]) = "";
		(Amult[13] => Cmult[45]) = "";
		(Amult[14] => Cmult[45]) = "";
		(Amult[15] => Cmult[45]) = "";
		(Amult[16] => Cmult[45]) = "";
		(Amult[17] => Cmult[45]) = "";
		(Amult[18] => Cmult[45]) = "";
		(Amult[19] => Cmult[45]) = "";
		(Amult[20] => Cmult[45]) = "";
		(Amult[21] => Cmult[45]) = "";
		(Amult[22] => Cmult[45]) = "";
		(Amult[23] => Cmult[45]) = "";
		(Amult[24] => Cmult[45]) = "";
		(Amult[25] => Cmult[45]) = "";
		(Amult[26] => Cmult[45]) = "";
		(Amult[27] => Cmult[45]) = "";
		(Amult[28] => Cmult[45]) = "";
		(Amult[29] => Cmult[45]) = "";
		(Amult[30] => Cmult[45]) = "";
		(Amult[31] => Cmult[45]) = "";
		(Bmult[0]  => Cmult[45]) = "";
		(Bmult[1]  => Cmult[45]) = "";
		(Bmult[2]  => Cmult[45]) = "";
		(Bmult[3]  => Cmult[45]) = "";
		(Bmult[4]  => Cmult[45]) = "";
		(Bmult[5]  => Cmult[45]) = "";
		(Bmult[6]  => Cmult[45]) = "";
		(Bmult[7]  => Cmult[45]) = "";
		(Bmult[8]  => Cmult[45]) = "";
		(Bmult[9]  => Cmult[45]) = "";
		(Bmult[10] => Cmult[45]) = "";
		(Bmult[11] => Cmult[45]) = "";
		(Bmult[12] => Cmult[45]) = "";
		(Bmult[13] => Cmult[45]) = "";
		(Bmult[14] => Cmult[45]) = "";
		(Bmult[15] => Cmult[45]) = "";
		(Bmult[16] => Cmult[45]) = "";
		(Bmult[17] => Cmult[45]) = "";
		(Bmult[18] => Cmult[45]) = "";
		(Bmult[19] => Cmult[45]) = "";
		(Bmult[20] => Cmult[45]) = "";
		(Bmult[21] => Cmult[45]) = "";
		(Bmult[22] => Cmult[45]) = "";
		(Bmult[23] => Cmult[45]) = "";
		(Bmult[24] => Cmult[45]) = "";
		(Bmult[25] => Cmult[45]) = "";
		(Bmult[26] => Cmult[45]) = "";
		(Bmult[27] => Cmult[45]) = "";
		(Bmult[28] => Cmult[45]) = "";
		(Bmult[29] => Cmult[45]) = "";
		(Bmult[30] => Cmult[45]) = "";
		(Bmult[31] => Cmult[45]) = "";		
		(Valid_mult[0] => Cmult[45]) = "";
		(Valid_mult[1] => Cmult[45]) = "";
		(sel_mul_32x32 => Cmult[45]) = "";
		(Amult[0]  => Cmult[46]) = "";
		(Amult[1]  => Cmult[46]) = "";
		(Amult[2]  => Cmult[46]) = "";
		(Amult[3]  => Cmult[46]) = "";
		(Amult[4]  => Cmult[46]) = "";
		(Amult[5]  => Cmult[46]) = "";
		(Amult[6]  => Cmult[46]) = "";
		(Amult[7]  => Cmult[46]) = "";
		(Amult[8]  => Cmult[46]) = "";
		(Amult[9]  => Cmult[46]) = "";
		(Amult[10] => Cmult[46]) = "";
		(Amult[11] => Cmult[46]) = "";
		(Amult[12] => Cmult[46]) = "";
		(Amult[13] => Cmult[46]) = "";
		(Amult[14] => Cmult[46]) = "";
		(Amult[15] => Cmult[46]) = "";
		(Amult[16] => Cmult[46]) = "";
		(Amult[17] => Cmult[46]) = "";
		(Amult[18] => Cmult[46]) = "";
		(Amult[19] => Cmult[46]) = "";
		(Amult[20] => Cmult[46]) = "";
		(Amult[21] => Cmult[46]) = "";
		(Amult[22] => Cmult[46]) = "";
		(Amult[23] => Cmult[46]) = "";
		(Amult[24] => Cmult[46]) = "";
		(Amult[25] => Cmult[46]) = "";
		(Amult[26] => Cmult[46]) = "";
		(Amult[27] => Cmult[46]) = "";
		(Amult[28] => Cmult[46]) = "";
		(Amult[29] => Cmult[46]) = "";
		(Amult[30] => Cmult[46]) = "";
		(Amult[31] => Cmult[46]) = "";
		(Bmult[0]  => Cmult[46]) = "";
		(Bmult[1]  => Cmult[46]) = "";
		(Bmult[2]  => Cmult[46]) = "";
		(Bmult[3]  => Cmult[46]) = "";
		(Bmult[4]  => Cmult[46]) = "";
		(Bmult[5]  => Cmult[46]) = "";
		(Bmult[6]  => Cmult[46]) = "";
		(Bmult[7]  => Cmult[46]) = "";
		(Bmult[8]  => Cmult[46]) = "";
		(Bmult[9]  => Cmult[46]) = "";
		(Bmult[10] => Cmult[46]) = "";
		(Bmult[11] => Cmult[46]) = "";
		(Bmult[12] => Cmult[46]) = "";
		(Bmult[13] => Cmult[46]) = "";
		(Bmult[14] => Cmult[46]) = "";
		(Bmult[15] => Cmult[46]) = "";
		(Bmult[16] => Cmult[46]) = "";
		(Bmult[17] => Cmult[46]) = "";
		(Bmult[18] => Cmult[46]) = "";
		(Bmult[19] => Cmult[46]) = "";
		(Bmult[20] => Cmult[46]) = "";
		(Bmult[21] => Cmult[46]) = "";
		(Bmult[22] => Cmult[46]) = "";
		(Bmult[23] => Cmult[46]) = "";
		(Bmult[24] => Cmult[46]) = "";
		(Bmult[25] => Cmult[46]) = "";
		(Bmult[26] => Cmult[46]) = "";
		(Bmult[27] => Cmult[46]) = "";
		(Bmult[28] => Cmult[46]) = "";
		(Bmult[29] => Cmult[46]) = "";
		(Bmult[30] => Cmult[46]) = "";
		(Bmult[31] => Cmult[46]) = "";		
		(Valid_mult[0] => Cmult[46]) = "";
		(Valid_mult[1] => Cmult[46]) = "";
		(sel_mul_32x32 => Cmult[46]) = "";
		(Amult[0]  => Cmult[47]) = "";
		(Amult[1]  => Cmult[47]) = "";
		(Amult[2]  => Cmult[47]) = "";
		(Amult[3]  => Cmult[47]) = "";
		(Amult[4]  => Cmult[47]) = "";
		(Amult[5]  => Cmult[47]) = "";
		(Amult[6]  => Cmult[47]) = "";
		(Amult[7]  => Cmult[47]) = "";
		(Amult[8]  => Cmult[47]) = "";
		(Amult[9]  => Cmult[47]) = "";
		(Amult[10] => Cmult[47]) = "";
		(Amult[11] => Cmult[47]) = "";
		(Amult[12] => Cmult[47]) = "";
		(Amult[13] => Cmult[47]) = "";
		(Amult[14] => Cmult[47]) = "";
		(Amult[15] => Cmult[47]) = "";
		(Amult[16] => Cmult[47]) = "";
		(Amult[17] => Cmult[47]) = "";
		(Amult[18] => Cmult[47]) = "";
		(Amult[19] => Cmult[47]) = "";
		(Amult[20] => Cmult[47]) = "";
		(Amult[21] => Cmult[47]) = "";
		(Amult[22] => Cmult[47]) = "";
		(Amult[23] => Cmult[47]) = "";
		(Amult[24] => Cmult[47]) = "";
		(Amult[25] => Cmult[47]) = "";
		(Amult[26] => Cmult[47]) = "";
		(Amult[27] => Cmult[47]) = "";
		(Amult[28] => Cmult[47]) = "";
		(Amult[29] => Cmult[47]) = "";
		(Amult[30] => Cmult[47]) = "";
		(Amult[31] => Cmult[47]) = "";
		(Bmult[0]  => Cmult[47]) = "";
		(Bmult[1]  => Cmult[47]) = "";
		(Bmult[2]  => Cmult[47]) = "";
		(Bmult[3]  => Cmult[47]) = "";
		(Bmult[4]  => Cmult[47]) = "";
		(Bmult[5]  => Cmult[47]) = "";
		(Bmult[6]  => Cmult[47]) = "";
		(Bmult[7]  => Cmult[47]) = "";
		(Bmult[8]  => Cmult[47]) = "";
		(Bmult[9]  => Cmult[47]) = "";
		(Bmult[10] => Cmult[47]) = "";
		(Bmult[11] => Cmult[47]) = "";
		(Bmult[12] => Cmult[47]) = "";
		(Bmult[13] => Cmult[47]) = "";
		(Bmult[14] => Cmult[47]) = "";
		(Bmult[15] => Cmult[47]) = "";
		(Bmult[16] => Cmult[47]) = "";
		(Bmult[17] => Cmult[47]) = "";
		(Bmult[18] => Cmult[47]) = "";
		(Bmult[19] => Cmult[47]) = "";
		(Bmult[20] => Cmult[47]) = "";
		(Bmult[21] => Cmult[47]) = "";
		(Bmult[22] => Cmult[47]) = "";
		(Bmult[23] => Cmult[47]) = "";
		(Bmult[24] => Cmult[47]) = "";
		(Bmult[25] => Cmult[47]) = "";
		(Bmult[26] => Cmult[47]) = "";
		(Bmult[27] => Cmult[47]) = "";
		(Bmult[28] => Cmult[47]) = "";
		(Bmult[29] => Cmult[47]) = "";
		(Bmult[30] => Cmult[47]) = "";
		(Bmult[31] => Cmult[47]) = "";		
		(Valid_mult[0] => Cmult[47]) = "";
		(Valid_mult[1] => Cmult[47]) = "";
		(sel_mul_32x32 => Cmult[47]) = "";
		(Amult[0]  => Cmult[48]) = "";
		(Amult[1]  => Cmult[48]) = "";
		(Amult[2]  => Cmult[48]) = "";
		(Amult[3]  => Cmult[48]) = "";
		(Amult[4]  => Cmult[48]) = "";
		(Amult[5]  => Cmult[48]) = "";
		(Amult[6]  => Cmult[48]) = "";
		(Amult[7]  => Cmult[48]) = "";
		(Amult[8]  => Cmult[48]) = "";
		(Amult[9]  => Cmult[48]) = "";
		(Amult[10] => Cmult[48]) = "";
		(Amult[11] => Cmult[48]) = "";
		(Amult[12] => Cmult[48]) = "";
		(Amult[13] => Cmult[48]) = "";
		(Amult[14] => Cmult[48]) = "";
		(Amult[15] => Cmult[48]) = "";
		(Amult[16] => Cmult[48]) = "";
		(Amult[17] => Cmult[48]) = "";
		(Amult[18] => Cmult[48]) = "";
		(Amult[19] => Cmult[48]) = "";
		(Amult[20] => Cmult[48]) = "";
		(Amult[21] => Cmult[48]) = "";
		(Amult[22] => Cmult[48]) = "";
		(Amult[23] => Cmult[48]) = "";
		(Amult[24] => Cmult[48]) = "";
		(Amult[25] => Cmult[48]) = "";
		(Amult[26] => Cmult[48]) = "";
		(Amult[27] => Cmult[48]) = "";
		(Amult[28] => Cmult[48]) = "";
		(Amult[29] => Cmult[48]) = "";
		(Amult[30] => Cmult[48]) = "";
		(Amult[31] => Cmult[48]) = "";
		(Bmult[0]  => Cmult[48]) = "";
		(Bmult[1]  => Cmult[48]) = "";
		(Bmult[2]  => Cmult[48]) = "";
		(Bmult[3]  => Cmult[48]) = "";
		(Bmult[4]  => Cmult[48]) = "";
		(Bmult[5]  => Cmult[48]) = "";
		(Bmult[6]  => Cmult[48]) = "";
		(Bmult[7]  => Cmult[48]) = "";
		(Bmult[8]  => Cmult[48]) = "";
		(Bmult[9]  => Cmult[48]) = "";
		(Bmult[10] => Cmult[48]) = "";
		(Bmult[11] => Cmult[48]) = "";
		(Bmult[12] => Cmult[48]) = "";
		(Bmult[13] => Cmult[48]) = "";
		(Bmult[14] => Cmult[48]) = "";
		(Bmult[15] => Cmult[48]) = "";
		(Bmult[16] => Cmult[48]) = "";
		(Bmult[17] => Cmult[48]) = "";
		(Bmult[18] => Cmult[48]) = "";
		(Bmult[19] => Cmult[48]) = "";
		(Bmult[20] => Cmult[48]) = "";
		(Bmult[21] => Cmult[48]) = "";
		(Bmult[22] => Cmult[48]) = "";
		(Bmult[23] => Cmult[48]) = "";
		(Bmult[24] => Cmult[48]) = "";
		(Bmult[25] => Cmult[48]) = "";
		(Bmult[26] => Cmult[48]) = "";
		(Bmult[27] => Cmult[48]) = "";
		(Bmult[28] => Cmult[48]) = "";
		(Bmult[29] => Cmult[48]) = "";
		(Bmult[30] => Cmult[48]) = "";
		(Bmult[31] => Cmult[48]) = "";		
		(Valid_mult[0] => Cmult[48]) = "";
		(Valid_mult[1] => Cmult[48]) = "";
		(sel_mul_32x32 => Cmult[48]) = "";	
		(Amult[0]  => Cmult[49]) = "";
		(Amult[1]  => Cmult[49]) = "";
		(Amult[2]  => Cmult[49]) = "";
		(Amult[3]  => Cmult[49]) = "";
		(Amult[4]  => Cmult[49]) = "";
		(Amult[5]  => Cmult[49]) = "";
		(Amult[6]  => Cmult[49]) = "";
		(Amult[7]  => Cmult[49]) = "";
		(Amult[8]  => Cmult[49]) = "";
		(Amult[9]  => Cmult[49]) = "";
		(Amult[10] => Cmult[49]) = "";
		(Amult[11] => Cmult[49]) = "";
		(Amult[12] => Cmult[49]) = "";
		(Amult[13] => Cmult[49]) = "";
		(Amult[14] => Cmult[49]) = "";
		(Amult[15] => Cmult[49]) = "";
		(Amult[16] => Cmult[49]) = "";
		(Amult[17] => Cmult[49]) = "";
		(Amult[18] => Cmult[49]) = "";
		(Amult[19] => Cmult[49]) = "";
		(Amult[20] => Cmult[49]) = "";
		(Amult[21] => Cmult[49]) = "";
		(Amult[22] => Cmult[49]) = "";
		(Amult[23] => Cmult[49]) = "";
		(Amult[24] => Cmult[49]) = "";
		(Amult[25] => Cmult[49]) = "";
		(Amult[26] => Cmult[49]) = "";
		(Amult[27] => Cmult[49]) = "";
		(Amult[28] => Cmult[49]) = "";
		(Amult[29] => Cmult[49]) = "";
		(Amult[30] => Cmult[49]) = "";
		(Amult[31] => Cmult[49]) = "";
		(Bmult[0]  => Cmult[49]) = "";
		(Bmult[1]  => Cmult[49]) = "";
		(Bmult[2]  => Cmult[49]) = "";
		(Bmult[3]  => Cmult[49]) = "";
		(Bmult[4]  => Cmult[49]) = "";
		(Bmult[5]  => Cmult[49]) = "";
		(Bmult[6]  => Cmult[49]) = "";
		(Bmult[7]  => Cmult[49]) = "";
		(Bmult[8]  => Cmult[49]) = "";
		(Bmult[9]  => Cmult[49]) = "";
		(Bmult[10] => Cmult[49]) = "";
		(Bmult[11] => Cmult[49]) = "";
		(Bmult[12] => Cmult[49]) = "";
		(Bmult[13] => Cmult[49]) = "";
		(Bmult[14] => Cmult[49]) = "";
		(Bmult[15] => Cmult[49]) = "";
		(Bmult[16] => Cmult[49]) = "";
		(Bmult[17] => Cmult[49]) = "";
		(Bmult[18] => Cmult[49]) = "";
		(Bmult[19] => Cmult[49]) = "";
		(Bmult[20] => Cmult[49]) = "";
		(Bmult[21] => Cmult[49]) = "";
		(Bmult[22] => Cmult[49]) = "";
		(Bmult[23] => Cmult[49]) = "";
		(Bmult[24] => Cmult[49]) = "";
		(Bmult[25] => Cmult[49]) = "";
		(Bmult[26] => Cmult[49]) = "";
		(Bmult[27] => Cmult[49]) = "";
		(Bmult[28] => Cmult[49]) = "";
		(Bmult[29] => Cmult[49]) = "";
		(Bmult[30] => Cmult[49]) = "";
		(Bmult[31] => Cmult[49]) = "";		
		(Valid_mult[0] => Cmult[49]) = "";
		(Valid_mult[1] => Cmult[49]) = "";
		(sel_mul_32x32 => Cmult[49]) = "";
		(Amult[0]  => Cmult[50]) = "";
		(Amult[1]  => Cmult[50]) = "";
		(Amult[2]  => Cmult[50]) = "";
		(Amult[3]  => Cmult[50]) = "";
		(Amult[4]  => Cmult[50]) = "";
		(Amult[5]  => Cmult[50]) = "";
		(Amult[6]  => Cmult[50]) = "";
		(Amult[7]  => Cmult[50]) = "";
		(Amult[8]  => Cmult[50]) = "";
		(Amult[9]  => Cmult[50]) = "";
		(Amult[10] => Cmult[50]) = "";
		(Amult[11] => Cmult[50]) = "";
		(Amult[12] => Cmult[50]) = "";
		(Amult[13] => Cmult[50]) = "";
		(Amult[14] => Cmult[50]) = "";
		(Amult[15] => Cmult[50]) = "";
		(Amult[16] => Cmult[50]) = "";
		(Amult[17] => Cmult[50]) = "";
		(Amult[18] => Cmult[50]) = "";
		(Amult[19] => Cmult[50]) = "";
		(Amult[20] => Cmult[50]) = "";
		(Amult[21] => Cmult[50]) = "";
		(Amult[22] => Cmult[50]) = "";
		(Amult[23] => Cmult[50]) = "";
		(Amult[24] => Cmult[50]) = "";
		(Amult[25] => Cmult[50]) = "";
		(Amult[26] => Cmult[50]) = "";
		(Amult[27] => Cmult[50]) = "";
		(Amult[28] => Cmult[50]) = "";
		(Amult[29] => Cmult[50]) = "";
		(Amult[30] => Cmult[50]) = "";
		(Amult[31] => Cmult[50]) = "";
		(Bmult[0]  => Cmult[50]) = "";
		(Bmult[1]  => Cmult[50]) = "";
		(Bmult[2]  => Cmult[50]) = "";
		(Bmult[3]  => Cmult[50]) = "";
		(Bmult[4]  => Cmult[50]) = "";
		(Bmult[5]  => Cmult[50]) = "";
		(Bmult[6]  => Cmult[50]) = "";
		(Bmult[7]  => Cmult[50]) = "";
		(Bmult[8]  => Cmult[50]) = "";
		(Bmult[9]  => Cmult[50]) = "";
		(Bmult[10] => Cmult[50]) = "";
		(Bmult[11] => Cmult[50]) = "";
		(Bmult[12] => Cmult[50]) = "";
		(Bmult[13] => Cmult[50]) = "";
		(Bmult[14] => Cmult[50]) = "";
		(Bmult[15] => Cmult[50]) = "";
		(Bmult[16] => Cmult[50]) = "";
		(Bmult[17] => Cmult[50]) = "";
		(Bmult[18] => Cmult[50]) = "";
		(Bmult[19] => Cmult[50]) = "";
		(Bmult[20] => Cmult[50]) = "";
		(Bmult[21] => Cmult[50]) = "";
		(Bmult[22] => Cmult[50]) = "";
		(Bmult[23] => Cmult[50]) = "";
		(Bmult[24] => Cmult[50]) = "";
		(Bmult[25] => Cmult[50]) = "";
		(Bmult[26] => Cmult[50]) = "";
		(Bmult[27] => Cmult[50]) = "";
		(Bmult[28] => Cmult[50]) = "";
		(Bmult[29] => Cmult[50]) = "";
		(Bmult[30] => Cmult[50]) = "";
		(Bmult[31] => Cmult[50]) = "";		
		(Valid_mult[0] => Cmult[50]) = "";
		(Valid_mult[1] => Cmult[50]) = "";
		(sel_mul_32x32 => Cmult[50]) = "";
		(Amult[0]  => Cmult[51]) = "";
		(Amult[1]  => Cmult[51]) = "";
		(Amult[2]  => Cmult[51]) = "";
		(Amult[3]  => Cmult[51]) = "";
		(Amult[4]  => Cmult[51]) = "";
		(Amult[5]  => Cmult[51]) = "";
		(Amult[6]  => Cmult[51]) = "";
		(Amult[7]  => Cmult[51]) = "";
		(Amult[8]  => Cmult[51]) = "";
		(Amult[9]  => Cmult[51]) = "";
		(Amult[10] => Cmult[51]) = "";
		(Amult[11] => Cmult[51]) = "";
		(Amult[12] => Cmult[51]) = "";
		(Amult[13] => Cmult[51]) = "";
		(Amult[14] => Cmult[51]) = "";
		(Amult[15] => Cmult[51]) = "";
		(Amult[16] => Cmult[51]) = "";
		(Amult[17] => Cmult[51]) = "";
		(Amult[18] => Cmult[51]) = "";
		(Amult[19] => Cmult[51]) = "";
		(Amult[20] => Cmult[51]) = "";
		(Amult[21] => Cmult[51]) = "";
		(Amult[22] => Cmult[51]) = "";
		(Amult[23] => Cmult[51]) = "";
		(Amult[24] => Cmult[51]) = "";
		(Amult[25] => Cmult[51]) = "";
		(Amult[26] => Cmult[51]) = "";
		(Amult[27] => Cmult[51]) = "";
		(Amult[28] => Cmult[51]) = "";
		(Amult[29] => Cmult[51]) = "";
		(Amult[30] => Cmult[51]) = "";
		(Amult[31] => Cmult[51]) = "";
		(Bmult[0]  => Cmult[51]) = "";
		(Bmult[1]  => Cmult[51]) = "";
		(Bmult[2]  => Cmult[51]) = "";
		(Bmult[3]  => Cmult[51]) = "";
		(Bmult[4]  => Cmult[51]) = "";
		(Bmult[5]  => Cmult[51]) = "";
		(Bmult[6]  => Cmult[51]) = "";
		(Bmult[7]  => Cmult[51]) = "";
		(Bmult[8]  => Cmult[51]) = "";
		(Bmult[9]  => Cmult[51]) = "";
		(Bmult[10] => Cmult[51]) = "";
		(Bmult[11] => Cmult[51]) = "";
		(Bmult[12] => Cmult[51]) = "";
		(Bmult[13] => Cmult[51]) = "";
		(Bmult[14] => Cmult[51]) = "";
		(Bmult[15] => Cmult[51]) = "";
		(Bmult[16] => Cmult[51]) = "";
		(Bmult[17] => Cmult[51]) = "";
		(Bmult[18] => Cmult[51]) = "";
		(Bmult[19] => Cmult[51]) = "";
		(Bmult[20] => Cmult[51]) = "";
		(Bmult[21] => Cmult[51]) = "";
		(Bmult[22] => Cmult[51]) = "";
		(Bmult[23] => Cmult[51]) = "";
		(Bmult[24] => Cmult[51]) = "";
		(Bmult[25] => Cmult[51]) = "";
		(Bmult[26] => Cmult[51]) = "";
		(Bmult[27] => Cmult[51]) = "";
		(Bmult[28] => Cmult[51]) = "";
		(Bmult[29] => Cmult[51]) = "";
		(Bmult[30] => Cmult[51]) = "";
		(Bmult[31] => Cmult[51]) = "";		
		(Valid_mult[0] => Cmult[51]) = "";
		(Valid_mult[1] => Cmult[51]) = "";
		(sel_mul_32x32 => Cmult[51]) = "";
		(Amult[0]  => Cmult[52]) = "";
		(Amult[1]  => Cmult[52]) = "";
		(Amult[2]  => Cmult[52]) = "";
		(Amult[3]  => Cmult[52]) = "";
		(Amult[4]  => Cmult[52]) = "";
		(Amult[5]  => Cmult[52]) = "";
		(Amult[6]  => Cmult[52]) = "";
		(Amult[7]  => Cmult[52]) = "";
		(Amult[8]  => Cmult[52]) = "";
		(Amult[9]  => Cmult[52]) = "";
		(Amult[10] => Cmult[52]) = "";
		(Amult[11] => Cmult[52]) = "";
		(Amult[12] => Cmult[52]) = "";
		(Amult[13] => Cmult[52]) = "";
		(Amult[14] => Cmult[52]) = "";
		(Amult[15] => Cmult[52]) = "";
		(Amult[16] => Cmult[52]) = "";
		(Amult[17] => Cmult[52]) = "";
		(Amult[18] => Cmult[52]) = "";
		(Amult[19] => Cmult[52]) = "";
		(Amult[20] => Cmult[52]) = "";
		(Amult[21] => Cmult[52]) = "";
		(Amult[22] => Cmult[52]) = "";
		(Amult[23] => Cmult[52]) = "";
		(Amult[24] => Cmult[52]) = "";
		(Amult[25] => Cmult[52]) = "";
		(Amult[26] => Cmult[52]) = "";
		(Amult[27] => Cmult[52]) = "";
		(Amult[28] => Cmult[52]) = "";
		(Amult[29] => Cmult[52]) = "";
		(Amult[30] => Cmult[52]) = "";
		(Amult[31] => Cmult[52]) = "";
		(Bmult[0]  => Cmult[52]) = "";
		(Bmult[1]  => Cmult[52]) = "";
		(Bmult[2]  => Cmult[52]) = "";
		(Bmult[3]  => Cmult[52]) = "";
		(Bmult[4]  => Cmult[52]) = "";
		(Bmult[5]  => Cmult[52]) = "";
		(Bmult[6]  => Cmult[52]) = "";
		(Bmult[7]  => Cmult[52]) = "";
		(Bmult[8]  => Cmult[52]) = "";
		(Bmult[9]  => Cmult[52]) = "";
		(Bmult[10] => Cmult[52]) = "";
		(Bmult[11] => Cmult[52]) = "";
		(Bmult[12] => Cmult[52]) = "";
		(Bmult[13] => Cmult[52]) = "";
		(Bmult[14] => Cmult[52]) = "";
		(Bmult[15] => Cmult[52]) = "";
		(Bmult[16] => Cmult[52]) = "";
		(Bmult[17] => Cmult[52]) = "";
		(Bmult[18] => Cmult[52]) = "";
		(Bmult[19] => Cmult[52]) = "";
		(Bmult[20] => Cmult[52]) = "";
		(Bmult[21] => Cmult[52]) = "";
		(Bmult[22] => Cmult[52]) = "";
		(Bmult[23] => Cmult[52]) = "";
		(Bmult[24] => Cmult[52]) = "";
		(Bmult[25] => Cmult[52]) = "";
		(Bmult[26] => Cmult[52]) = "";
		(Bmult[27] => Cmult[52]) = "";
		(Bmult[28] => Cmult[52]) = "";
		(Bmult[29] => Cmult[52]) = "";
		(Bmult[30] => Cmult[52]) = "";
		(Bmult[31] => Cmult[52]) = "";		
		(Valid_mult[0] => Cmult[52]) = "";
		(Valid_mult[1] => Cmult[52]) = "";
		(sel_mul_32x32 => Cmult[52]) = "";
		(Amult[0]  => Cmult[53]) = "";
		(Amult[1]  => Cmult[53]) = "";
		(Amult[2]  => Cmult[53]) = "";
		(Amult[3]  => Cmult[53]) = "";
		(Amult[4]  => Cmult[53]) = "";
		(Amult[5]  => Cmult[53]) = "";
		(Amult[6]  => Cmult[53]) = "";
		(Amult[7]  => Cmult[53]) = "";
		(Amult[8]  => Cmult[53]) = "";
		(Amult[9]  => Cmult[53]) = "";
		(Amult[10] => Cmult[53]) = "";
		(Amult[11] => Cmult[53]) = "";
		(Amult[12] => Cmult[53]) = "";
		(Amult[13] => Cmult[53]) = "";
		(Amult[14] => Cmult[53]) = "";
		(Amult[15] => Cmult[53]) = "";
		(Amult[16] => Cmult[53]) = "";
		(Amult[17] => Cmult[53]) = "";
		(Amult[18] => Cmult[53]) = "";
		(Amult[19] => Cmult[53]) = "";
		(Amult[20] => Cmult[53]) = "";
		(Amult[21] => Cmult[53]) = "";
		(Amult[22] => Cmult[53]) = "";
		(Amult[23] => Cmult[53]) = "";
		(Amult[24] => Cmult[53]) = "";
		(Amult[25] => Cmult[53]) = "";
		(Amult[26] => Cmult[53]) = "";
		(Amult[27] => Cmult[53]) = "";
		(Amult[28] => Cmult[53]) = "";
		(Amult[29] => Cmult[53]) = "";
		(Amult[30] => Cmult[53]) = "";
		(Amult[31] => Cmult[53]) = "";
		(Bmult[0]  => Cmult[53]) = "";
		(Bmult[1]  => Cmult[53]) = "";
		(Bmult[2]  => Cmult[53]) = "";
		(Bmult[3]  => Cmult[53]) = "";
		(Bmult[4]  => Cmult[53]) = "";
		(Bmult[5]  => Cmult[53]) = "";
		(Bmult[6]  => Cmult[53]) = "";
		(Bmult[7]  => Cmult[53]) = "";
		(Bmult[8]  => Cmult[53]) = "";
		(Bmult[9]  => Cmult[53]) = "";
		(Bmult[10] => Cmult[53]) = "";
		(Bmult[11] => Cmult[53]) = "";
		(Bmult[12] => Cmult[53]) = "";
		(Bmult[13] => Cmult[53]) = "";
		(Bmult[14] => Cmult[53]) = "";
		(Bmult[15] => Cmult[53]) = "";
		(Bmult[16] => Cmult[53]) = "";
		(Bmult[17] => Cmult[53]) = "";
		(Bmult[18] => Cmult[53]) = "";
		(Bmult[19] => Cmult[53]) = "";
		(Bmult[20] => Cmult[53]) = "";
		(Bmult[21] => Cmult[53]) = "";
		(Bmult[22] => Cmult[53]) = "";
		(Bmult[23] => Cmult[53]) = "";
		(Bmult[24] => Cmult[53]) = "";
		(Bmult[25] => Cmult[53]) = "";
		(Bmult[26] => Cmult[53]) = "";
		(Bmult[27] => Cmult[53]) = "";
		(Bmult[28] => Cmult[53]) = "";
		(Bmult[29] => Cmult[53]) = "";
		(Bmult[30] => Cmult[53]) = "";
		(Bmult[31] => Cmult[53]) = "";		
		(Valid_mult[0] => Cmult[53]) = "";
		(Valid_mult[1] => Cmult[53]) = "";
		(sel_mul_32x32 => Cmult[53]) = "";
		(Amult[0]  => Cmult[54]) = "";
		(Amult[1]  => Cmult[54]) = "";
		(Amult[2]  => Cmult[54]) = "";
		(Amult[3]  => Cmult[54]) = "";
		(Amult[4]  => Cmult[54]) = "";
		(Amult[5]  => Cmult[54]) = "";
		(Amult[6]  => Cmult[54]) = "";
		(Amult[7]  => Cmult[54]) = "";
		(Amult[8]  => Cmult[54]) = "";
		(Amult[9]  => Cmult[54]) = "";
		(Amult[10] => Cmult[54]) = "";
		(Amult[11] => Cmult[54]) = "";
		(Amult[12] => Cmult[54]) = "";
		(Amult[13] => Cmult[54]) = "";
		(Amult[14] => Cmult[54]) = "";
		(Amult[15] => Cmult[54]) = "";
		(Amult[16] => Cmult[54]) = "";
		(Amult[17] => Cmult[54]) = "";
		(Amult[18] => Cmult[54]) = "";
		(Amult[19] => Cmult[54]) = "";
		(Amult[20] => Cmult[54]) = "";
		(Amult[21] => Cmult[54]) = "";
		(Amult[22] => Cmult[54]) = "";
		(Amult[23] => Cmult[54]) = "";
		(Amult[24] => Cmult[54]) = "";
		(Amult[25] => Cmult[54]) = "";
		(Amult[26] => Cmult[54]) = "";
		(Amult[27] => Cmult[54]) = "";
		(Amult[28] => Cmult[54]) = "";
		(Amult[29] => Cmult[54]) = "";
		(Amult[30] => Cmult[54]) = "";
		(Amult[31] => Cmult[54]) = "";
		(Bmult[0]  => Cmult[54]) = "";
		(Bmult[1]  => Cmult[54]) = "";
		(Bmult[2]  => Cmult[54]) = "";
		(Bmult[3]  => Cmult[54]) = "";
		(Bmult[4]  => Cmult[54]) = "";
		(Bmult[5]  => Cmult[54]) = "";
		(Bmult[6]  => Cmult[54]) = "";
		(Bmult[7]  => Cmult[54]) = "";
		(Bmult[8]  => Cmult[54]) = "";
		(Bmult[9]  => Cmult[54]) = "";
		(Bmult[10] => Cmult[54]) = "";
		(Bmult[11] => Cmult[54]) = "";
		(Bmult[12] => Cmult[54]) = "";
		(Bmult[13] => Cmult[54]) = "";
		(Bmult[14] => Cmult[54]) = "";
		(Bmult[15] => Cmult[54]) = "";
		(Bmult[16] => Cmult[54]) = "";
		(Bmult[17] => Cmult[54]) = "";
		(Bmult[18] => Cmult[54]) = "";
		(Bmult[19] => Cmult[54]) = "";
		(Bmult[20] => Cmult[54]) = "";
		(Bmult[21] => Cmult[54]) = "";
		(Bmult[22] => Cmult[54]) = "";
		(Bmult[23] => Cmult[54]) = "";
		(Bmult[24] => Cmult[54]) = "";
		(Bmult[25] => Cmult[54]) = "";
		(Bmult[26] => Cmult[54]) = "";
		(Bmult[27] => Cmult[54]) = "";
		(Bmult[28] => Cmult[54]) = "";
		(Bmult[29] => Cmult[54]) = "";
		(Bmult[30] => Cmult[54]) = "";
		(Bmult[31] => Cmult[54]) = "";		
		(Valid_mult[0] => Cmult[54]) = "";
		(Valid_mult[1] => Cmult[54]) = "";
		(sel_mul_32x32 => Cmult[54]) = "";
		(Amult[0]  => Cmult[55]) = "";
		(Amult[1]  => Cmult[55]) = "";
		(Amult[2]  => Cmult[55]) = "";
		(Amult[3]  => Cmult[55]) = "";
		(Amult[4]  => Cmult[55]) = "";
		(Amult[5]  => Cmult[55]) = "";
		(Amult[6]  => Cmult[55]) = "";
		(Amult[7]  => Cmult[55]) = "";
		(Amult[8]  => Cmult[55]) = "";
		(Amult[9]  => Cmult[55]) = "";
		(Amult[10] => Cmult[55]) = "";
		(Amult[11] => Cmult[55]) = "";
		(Amult[12] => Cmult[55]) = "";
		(Amult[13] => Cmult[55]) = "";
		(Amult[14] => Cmult[55]) = "";
		(Amult[15] => Cmult[55]) = "";
		(Amult[16] => Cmult[55]) = "";
		(Amult[17] => Cmult[55]) = "";
		(Amult[18] => Cmult[55]) = "";
		(Amult[19] => Cmult[55]) = "";
		(Amult[20] => Cmult[55]) = "";
		(Amult[21] => Cmult[55]) = "";
		(Amult[22] => Cmult[55]) = "";
		(Amult[23] => Cmult[55]) = "";
		(Amult[24] => Cmult[55]) = "";
		(Amult[25] => Cmult[55]) = "";
		(Amult[26] => Cmult[55]) = "";
		(Amult[27] => Cmult[55]) = "";
		(Amult[28] => Cmult[55]) = "";
		(Amult[29] => Cmult[55]) = "";
		(Amult[30] => Cmult[55]) = "";
		(Amult[31] => Cmult[55]) = "";
		(Bmult[0]  => Cmult[55]) = "";
		(Bmult[1]  => Cmult[55]) = "";
		(Bmult[2]  => Cmult[55]) = "";
		(Bmult[3]  => Cmult[55]) = "";
		(Bmult[4]  => Cmult[55]) = "";
		(Bmult[5]  => Cmult[55]) = "";
		(Bmult[6]  => Cmult[55]) = "";
		(Bmult[7]  => Cmult[55]) = "";
		(Bmult[8]  => Cmult[55]) = "";
		(Bmult[9]  => Cmult[55]) = "";
		(Bmult[10] => Cmult[55]) = "";
		(Bmult[11] => Cmult[55]) = "";
		(Bmult[12] => Cmult[55]) = "";
		(Bmult[13] => Cmult[55]) = "";
		(Bmult[14] => Cmult[55]) = "";
		(Bmult[15] => Cmult[55]) = "";
		(Bmult[16] => Cmult[55]) = "";
		(Bmult[17] => Cmult[55]) = "";
		(Bmult[18] => Cmult[55]) = "";
		(Bmult[19] => Cmult[55]) = "";
		(Bmult[20] => Cmult[55]) = "";
		(Bmult[21] => Cmult[55]) = "";
		(Bmult[22] => Cmult[55]) = "";
		(Bmult[23] => Cmult[55]) = "";
		(Bmult[24] => Cmult[55]) = "";
		(Bmult[25] => Cmult[55]) = "";
		(Bmult[26] => Cmult[55]) = "";
		(Bmult[27] => Cmult[55]) = "";
		(Bmult[28] => Cmult[55]) = "";
		(Bmult[29] => Cmult[55]) = "";
		(Bmult[30] => Cmult[55]) = "";
		(Bmult[31] => Cmult[55]) = "";		
		(Valid_mult[0] => Cmult[55]) = "";
		(Valid_mult[1] => Cmult[55]) = "";
		(sel_mul_32x32 => Cmult[55]) = "";
		(Amult[0]  => Cmult[56]) = "";
		(Amult[1]  => Cmult[56]) = "";
		(Amult[2]  => Cmult[56]) = "";
		(Amult[3]  => Cmult[56]) = "";
		(Amult[4]  => Cmult[56]) = "";
		(Amult[5]  => Cmult[56]) = "";
		(Amult[6]  => Cmult[56]) = "";
		(Amult[7]  => Cmult[56]) = "";
		(Amult[8]  => Cmult[56]) = "";
		(Amult[9]  => Cmult[56]) = "";
		(Amult[10] => Cmult[56]) = "";
		(Amult[11] => Cmult[56]) = "";
		(Amult[12] => Cmult[56]) = "";
		(Amult[13] => Cmult[56]) = "";
		(Amult[14] => Cmult[56]) = "";
		(Amult[15] => Cmult[56]) = "";
		(Amult[16] => Cmult[56]) = "";
		(Amult[17] => Cmult[56]) = "";
		(Amult[18] => Cmult[56]) = "";
		(Amult[19] => Cmult[56]) = "";
		(Amult[20] => Cmult[56]) = "";
		(Amult[21] => Cmult[56]) = "";
		(Amult[22] => Cmult[56]) = "";
		(Amult[23] => Cmult[56]) = "";
		(Amult[24] => Cmult[56]) = "";
		(Amult[25] => Cmult[56]) = "";
		(Amult[26] => Cmult[56]) = "";
		(Amult[27] => Cmult[56]) = "";
		(Amult[28] => Cmult[56]) = "";
		(Amult[29] => Cmult[56]) = "";
		(Amult[30] => Cmult[56]) = "";
		(Amult[31] => Cmult[56]) = "";
		(Bmult[0]  => Cmult[56]) = "";
		(Bmult[1]  => Cmult[56]) = "";
		(Bmult[2]  => Cmult[56]) = "";
		(Bmult[3]  => Cmult[56]) = "";
		(Bmult[4]  => Cmult[56]) = "";
		(Bmult[5]  => Cmult[56]) = "";
		(Bmult[6]  => Cmult[56]) = "";
		(Bmult[7]  => Cmult[56]) = "";
		(Bmult[8]  => Cmult[56]) = "";
		(Bmult[9]  => Cmult[56]) = "";
		(Bmult[10] => Cmult[56]) = "";
		(Bmult[11] => Cmult[56]) = "";
		(Bmult[12] => Cmult[56]) = "";
		(Bmult[13] => Cmult[56]) = "";
		(Bmult[14] => Cmult[56]) = "";
		(Bmult[15] => Cmult[56]) = "";
		(Bmult[16] => Cmult[56]) = "";
		(Bmult[17] => Cmult[56]) = "";
		(Bmult[18] => Cmult[56]) = "";
		(Bmult[19] => Cmult[56]) = "";
		(Bmult[20] => Cmult[56]) = "";
		(Bmult[21] => Cmult[56]) = "";
		(Bmult[22] => Cmult[56]) = "";
		(Bmult[23] => Cmult[56]) = "";
		(Bmult[24] => Cmult[56]) = "";
		(Bmult[25] => Cmult[56]) = "";
		(Bmult[26] => Cmult[56]) = "";
		(Bmult[27] => Cmult[56]) = "";
		(Bmult[28] => Cmult[56]) = "";
		(Bmult[29] => Cmult[56]) = "";
		(Bmult[30] => Cmult[56]) = "";
		(Bmult[31] => Cmult[56]) = "";		
		(Valid_mult[0] => Cmult[56]) = "";
		(Valid_mult[1] => Cmult[56]) = "";
		(sel_mul_32x32 => Cmult[56]) = "";
		(Amult[0]  => Cmult[57]) = "";
		(Amult[1]  => Cmult[57]) = "";
		(Amult[2]  => Cmult[57]) = "";
		(Amult[3]  => Cmult[57]) = "";
		(Amult[4]  => Cmult[57]) = "";
		(Amult[5]  => Cmult[57]) = "";
		(Amult[6]  => Cmult[57]) = "";
		(Amult[7]  => Cmult[57]) = "";
		(Amult[8]  => Cmult[57]) = "";
		(Amult[9]  => Cmult[57]) = "";
		(Amult[10] => Cmult[57]) = "";
		(Amult[11] => Cmult[57]) = "";
		(Amult[12] => Cmult[57]) = "";
		(Amult[13] => Cmult[57]) = "";
		(Amult[14] => Cmult[57]) = "";
		(Amult[15] => Cmult[57]) = "";
		(Amult[16] => Cmult[57]) = "";
		(Amult[17] => Cmult[57]) = "";
		(Amult[18] => Cmult[57]) = "";
		(Amult[19] => Cmult[57]) = "";
		(Amult[20] => Cmult[57]) = "";
		(Amult[21] => Cmult[57]) = "";
		(Amult[22] => Cmult[57]) = "";
		(Amult[23] => Cmult[57]) = "";
		(Amult[24] => Cmult[57]) = "";
		(Amult[25] => Cmult[57]) = "";
		(Amult[26] => Cmult[57]) = "";
		(Amult[27] => Cmult[57]) = "";
		(Amult[28] => Cmult[57]) = "";
		(Amult[29] => Cmult[57]) = "";
		(Amult[30] => Cmult[57]) = "";
		(Amult[31] => Cmult[57]) = "";
		(Bmult[0]  => Cmult[57]) = "";
		(Bmult[1]  => Cmult[57]) = "";
		(Bmult[2]  => Cmult[57]) = "";
		(Bmult[3]  => Cmult[57]) = "";
		(Bmult[4]  => Cmult[57]) = "";
		(Bmult[5]  => Cmult[57]) = "";
		(Bmult[6]  => Cmult[57]) = "";
		(Bmult[7]  => Cmult[57]) = "";
		(Bmult[8]  => Cmult[57]) = "";
		(Bmult[9]  => Cmult[57]) = "";
		(Bmult[10] => Cmult[57]) = "";
		(Bmult[11] => Cmult[57]) = "";
		(Bmult[12] => Cmult[57]) = "";
		(Bmult[13] => Cmult[57]) = "";
		(Bmult[14] => Cmult[57]) = "";
		(Bmult[15] => Cmult[57]) = "";
		(Bmult[16] => Cmult[57]) = "";
		(Bmult[17] => Cmult[57]) = "";
		(Bmult[18] => Cmult[57]) = "";
		(Bmult[19] => Cmult[57]) = "";
		(Bmult[20] => Cmult[57]) = "";
		(Bmult[21] => Cmult[57]) = "";
		(Bmult[22] => Cmult[57]) = "";
		(Bmult[23] => Cmult[57]) = "";
		(Bmult[24] => Cmult[57]) = "";
		(Bmult[25] => Cmult[57]) = "";
		(Bmult[26] => Cmult[57]) = "";
		(Bmult[27] => Cmult[57]) = "";
		(Bmult[28] => Cmult[57]) = "";
		(Bmult[29] => Cmult[57]) = "";
		(Bmult[30] => Cmult[57]) = "";
		(Bmult[31] => Cmult[57]) = "";		
		(Valid_mult[0] => Cmult[57]) = "";
		(Valid_mult[1] => Cmult[57]) = "";
		(sel_mul_32x32 => Cmult[57]) = "";
		(Amult[0]  => Cmult[58]) = "";
		(Amult[1]  => Cmult[58]) = "";
		(Amult[2]  => Cmult[58]) = "";
		(Amult[3]  => Cmult[58]) = "";
		(Amult[4]  => Cmult[58]) = "";
		(Amult[5]  => Cmult[58]) = "";
		(Amult[6]  => Cmult[58]) = "";
		(Amult[7]  => Cmult[58]) = "";
		(Amult[8]  => Cmult[58]) = "";
		(Amult[9]  => Cmult[58]) = "";
		(Amult[10] => Cmult[58]) = "";
		(Amult[11] => Cmult[58]) = "";
		(Amult[12] => Cmult[58]) = "";
		(Amult[13] => Cmult[58]) = "";
		(Amult[14] => Cmult[58]) = "";
		(Amult[15] => Cmult[58]) = "";
		(Amult[16] => Cmult[58]) = "";
		(Amult[17] => Cmult[58]) = "";
		(Amult[18] => Cmult[58]) = "";
		(Amult[19] => Cmult[58]) = "";
		(Amult[20] => Cmult[58]) = "";
		(Amult[21] => Cmult[58]) = "";
		(Amult[22] => Cmult[58]) = "";
		(Amult[23] => Cmult[58]) = "";
		(Amult[24] => Cmult[58]) = "";
		(Amult[25] => Cmult[58]) = "";
		(Amult[26] => Cmult[58]) = "";
		(Amult[27] => Cmult[58]) = "";
		(Amult[28] => Cmult[58]) = "";
		(Amult[29] => Cmult[58]) = "";
		(Amult[30] => Cmult[58]) = "";
		(Amult[31] => Cmult[58]) = "";
		(Bmult[0]  => Cmult[58]) = "";
		(Bmult[1]  => Cmult[58]) = "";
		(Bmult[2]  => Cmult[58]) = "";
		(Bmult[3]  => Cmult[58]) = "";
		(Bmult[4]  => Cmult[58]) = "";
		(Bmult[5]  => Cmult[58]) = "";
		(Bmult[6]  => Cmult[58]) = "";
		(Bmult[7]  => Cmult[58]) = "";
		(Bmult[8]  => Cmult[58]) = "";
		(Bmult[9]  => Cmult[58]) = "";
		(Bmult[10] => Cmult[58]) = "";
		(Bmult[11] => Cmult[58]) = "";
		(Bmult[12] => Cmult[58]) = "";
		(Bmult[13] => Cmult[58]) = "";
		(Bmult[14] => Cmult[58]) = "";
		(Bmult[15] => Cmult[58]) = "";
		(Bmult[16] => Cmult[58]) = "";
		(Bmult[17] => Cmult[58]) = "";
		(Bmult[18] => Cmult[58]) = "";
		(Bmult[19] => Cmult[58]) = "";
		(Bmult[20] => Cmult[58]) = "";
		(Bmult[21] => Cmult[58]) = "";
		(Bmult[22] => Cmult[58]) = "";
		(Bmult[23] => Cmult[58]) = "";
		(Bmult[24] => Cmult[58]) = "";
		(Bmult[25] => Cmult[58]) = "";
		(Bmult[26] => Cmult[58]) = "";
		(Bmult[27] => Cmult[58]) = "";
		(Bmult[28] => Cmult[58]) = "";
		(Bmult[29] => Cmult[58]) = "";
		(Bmult[30] => Cmult[58]) = "";
		(Bmult[31] => Cmult[58]) = "";		
		(Valid_mult[0] => Cmult[58]) = "";
		(Valid_mult[1] => Cmult[58]) = "";
		(sel_mul_32x32 => Cmult[58]) = "";	
		(Amult[0]  => Cmult[59]) = "";
		(Amult[1]  => Cmult[59]) = "";
		(Amult[2]  => Cmult[59]) = "";
		(Amult[3]  => Cmult[59]) = "";
		(Amult[4]  => Cmult[59]) = "";
		(Amult[5]  => Cmult[59]) = "";
		(Amult[6]  => Cmult[59]) = "";
		(Amult[7]  => Cmult[59]) = "";
		(Amult[8]  => Cmult[59]) = "";
		(Amult[9]  => Cmult[59]) = "";
		(Amult[10] => Cmult[59]) = "";
		(Amult[11] => Cmult[59]) = "";
		(Amult[12] => Cmult[59]) = "";
		(Amult[13] => Cmult[59]) = "";
		(Amult[14] => Cmult[59]) = "";
		(Amult[15] => Cmult[59]) = "";
		(Amult[16] => Cmult[59]) = "";
		(Amult[17] => Cmult[59]) = "";
		(Amult[18] => Cmult[59]) = "";
		(Amult[19] => Cmult[59]) = "";
		(Amult[20] => Cmult[59]) = "";
		(Amult[21] => Cmult[59]) = "";
		(Amult[22] => Cmult[59]) = "";
		(Amult[23] => Cmult[59]) = "";
		(Amult[24] => Cmult[59]) = "";
		(Amult[25] => Cmult[59]) = "";
		(Amult[26] => Cmult[59]) = "";
		(Amult[27] => Cmult[59]) = "";
		(Amult[28] => Cmult[59]) = "";
		(Amult[29] => Cmult[59]) = "";
		(Amult[30] => Cmult[59]) = "";
		(Amult[31] => Cmult[59]) = "";
		(Bmult[0]  => Cmult[59]) = "";
		(Bmult[1]  => Cmult[59]) = "";
		(Bmult[2]  => Cmult[59]) = "";
		(Bmult[3]  => Cmult[59]) = "";
		(Bmult[4]  => Cmult[59]) = "";
		(Bmult[5]  => Cmult[59]) = "";
		(Bmult[6]  => Cmult[59]) = "";
		(Bmult[7]  => Cmult[59]) = "";
		(Bmult[8]  => Cmult[59]) = "";
		(Bmult[9]  => Cmult[59]) = "";
		(Bmult[10] => Cmult[59]) = "";
		(Bmult[11] => Cmult[59]) = "";
		(Bmult[12] => Cmult[59]) = "";
		(Bmult[13] => Cmult[59]) = "";
		(Bmult[14] => Cmult[59]) = "";
		(Bmult[15] => Cmult[59]) = "";
		(Bmult[16] => Cmult[59]) = "";
		(Bmult[17] => Cmult[59]) = "";
		(Bmult[18] => Cmult[59]) = "";
		(Bmult[19] => Cmult[59]) = "";
		(Bmult[20] => Cmult[59]) = "";
		(Bmult[21] => Cmult[59]) = "";
		(Bmult[22] => Cmult[59]) = "";
		(Bmult[23] => Cmult[59]) = "";
		(Bmult[24] => Cmult[59]) = "";
		(Bmult[25] => Cmult[59]) = "";
		(Bmult[26] => Cmult[59]) = "";
		(Bmult[27] => Cmult[59]) = "";
		(Bmult[28] => Cmult[59]) = "";
		(Bmult[29] => Cmult[59]) = "";
		(Bmult[30] => Cmult[59]) = "";
		(Bmult[31] => Cmult[59]) = "";		
		(Valid_mult[0] => Cmult[59]) = "";
		(Valid_mult[1] => Cmult[59]) = "";
		(sel_mul_32x32 => Cmult[59]) = "";
		(Amult[0]  => Cmult[60]) = "";
		(Amult[1]  => Cmult[60]) = "";
		(Amult[2]  => Cmult[60]) = "";
		(Amult[3]  => Cmult[60]) = "";
		(Amult[4]  => Cmult[60]) = "";
		(Amult[5]  => Cmult[60]) = "";
		(Amult[6]  => Cmult[60]) = "";
		(Amult[7]  => Cmult[60]) = "";
		(Amult[8]  => Cmult[60]) = "";
		(Amult[9]  => Cmult[60]) = "";
		(Amult[10] => Cmult[60]) = "";
		(Amult[11] => Cmult[60]) = "";
		(Amult[12] => Cmult[60]) = "";
		(Amult[13] => Cmult[60]) = "";
		(Amult[14] => Cmult[60]) = "";
		(Amult[15] => Cmult[60]) = "";
		(Amult[16] => Cmult[60]) = "";
		(Amult[17] => Cmult[60]) = "";
		(Amult[18] => Cmult[60]) = "";
		(Amult[19] => Cmult[60]) = "";
		(Amult[20] => Cmult[60]) = "";
		(Amult[21] => Cmult[60]) = "";
		(Amult[22] => Cmult[60]) = "";
		(Amult[23] => Cmult[60]) = "";
		(Amult[24] => Cmult[60]) = "";
		(Amult[25] => Cmult[60]) = "";
		(Amult[26] => Cmult[60]) = "";
		(Amult[27] => Cmult[60]) = "";
		(Amult[28] => Cmult[60]) = "";
		(Amult[29] => Cmult[60]) = "";
		(Amult[30] => Cmult[60]) = "";
		(Amult[31] => Cmult[60]) = "";
		(Bmult[0]  => Cmult[60]) = "";
		(Bmult[1]  => Cmult[60]) = "";
		(Bmult[2]  => Cmult[60]) = "";
		(Bmult[3]  => Cmult[60]) = "";
		(Bmult[4]  => Cmult[60]) = "";
		(Bmult[5]  => Cmult[60]) = "";
		(Bmult[6]  => Cmult[60]) = "";
		(Bmult[7]  => Cmult[60]) = "";
		(Bmult[8]  => Cmult[60]) = "";
		(Bmult[9]  => Cmult[60]) = "";
		(Bmult[10] => Cmult[60]) = "";
		(Bmult[11] => Cmult[60]) = "";
		(Bmult[12] => Cmult[60]) = "";
		(Bmult[13] => Cmult[60]) = "";
		(Bmult[14] => Cmult[60]) = "";
		(Bmult[15] => Cmult[60]) = "";
		(Bmult[16] => Cmult[60]) = "";
		(Bmult[17] => Cmult[60]) = "";
		(Bmult[18] => Cmult[60]) = "";
		(Bmult[19] => Cmult[60]) = "";
		(Bmult[20] => Cmult[60]) = "";
		(Bmult[21] => Cmult[60]) = "";
		(Bmult[22] => Cmult[60]) = "";
		(Bmult[23] => Cmult[60]) = "";
		(Bmult[24] => Cmult[60]) = "";
		(Bmult[25] => Cmult[60]) = "";
		(Bmult[26] => Cmult[60]) = "";
		(Bmult[27] => Cmult[60]) = "";
		(Bmult[28] => Cmult[60]) = "";
		(Bmult[29] => Cmult[60]) = "";
		(Bmult[30] => Cmult[60]) = "";
		(Bmult[31] => Cmult[60]) = "";		
		(Valid_mult[0] => Cmult[60]) = "";
		(Valid_mult[1] => Cmult[60]) = "";
		(sel_mul_32x32 => Cmult[60]) = "";
		(Amult[0]  => Cmult[61]) = "";
		(Amult[1]  => Cmult[61]) = "";
		(Amult[2]  => Cmult[61]) = "";
		(Amult[3]  => Cmult[61]) = "";
		(Amult[4]  => Cmult[61]) = "";
		(Amult[5]  => Cmult[61]) = "";
		(Amult[6]  => Cmult[61]) = "";
		(Amult[7]  => Cmult[61]) = "";
		(Amult[8]  => Cmult[61]) = "";
		(Amult[9]  => Cmult[61]) = "";
		(Amult[10] => Cmult[61]) = "";
		(Amult[11] => Cmult[61]) = "";
		(Amult[12] => Cmult[61]) = "";
		(Amult[13] => Cmult[61]) = "";
		(Amult[14] => Cmult[61]) = "";
		(Amult[15] => Cmult[61]) = "";
		(Amult[16] => Cmult[61]) = "";
		(Amult[17] => Cmult[61]) = "";
		(Amult[18] => Cmult[61]) = "";
		(Amult[19] => Cmult[61]) = "";
		(Amult[20] => Cmult[61]) = "";
		(Amult[21] => Cmult[61]) = "";
		(Amult[22] => Cmult[61]) = "";
		(Amult[23] => Cmult[61]) = "";
		(Amult[24] => Cmult[61]) = "";
		(Amult[25] => Cmult[61]) = "";
		(Amult[26] => Cmult[61]) = "";
		(Amult[27] => Cmult[61]) = "";
		(Amult[28] => Cmult[61]) = "";
		(Amult[29] => Cmult[61]) = "";
		(Amult[30] => Cmult[61]) = "";
		(Amult[31] => Cmult[61]) = "";
		(Bmult[0]  => Cmult[61]) = "";
		(Bmult[1]  => Cmult[61]) = "";
		(Bmult[2]  => Cmult[61]) = "";
		(Bmult[3]  => Cmult[61]) = "";
		(Bmult[4]  => Cmult[61]) = "";
		(Bmult[5]  => Cmult[61]) = "";
		(Bmult[6]  => Cmult[61]) = "";
		(Bmult[7]  => Cmult[61]) = "";
		(Bmult[8]  => Cmult[61]) = "";
		(Bmult[9]  => Cmult[61]) = "";
		(Bmult[10] => Cmult[61]) = "";
		(Bmult[11] => Cmult[61]) = "";
		(Bmult[12] => Cmult[61]) = "";
		(Bmult[13] => Cmult[61]) = "";
		(Bmult[14] => Cmult[61]) = "";
		(Bmult[15] => Cmult[61]) = "";
		(Bmult[16] => Cmult[61]) = "";
		(Bmult[17] => Cmult[61]) = "";
		(Bmult[18] => Cmult[61]) = "";
		(Bmult[19] => Cmult[61]) = "";
		(Bmult[20] => Cmult[61]) = "";
		(Bmult[21] => Cmult[61]) = "";
		(Bmult[22] => Cmult[61]) = "";
		(Bmult[23] => Cmult[61]) = "";
		(Bmult[24] => Cmult[61]) = "";
		(Bmult[25] => Cmult[61]) = "";
		(Bmult[26] => Cmult[61]) = "";
		(Bmult[27] => Cmult[61]) = "";
		(Bmult[28] => Cmult[61]) = "";
		(Bmult[29] => Cmult[61]) = "";
		(Bmult[30] => Cmult[61]) = "";
		(Bmult[31] => Cmult[61]) = "";		
		(Valid_mult[0] => Cmult[61]) = "";
		(Valid_mult[1] => Cmult[61]) = "";
		(sel_mul_32x32 => Cmult[61]) = "";
		(Amult[0]  => Cmult[62]) = "";
		(Amult[1]  => Cmult[62]) = "";
		(Amult[2]  => Cmult[62]) = "";
		(Amult[3]  => Cmult[62]) = "";
		(Amult[4]  => Cmult[62]) = "";
		(Amult[5]  => Cmult[62]) = "";
		(Amult[6]  => Cmult[62]) = "";
		(Amult[7]  => Cmult[62]) = "";
		(Amult[8]  => Cmult[62]) = "";
		(Amult[9]  => Cmult[62]) = "";
		(Amult[10] => Cmult[62]) = "";
		(Amult[11] => Cmult[62]) = "";
		(Amult[12] => Cmult[62]) = "";
		(Amult[13] => Cmult[62]) = "";
		(Amult[14] => Cmult[62]) = "";
		(Amult[15] => Cmult[62]) = "";
		(Amult[16] => Cmult[62]) = "";
		(Amult[17] => Cmult[62]) = "";
		(Amult[18] => Cmult[62]) = "";
		(Amult[19] => Cmult[62]) = "";
		(Amult[20] => Cmult[62]) = "";
		(Amult[21] => Cmult[62]) = "";
		(Amult[22] => Cmult[62]) = "";
		(Amult[23] => Cmult[62]) = "";
		(Amult[24] => Cmult[62]) = "";
		(Amult[25] => Cmult[62]) = "";
		(Amult[26] => Cmult[62]) = "";
		(Amult[27] => Cmult[62]) = "";
		(Amult[28] => Cmult[62]) = "";
		(Amult[29] => Cmult[62]) = "";
		(Amult[30] => Cmult[62]) = "";
		(Amult[31] => Cmult[62]) = "";
		(Bmult[0]  => Cmult[62]) = "";
		(Bmult[1]  => Cmult[62]) = "";
		(Bmult[2]  => Cmult[62]) = "";
		(Bmult[3]  => Cmult[62]) = "";
		(Bmult[4]  => Cmult[62]) = "";
		(Bmult[5]  => Cmult[62]) = "";
		(Bmult[6]  => Cmult[62]) = "";
		(Bmult[7]  => Cmult[62]) = "";
		(Bmult[8]  => Cmult[62]) = "";
		(Bmult[9]  => Cmult[62]) = "";
		(Bmult[10] => Cmult[62]) = "";
		(Bmult[11] => Cmult[62]) = "";
		(Bmult[12] => Cmult[62]) = "";
		(Bmult[13] => Cmult[62]) = "";
		(Bmult[14] => Cmult[62]) = "";
		(Bmult[15] => Cmult[62]) = "";
		(Bmult[16] => Cmult[62]) = "";
		(Bmult[17] => Cmult[62]) = "";
		(Bmult[18] => Cmult[62]) = "";
		(Bmult[19] => Cmult[62]) = "";
		(Bmult[20] => Cmult[62]) = "";
		(Bmult[21] => Cmult[62]) = "";
		(Bmult[22] => Cmult[62]) = "";
		(Bmult[23] => Cmult[62]) = "";
		(Bmult[24] => Cmult[62]) = "";
		(Bmult[25] => Cmult[62]) = "";
		(Bmult[26] => Cmult[62]) = "";
		(Bmult[27] => Cmult[62]) = "";
		(Bmult[28] => Cmult[62]) = "";
		(Bmult[29] => Cmult[62]) = "";
		(Bmult[30] => Cmult[62]) = "";
		(Bmult[31] => Cmult[62]) = "";		
		(Valid_mult[0] => Cmult[62]) = "";
		(Valid_mult[1] => Cmult[62]) = "";
		(sel_mul_32x32 => Cmult[62]) = "";
		(Amult[0]  => Cmult[63]) = "";
		(Amult[1]  => Cmult[63]) = "";
		(Amult[2]  => Cmult[63]) = "";
		(Amult[3]  => Cmult[63]) = "";
		(Amult[4]  => Cmult[63]) = "";
		(Amult[5]  => Cmult[63]) = "";
		(Amult[6]  => Cmult[63]) = "";
		(Amult[7]  => Cmult[63]) = "";
		(Amult[8]  => Cmult[63]) = "";
		(Amult[9]  => Cmult[63]) = "";
		(Amult[10] => Cmult[63]) = "";
		(Amult[11] => Cmult[63]) = "";
		(Amult[12] => Cmult[63]) = "";
		(Amult[13] => Cmult[63]) = "";
		(Amult[14] => Cmult[63]) = "";
		(Amult[15] => Cmult[63]) = "";
		(Amult[16] => Cmult[63]) = "";
		(Amult[17] => Cmult[63]) = "";
		(Amult[18] => Cmult[63]) = "";
		(Amult[19] => Cmult[63]) = "";
		(Amult[20] => Cmult[63]) = "";
		(Amult[21] => Cmult[63]) = "";
		(Amult[22] => Cmult[63]) = "";
		(Amult[23] => Cmult[63]) = "";
		(Amult[24] => Cmult[63]) = "";
		(Amult[25] => Cmult[63]) = "";
		(Amult[26] => Cmult[63]) = "";
		(Amult[27] => Cmult[63]) = "";
		(Amult[28] => Cmult[63]) = "";
		(Amult[29] => Cmult[63]) = "";
		(Amult[30] => Cmult[63]) = "";
		(Amult[31] => Cmult[63]) = "";
		(Bmult[0]  => Cmult[63]) = "";
		(Bmult[1]  => Cmult[63]) = "";
		(Bmult[2]  => Cmult[63]) = "";
		(Bmult[3]  => Cmult[63]) = "";
		(Bmult[4]  => Cmult[63]) = "";
		(Bmult[5]  => Cmult[63]) = "";
		(Bmult[6]  => Cmult[63]) = "";
		(Bmult[7]  => Cmult[63]) = "";
		(Bmult[8]  => Cmult[63]) = "";
		(Bmult[9]  => Cmult[63]) = "";
		(Bmult[10] => Cmult[63]) = "";
		(Bmult[11] => Cmult[63]) = "";
		(Bmult[12] => Cmult[63]) = "";
		(Bmult[13] => Cmult[63]) = "";
		(Bmult[14] => Cmult[63]) = "";
		(Bmult[15] => Cmult[63]) = "";
		(Bmult[16] => Cmult[63]) = "";
		(Bmult[17] => Cmult[63]) = "";
		(Bmult[18] => Cmult[63]) = "";
		(Bmult[19] => Cmult[63]) = "";
		(Bmult[20] => Cmult[63]) = "";
		(Bmult[21] => Cmult[63]) = "";
		(Bmult[22] => Cmult[63]) = "";
		(Bmult[23] => Cmult[63]) = "";
		(Bmult[24] => Cmult[63]) = "";
		(Bmult[25] => Cmult[63]) = "";
		(Bmult[26] => Cmult[63]) = "";
		(Bmult[27] => Cmult[63]) = "";
		(Bmult[28] => Cmult[63]) = "";
		(Bmult[29] => Cmult[63]) = "";
		(Bmult[30] => Cmult[63]) = "";
		(Bmult[31] => Cmult[63]) = "";		
		(Valid_mult[0] => Cmult[63]) = "";
		(Valid_mult[1] => Cmult[63]) = "";
		(sel_mul_32x32 => Cmult[63]) = "";		
    endspecify
`endif
	
	always @(*) begin
		if (sel_mul_32x32 == 1'b1) begin
			if (Valid_mult[0] == 1'b1) begin
				Cmult <= Amult * Bmult;
			end
		end else begin
			if (Valid_mult[0] == 1'b1) begin
				Cmult[31:0] <= Amult[15:0] * Bmult[15:0];
			end
			if (Valid_mult[1] == 1'b1) begin
				Cmult[63:32] <= Amult[31:16] * Bmult[31:16];
			end
		end
	end


endmodule
// ../../../quicklogic/pp3/primitives/mult/mult.sim.v }}}

// ../../../quicklogic/pp3/primitives/gmux/gmux_ip.sim.v {{{
(* whitebox *)
(* FASM_FEATURES="I_invblock.I_J0.ZINV.IS0;I_invblock.I_J1.ZINV.IS1;I_invblock.I_J2.ZINV.IS0;I_invblock.I_J3.ZINV.IS0;I_invblock.I_J4.ZINV.IS1" *)
module GMUX_IP (IP, IC, IS0, IZ);

    input  wire IP;
    input  wire IC;
    input  wire IS0;

    (* DELAY_CONST_IP="{iopath_IP_IZ}" *)
    (* DELAY_CONST_IC="{iopath_IC_IZ}" *)
    (* DELAY_CONST_IS0="1e-10" *)
    (* clkbuf_driver *)
    output wire IZ;

    specify
        (IP => IZ) = "";
		(IC => IZ) = "";
		(IS0 => IZ) = "";
    endspecify

    assign IZ = IS0 ? IC : IP;

endmodule
// ../../../quicklogic/pp3/primitives/gmux/gmux_ip.sim.v }}}

// ../../../quicklogic/pp3/primitives/gmux/gmux_ic.sim.v {{{
(* whitebox *)
(* FASM_FEATURES="I_invblock.I_J0.ZINV.IS0;I_invblock.I_J1.ZINV.IS1;I_invblock.I_J2.ZINV.IS0;I_invblock.I_J3.ZINV.IS0;I_invblock.I_J4.ZINV.IS1" *)
module GMUX_IC (IC, IS0, IZ);

    input  wire IC;
    input  wire IS0;

    (* DELAY_CONST_IC="{iopath_IC_IZ}" *)
    (* DELAY_CONST_IS0="1e-10" *)
    (* clkbuf_driver *)
    output wire IZ;
	
    specify
        (IC => IZ) = "";
		(IS0 => IZ) = "";
    endspecify

    assign IZ = IS0 ? IC : 1'bx;

endmodule
// ../../../quicklogic/pp3/primitives/gmux/gmux_ic.sim.v }}}

// ../../../quicklogic/pp3/primitives/ramfifo/ramfifo.sim.v {{{
`timescale 1ns/10ps
module fifo_controller_model(
	 Rst_n,
	 Push_Clk,
	 Pop_Clk,
	
	 Fifo_Push,
	 Fifo_Push_Flush,
	 Fifo_Full,
	 Fifo_Full_Usr,
	
	 Fifo_Pop,
	 Fifo_Pop_Flush,
	 Fifo_Empty,
	 Fifo_Empty_Usr,
	
	 Write_Addr,
	
	 Read_Addr,
	 												 
	 Fifo_Ram_Mode,
	 Fifo_Sync_Mode,
	 Fifo_Push_Width,
	 Fifo_Pop_Width
	  );
	
    parameter MAX_PTR_WIDTH   = 12;
  
    parameter DEPTH1 = (1<<(MAX_PTR_WIDTH-3));
    parameter DEPTH2 = (1<<(MAX_PTR_WIDTH-2));
    parameter DEPTH3 = (1<<(MAX_PTR_WIDTH-1));
  
    parameter D1_QTR_A = MAX_PTR_WIDTH - 5;
    parameter D2_QTR_A = MAX_PTR_WIDTH - 4;
    parameter D3_QTR_A = MAX_PTR_WIDTH - 3;

	input	Rst_n;
	input	Push_Clk;
	input	Pop_Clk;
	
	input	Fifo_Push;
	input	Fifo_Push_Flush;
	output	Fifo_Full;
	output	[3:0]  Fifo_Full_Usr;
                            		
	input	Fifo_Pop;
	input	Fifo_Pop_Flush;
	output	Fifo_Empty;
	output	[3:0]  Fifo_Empty_Usr;
	
	output	[MAX_PTR_WIDTH-2:0]  Write_Addr;
	
	output	[MAX_PTR_WIDTH-2:0]  Read_Addr;
		
	input  Fifo_Ram_Mode;
	input  Fifo_Sync_Mode;
	input  [1:0] Fifo_Push_Width;
	input  [1:0] Fifo_Pop_Width;
	
	reg    flush_pop_clk_tf;
	reg    flush_pop2push_clk1;
	reg    flush_push_clk_tf;
	reg    flush_push2pop_clk1;
	reg    pop_local_flush_mask;
	reg    push_flush_tf_pop_clk;
	reg    pop2push_ack1;
	reg    pop2push_ack2;
	reg    push_local_flush_mask;
	reg    pop_flush_tf_push_clk;
	reg    push2pop_ack1;
	reg    push2pop_ack2;
	
	reg    fifo_full_flag_f;
	reg    [3:0]  Fifo_Full_Usr;

	reg    fifo_empty_flag_f;
	reg    [3:0]  Fifo_Empty_Usr;

	reg    [MAX_PTR_WIDTH-1:0]  push_ptr_push_clk;
	reg    [MAX_PTR_WIDTH-1:0]  pop_ptr_push_clk;
	reg    [MAX_PTR_WIDTH-1:0]  pop_ptr_async;	    
	reg    [MAX_PTR_WIDTH-1:0]  pop_ptr_pop_clk ;
	reg    [MAX_PTR_WIDTH-1:0]  push_ptr_pop_clk;
	reg    [MAX_PTR_WIDTH-1:0]  push_ptr_async;

	reg    [1:0]  push_ptr_push_clk_mask;
	reg    [1:0]  pop_ptr_pop_clk_mask;

	reg    [MAX_PTR_WIDTH-1:0]  pop_ptr_push_clk_mux;
	reg    [MAX_PTR_WIDTH-1:0]  push_ptr_pop_clk_mux;

	reg    match_room4none;		
	reg    match_room4one;		
	reg    match_room4half;  	      
	reg    match_room4quart;

	reg    match_all_left;
	reg    match_half_left;
	reg    match_quart_left;

 	reg   [MAX_PTR_WIDTH-1:0]   depth1_reg;
 	reg   [MAX_PTR_WIDTH-1:0]   depth2_reg;
 	reg   [MAX_PTR_WIDTH-1:0]   depth3_reg;
  

	wire	push_clk_rst;
	wire	push_clk_rst_mux;
	wire	push_flush_done;
	wire	pop_clk_rst;
	wire	pop_clk_rst_mux;
	wire	pop_flush_done;

	wire	push_flush_gated;
	wire	pop_flush_gated;
	                          	
	wire	[MAX_PTR_WIDTH-2:0] Write_Addr;
	wire	[MAX_PTR_WIDTH-2:0] Read_Addr;
	
	wire	[MAX_PTR_WIDTH-1:0] push_ptr_push_clk_plus1;
	wire	[MAX_PTR_WIDTH-1:0] next_push_ptr_push_clk;
	wire	[MAX_PTR_WIDTH-1:0] pop_ptr_pop_clk_plus1;
	wire	[MAX_PTR_WIDTH-1:0] next_pop_ptr_pop_clk;
	wire	[MAX_PTR_WIDTH-1:0] next_push_ptr_push_clk_mask;
	wire	[MAX_PTR_WIDTH-1:0] next_pop_ptr_pop_clk_mask;

	wire	[MAX_PTR_WIDTH-1:0] pop_ptr_push_clk_l_shift1;
	wire	[MAX_PTR_WIDTH-1:0] pop_ptr_push_clk_l_shift2;
	wire	[MAX_PTR_WIDTH-1:0] pop_ptr_push_clk_r_shift1;
	wire	[MAX_PTR_WIDTH-1:0] pop_ptr_push_clk_r_shift2;

	wire	[MAX_PTR_WIDTH-1:0] push_ptr_pop_clk_l_shift1;
	wire	[MAX_PTR_WIDTH-1:0] push_ptr_pop_clk_l_shift2;
	wire	[MAX_PTR_WIDTH-1:0] push_ptr_pop_clk_r_shift1;
	wire	[MAX_PTR_WIDTH-1:0] push_ptr_pop_clk_r_shift2;

	wire	[MAX_PTR_WIDTH-1:0] push_diff;
	wire	[MAX_PTR_WIDTH-1:0] push_diff_plus_1;
	wire	[MAX_PTR_WIDTH-1:0] pop_diff;
		
	wire	match_room4all;		
	wire	match_room4eight;	
	
	wire	match_one_left;			
	wire	match_one2eight_left;
	
	integer	depth_sel_push;
	integer depth_sel_pop;

    initial
    begin
        depth1_reg = DEPTH1;
        depth2_reg = DEPTH2;
        depth3_reg = DEPTH3;
    end
	
	initial
	begin
		flush_pop_clk_tf		<= 1'b0;
		push2pop_ack1			<= 1'b0;
		push2pop_ack2			<= 1'b0;
		pop_local_flush_mask	<= 1'b0;
		flush_push2pop_clk1		<= 1'b0;
		push_flush_tf_pop_clk	<= 1'b0;
		flush_push_clk_tf	    <= 1'b0;
		pop2push_ack1			<= 1'b0;
		pop2push_ack2			<= 1'b0;
		push_local_flush_mask	<= 1'b0;
		flush_pop2push_clk1		<= 1'b0;
		pop_flush_tf_push_clk	<= 1'b0;
		push_ptr_push_clk	    <= 0;
		pop_ptr_push_clk		<= 0;
		pop_ptr_async			<= 0;
		fifo_full_flag_f		<= 0;
		pop_ptr_pop_clk			<= 0;
		push_ptr_pop_clk		<= 0;
		push_ptr_async			<= 0;
		fifo_empty_flag_f		<= 1;
		Fifo_Full_Usr			<= 4'b0001;
		Fifo_Empty_Usr			<= 4'b0000;
	end

	assign	Fifo_Full	= fifo_full_flag_f;
	assign	Fifo_Empty	= fifo_empty_flag_f;

	assign	Write_Addr	= push_ptr_push_clk[MAX_PTR_WIDTH-2:0];
	assign	Read_Addr	= next_pop_ptr_pop_clk[MAX_PTR_WIDTH-2:0];

	assign	push_ptr_push_clk_plus1			= push_ptr_push_clk + 1;
	assign	next_push_ptr_push_clk			= ( Fifo_Push ) ? push_ptr_push_clk_plus1 : push_ptr_push_clk;
	assign	next_push_ptr_push_clk_mask	= { ( push_ptr_push_clk_mask & next_push_ptr_push_clk[MAX_PTR_WIDTH-1:MAX_PTR_WIDTH-2] ), next_push_ptr_push_clk[MAX_PTR_WIDTH-3:0] };	

	assign	pop_ptr_pop_clk_plus1				= pop_ptr_pop_clk + 1;
	assign	next_pop_ptr_pop_clk				= ( Fifo_Pop ) ? pop_ptr_pop_clk_plus1 : pop_ptr_pop_clk;
	assign	next_pop_ptr_pop_clk_mask		= { ( pop_ptr_pop_clk_mask & next_pop_ptr_pop_clk[MAX_PTR_WIDTH-1:MAX_PTR_WIDTH-2] ), next_pop_ptr_pop_clk[MAX_PTR_WIDTH-3:0] };

	assign	pop_ptr_push_clk_l_shift1	= { pop_ptr_push_clk[MAX_PTR_WIDTH-2:0], 1'b0 };
	assign	pop_ptr_push_clk_l_shift2	= { pop_ptr_push_clk[MAX_PTR_WIDTH-3:0], 2'b0 };
	assign	pop_ptr_push_clk_r_shift1	= { 1'b0, pop_ptr_push_clk[MAX_PTR_WIDTH-1:1] };
	assign	pop_ptr_push_clk_r_shift2	= { 2'b0, pop_ptr_push_clk[MAX_PTR_WIDTH-1:2] };

	assign	push_ptr_pop_clk_l_shift1	= { push_ptr_pop_clk[MAX_PTR_WIDTH-2:0], 1'b0 };
	assign	push_ptr_pop_clk_l_shift2	= { push_ptr_pop_clk[MAX_PTR_WIDTH-3:0], 2'b0 };
	assign	push_ptr_pop_clk_r_shift1	= { 1'b0, push_ptr_pop_clk[MAX_PTR_WIDTH-1:1] };
	assign	push_ptr_pop_clk_r_shift2	= { 2'b0, push_ptr_pop_clk[MAX_PTR_WIDTH-1:2] };

	assign	push_diff					= next_push_ptr_push_clk_mask - pop_ptr_push_clk_mux;
	assign	push_diff_plus_1	= push_diff + 1;
	assign	pop_diff					= push_ptr_pop_clk_mux - next_pop_ptr_pop_clk_mask;

	assign	match_room4all		= ~|push_diff;
	assign	match_room4eight	= ( depth_sel_push == 3 ) ? ( push_diff >= DEPTH3-8 ) : ( depth_sel_push == 2 ) ? ( push_diff >= DEPTH2-8 ) : ( push_diff >= DEPTH1-8 );
	
	assign	match_one_left				= ( pop_diff == 1 );
	assign	match_one2eight_left	= ( pop_diff < 8 );

	assign	push_flush_gated	= Fifo_Push_Flush & ~push_local_flush_mask;
	assign	pop_flush_gated		= Fifo_Pop_Flush & ~pop_local_flush_mask;
	
	assign	push_clk_rst	= flush_pop2push_clk1 ^ pop_flush_tf_push_clk;
	assign	pop_clk_rst		= flush_push2pop_clk1 ^ push_flush_tf_pop_clk;
	
	assign	pop_flush_done	= push2pop_ack1 ^ push2pop_ack2;
	assign	push_flush_done	= pop2push_ack1 ^ pop2push_ack2;
	
	assign	push_clk_rst_mux	= ( Fifo_Sync_Mode ) ? ( Fifo_Push_Flush | Fifo_Pop_Flush ) : ( push_flush_gated | push_clk_rst );
	assign	pop_clk_rst_mux		= ( Fifo_Sync_Mode ) ? ( Fifo_Push_Flush | Fifo_Pop_Flush ) : ( pop_flush_gated | ( pop_local_flush_mask & ~pop_flush_done ) | pop_clk_rst );
	
	reg match_room_at_most63, match_at_most63_left;
	
	always@( push_diff or push_diff_plus_1 or depth_sel_push or match_room4none or match_room4one )
	begin
		if( depth_sel_push == 1 ) 
		begin
			match_room4none		<= ( push_diff[D1_QTR_A+2:0] == depth1_reg[D1_QTR_A+2:0] );

			match_room4one		<= ( push_diff_plus_1[D1_QTR_A+2:0] == depth1_reg ) | match_room4none;

			match_room4half		<= ( push_diff[D1_QTR_A+1] == 1'b1 );
			match_room4quart	<= ( push_diff[D1_QTR_A] == 1'b1 );
			
			match_room_at_most63    <=  push_diff[6];
		end
		else if( depth_sel_push == 2 ) 
		begin
			match_room4none		<= ( push_diff[D2_QTR_A+2:0] == depth2_reg[D2_QTR_A+2:0] );

			match_room4one		<= ( push_diff_plus_1[D2_QTR_A+2:0] == depth2_reg ) | match_room4none;

			match_room4half		<= ( push_diff[D2_QTR_A+1] == 1'b1 );
			match_room4quart	<= ( push_diff[D2_QTR_A] == 1'b1 );
			
			match_room_at_most63    <=  &push_diff[7:6];
		end
		else  
		begin
			match_room4none		<= ( push_diff == depth3_reg );
			match_room4one		<= ( push_diff_plus_1 == depth3_reg ) | match_room4none;

			match_room4half		<= ( push_diff[D3_QTR_A+1] == 1'b1 );
			match_room4quart	<= ( push_diff[D3_QTR_A] == 1'b1 );
			
			match_room_at_most63	<= &push_diff[8:6];
		end
	end
	
	assign room4_32s = ~push_diff[5];
	assign room4_16s = ~push_diff[4];
	assign room4_8s  = ~push_diff[3];
	assign room4_4s  = ~push_diff[2];
	assign room4_2s  = ~push_diff[1];
	assign room4_1s  = &push_diff[1:0];				
	
	always@( depth_sel_pop or pop_diff )
	begin
		if( depth_sel_pop == 1 ) 
		begin
			match_all_left		<= ( pop_diff[D1_QTR_A+2:0] == depth1_reg[D1_QTR_A+2:0] );

			match_half_left		<= ( pop_diff[D1_QTR_A+1] == 1'b1 );
			match_quart_left	<= ( pop_diff[D1_QTR_A] == 1'b1 );
			
			match_at_most63_left	<= ~pop_diff[6];
		end
		else if( depth_sel_pop == 2 ) 
		begin
			match_all_left		<= ( pop_diff[D2_QTR_A+2:0] == depth2_reg[D2_QTR_A+2:0] );

			match_half_left		<= ( pop_diff[D2_QTR_A+1] == 1'b1 );
			match_quart_left	<= ( pop_diff[D2_QTR_A] == 1'b1 );
			
			match_at_most63_left	<= ~|pop_diff[7:6];			
		end
		else  
		begin
			match_all_left		<= ( pop_diff == depth3_reg );

			match_half_left		<= ( pop_diff[D3_QTR_A+1] == 1'b1 );
			match_quart_left	<= ( pop_diff[D3_QTR_A] == 1'b1 );
			
			match_at_most63_left	<= ~|pop_diff[8:6];			
		end
	end

	assign at_least_32 = pop_diff[5];
	assign at_least_16 = pop_diff[4];
	assign at_least_8 = pop_diff[3];
	assign at_least_4 = pop_diff[2];
	assign at_least_2 = pop_diff[1];
	assign one_left = pop_diff[0];
	
	always@( posedge Pop_Clk or negedge Rst_n )
	begin
		if( ~Rst_n )
		begin
			push2pop_ack1 <= 1'b0;
			push2pop_ack2 <= 1'b0;
			flush_pop_clk_tf <= 1'b0;
			pop_local_flush_mask <= 1'b0;
			flush_push2pop_clk1 <= 1'b0;
			push_flush_tf_pop_clk <= 1'b0;
		end
		else
		begin
			push2pop_ack1 <= pop_flush_tf_push_clk;
			push2pop_ack2 <= push2pop_ack1;
			flush_push2pop_clk1 <= flush_push_clk_tf;
			if( pop_flush_gated )
			begin
				flush_pop_clk_tf	<= ~flush_pop_clk_tf;
			end
	
			if( pop_flush_gated & ~Fifo_Sync_Mode )
			begin
				pop_local_flush_mask	<= 1'b1;
			end
			else if( pop_flush_done )
			begin
				pop_local_flush_mask	<= 1'b0;
			end
	
			if( pop_clk_rst )
			begin
				push_flush_tf_pop_clk	<= ~push_flush_tf_pop_clk;
			end
		end
	end

	always@( posedge Push_Clk or negedge Rst_n )
	begin
		if( ~Rst_n )
		begin
			pop2push_ack1 <= 1'b0;
			pop2push_ack2 <= 1'b0;
			flush_push_clk_tf <= 1'b0;
			push_local_flush_mask <= 1'b0;
			flush_pop2push_clk1 <= 1'b0;
			pop_flush_tf_push_clk <= 1'b0;
		end
		else
		begin
			pop2push_ack1				<= push_flush_tf_pop_clk;
			pop2push_ack2				<= pop2push_ack1;
			flush_pop2push_clk1	<= flush_pop_clk_tf;
			if( push_flush_gated )
			begin
				flush_push_clk_tf	<= ~flush_push_clk_tf;
			end
	
			if( push_flush_gated & ~Fifo_Sync_Mode )
			begin
				push_local_flush_mask	<= 1'b1;
			end
			else if( push_flush_done )
			begin
				push_local_flush_mask	<= 1'b0;
			end
			
			if( push_clk_rst )
			begin
				pop_flush_tf_push_clk	<= ~pop_flush_tf_push_clk;
			end
		end
	end

	always@( Fifo_Push_Width or Fifo_Pop_Width or pop_ptr_push_clk_l_shift1 or pop_ptr_push_clk_l_shift2 or pop_ptr_push_clk_r_shift1 or
						pop_ptr_push_clk_r_shift2 or push_ptr_pop_clk_l_shift1 or push_ptr_pop_clk_l_shift2 or push_ptr_pop_clk_r_shift1 or push_ptr_pop_clk_r_shift2 or
						pop_ptr_push_clk or push_ptr_pop_clk )
	begin
		case( { Fifo_Push_Width, Fifo_Pop_Width } )
			4'b0001:	
      begin
      	push_ptr_push_clk_mask	<= 2'b11;
				pop_ptr_pop_clk_mask		<= 2'b01;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk_l_shift1;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk_r_shift1;
			end
			4'b0010:	
      begin
      	push_ptr_push_clk_mask	<= 2'b11;
				pop_ptr_pop_clk_mask		<= 2'b00;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk_l_shift2;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk_r_shift2;
			end
			4'b0100:	
      begin
      	push_ptr_push_clk_mask	<= 2'b01;
				pop_ptr_pop_clk_mask		<= 2'b11;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk_r_shift1;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk_l_shift1;
			end
      4'b0110:	
      begin
      	push_ptr_push_clk_mask	<= 2'b11;
				pop_ptr_pop_clk_mask		<= 2'b01;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk_l_shift1;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk_r_shift1;
			end
			4'b1000:	
      begin
      	push_ptr_push_clk_mask	<= 2'b00;
				pop_ptr_pop_clk_mask		<= 2'b11;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk_r_shift2;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk_l_shift2;
			end
			4'b1001:	
      begin
      	push_ptr_push_clk_mask	<= 2'b01;
				pop_ptr_pop_clk_mask		<= 2'b11;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk_r_shift1;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk_l_shift1;
			end
      default:	
      begin
      	push_ptr_push_clk_mask	<= 2'b11;
				pop_ptr_pop_clk_mask		<= 2'b11;
				pop_ptr_push_clk_mux		<= pop_ptr_push_clk;
				push_ptr_pop_clk_mux		<= push_ptr_pop_clk;
			end
  	endcase
	end
	
	always@( Fifo_Ram_Mode or Fifo_Push_Width )
	begin
		if( Fifo_Ram_Mode == Fifo_Push_Width[0] )
		begin
			depth_sel_push	<= 2;	
		end
		else if( Fifo_Ram_Mode == Fifo_Push_Width[1] )
		begin
			depth_sel_push	<= 1;	
		end
		else
		begin
			depth_sel_push	<= 3;	
		end
	end

	always@( Fifo_Ram_Mode or Fifo_Pop_Width )
	begin
		if( Fifo_Ram_Mode == Fifo_Pop_Width[0] )
		begin
			depth_sel_pop	<= 2;	
		end
		else if( Fifo_Ram_Mode == Fifo_Pop_Width[1] )
		begin
			depth_sel_pop	<= 1;	
		end
		else
		begin
			depth_sel_pop	<= 3;	
		end
	end

	always@( posedge Push_Clk or negedge Rst_n )
	begin
		if( ~Rst_n )
		begin
			push_ptr_push_clk	<= 0;
			pop_ptr_push_clk	<= 0;
			pop_ptr_async			<= 0;
			fifo_full_flag_f	<= 0;
		end
		else
		begin
			if( push_clk_rst_mux )
			begin
				push_ptr_push_clk	<= 0;
				pop_ptr_push_clk	<= 0;
				pop_ptr_async			<= 0;
				fifo_full_flag_f	<= 0;
			end
			else
			begin
				push_ptr_push_clk	<= next_push_ptr_push_clk; 
				pop_ptr_push_clk	<= ( Fifo_Sync_Mode ) ? next_pop_ptr_pop_clk : pop_ptr_async;
				pop_ptr_async			<= pop_ptr_pop_clk;
				fifo_full_flag_f	<= match_room4one | match_room4none;
			end
		end
	end

	always@( posedge Pop_Clk or negedge Rst_n )
	begin
		if( ~Rst_n )
		begin
			pop_ptr_pop_clk		<= 0;
			push_ptr_pop_clk	<= 0;
			push_ptr_async		<= 0;
			fifo_empty_flag_f	<= 1;
		end
		else
		begin
			if( pop_clk_rst_mux )
			begin
				pop_ptr_pop_clk		<= 0;
				push_ptr_pop_clk	<= 0;
				push_ptr_async		<= 0;
				fifo_empty_flag_f	<= 1;
			end
			else
			begin
				pop_ptr_pop_clk		<= next_pop_ptr_pop_clk;
				push_ptr_pop_clk	<= ( Fifo_Sync_Mode ) ? next_push_ptr_push_clk : push_ptr_async;
				push_ptr_async		<= push_ptr_push_clk;
				fifo_empty_flag_f	<= ( pop_diff == 1 ) | ( pop_diff == 0 );
			end
		end
	end	

	always@( posedge Push_Clk or negedge Rst_n )
	begin
		if( ~Rst_n )
		begin
 		    Fifo_Full_Usr	<= 4'b0001;
		end
		else
		begin
			if( match_room4none )
			begin
				Fifo_Full_Usr	<= 4'b0000;
			end
			else if( match_room4all )
			begin
				Fifo_Full_Usr	<= 4'b0001;
			end
			else if( ~match_room4half )
			begin
				Fifo_Full_Usr	<= 4'b0010;
			end
			else if( ~match_room4quart )
			begin
				Fifo_Full_Usr	<= 4'b0011;
			end
			else 
				begin
				if (match_room_at_most63)
					begin
					if (room4_32s)
						Fifo_Full_Usr <= 4'b1010;
					else if (room4_16s)
						Fifo_Full_Usr <= 4'b1011;
					else if (room4_8s)
						Fifo_Full_Usr <= 4'b1100;
					else if (room4_4s)
						Fifo_Full_Usr <= 4'b1101;
					else if (room4_2s)
						Fifo_Full_Usr <= 4'b1110;
					else if (room4_1s)
						Fifo_Full_Usr <= 4'b1111;
					else
						Fifo_Full_Usr <= 4'b1110;
					end
				else
					Fifo_Full_Usr <= 4'b0100;
				end
		end
	end

	always@( posedge Pop_Clk or negedge Rst_n )
	begin
		if( ~Rst_n )
		begin
			Fifo_Empty_Usr	<= 4'b0000;
		end
		else
		begin
			if( Fifo_Pop_Flush | ( pop_local_flush_mask & ~pop_flush_done ) | pop_clk_rst )
			begin
				Fifo_Empty_Usr	<= 4'b0000;
			end
			else 
			if( match_all_left )
			begin
				Fifo_Empty_Usr	<= 4'b1111;
			end
			else if( match_half_left )
			begin
				Fifo_Empty_Usr	<= 4'b1110;
			end
			else if( match_quart_left )
			begin
				Fifo_Empty_Usr	<= 4'b1101;
			end
			else 
				begin
				if (match_at_most63_left)
					begin
					if (at_least_32)
						Fifo_Empty_Usr	<= 4'b0110;
					else if	(at_least_16)
						Fifo_Empty_Usr	<= 4'b0101;					
					else if	(at_least_8)
						Fifo_Empty_Usr	<= 4'b0100;					
					else if	(at_least_4)
						Fifo_Empty_Usr	<= 4'b0011;					
					else if	(at_least_2)
						Fifo_Empty_Usr	<= 4'b0010;					
					else if	(one_left)
						Fifo_Empty_Usr	<= 4'b0001;
					else Fifo_Empty_Usr	<= 4'b0000;
					end
				else
					Fifo_Empty_Usr	<= 4'b1000;
				end
		end
	end
endmodule

`timescale 10 ps /1 ps

`define DATAWID 18 
`define WEWID 2

module ram(
						AA,
						AB,
						CLKA,
						CLKB,
						WENA,
						WENB,
						CENA,
						CENB,
						WENBA,
						WENBB,
						DA,
						QA,
						DB,
						QB
					);


parameter ADDRWID = 8;
parameter DEPTH = (1<<ADDRWID);

	output	[`DATAWID-1:0]	QA;
	input										CLKA;
	input										CENA;
	input										WENA;
	input		[`WEWID-1:0]		WENBA;
	input		[ADDRWID-1:0]	AA;
	input		[`DATAWID-1:0]	DA;
	output	[`DATAWID-1:0]	QB;
	
	input										CLKB;
	input										CENB;
	input										WENB;
	input		[`WEWID-1:0]		WENBB;
	input		[ADDRWID-1:0]	AB;
	input		[`DATAWID-1:0]	DB;
	
	integer	i, j, k, l, m;

	wire									CEN1;
	wire									OEN1;
	wire									WEN1;
	wire	[`WEWID-1:0]		WENB1;
	wire	[ADDRWID-1:0]	A1;

	reg	[ADDRWID-1:0]	AddrOut1;
	wire	[`DATAWID-1:0]	I1;
	
	wire									CEN2;
	wire									OEN2;
	wire									WEN2;
	wire	[`WEWID-1:0]		WENB2;
	wire	[ADDRWID-1:0]	A2;

	reg	[ADDRWID-1:0]	AddrOut2;
	wire	[`DATAWID-1:0]	I2;
	
	reg		[`DATAWID-1:0]	O1, QAreg;
	reg		[`DATAWID-1:0]	O2, QBreg;
	
	reg										WEN1_f;
	reg										WEN2_f;
	reg	[ADDRWID-1:0]	A2_f;
	reg	[ADDRWID-1:0]	A1_f;
	
	wire									CEN1_SEL;
	wire									WEN1_SEL;
	wire	[ADDRWID-1:0]	A1_SEL;
	wire	[`DATAWID-1:0]	I1_SEL;
	wire	[`WEWID-1:0]		WENB1_SEL;
	
	wire									CEN2_SEL;
	wire									WEN2_SEL;
	wire	[ADDRWID-1:0]	A2_SEL;
	wire	[`DATAWID-1:0]	I2_SEL;
	wire	[`WEWID-1:0]		WENB2_SEL;
	wire  overlap;
	
	wire CLKA_d, CLKB_d, CEN1_d, CEN2_d;
	
	assign	A1_SEL    = AA;
	assign	I1_SEL    = DA;
	assign	CEN1_SEL  = CENA;
	assign	WEN1_SEL  = WENA;
	assign	WENB1_SEL = WENBA;
	
	assign	A2_SEL    = AB;
	assign	I2_SEL    = DB;
	assign	CEN2_SEL  = CENB;
	assign	WEN2_SEL  = WENB;
	assign	WENB2_SEL = WENBB;
	
	assign	CEN1	= CEN1_SEL;
	assign	OEN1	= 1'b0;                           
	assign	WEN1	= WEN1_SEL;
	assign	WENB1	= WENB1_SEL;
	assign	A1		= A1_SEL;
	assign	I1		= I1_SEL;
	
	assign	CEN2	= CEN2_SEL;
	assign	OEN2	= 1'b0;
	assign	WEN2	= WEN2_SEL;
	assign	WENB2	= WENB2_SEL;
	assign	A2		= A2_SEL;
	assign	I2		= I2_SEL;

	reg		[`DATAWID-1:0]	ram[DEPTH-1:0];
	reg		[`DATAWID-1:0]	wrData1;
	reg		[`DATAWID-1:0]	wrData2;
	wire	[`DATAWID-1:0]	tmpData1;
	wire	[`DATAWID-1:0]	tmpData2;
	
reg CENreg1, CENreg2;

assign #1 CLKA_d = CLKA;
assign #1 CLKB_d = CLKB;

assign #2 CEN1_d = CEN1;
assign #2 CEN2_d = CEN2;

assign	QA = QAreg | O1;
assign	QB = QBreg | O2;

	assign	tmpData1	= ram[A1];
	assign	tmpData2	= ram[A2];
	
	assign	overlap	= ( A1_f == A2_f ) & WEN1_f & WEN2_f;
	
	initial
	begin
		for( i = 0; i < DEPTH; i = i+1 )
		begin
			ram[i]	= 18'bxxxxxxxxxxxxxxxxxx;
		end
	end
	
	always@( WENB1 or I1 or tmpData1 )
	begin
		for( j = 0; j < 9; j = j+1 )
		begin
			wrData1[j]	<= ( WENB1[0] ) ? tmpData1[j] : I1[j];
		end
		for( l = 9; l < 19; l = l+1 )
		begin
			wrData1[l]	<= ( WENB1[1] ) ? tmpData1[l] : I1[l];
		end
	end
	
	always@( posedge CLKA )
	begin
		if( ~WEN1 & ~CEN1 )
		begin
			ram[A1]	<= wrData1[`DATAWID-1:0];
		end
	end
	
	always@( posedge CLKA_d)
    if(~CEN1_d)
	    begin
	      O1	= 18'h3ffff;
        #100;
		    O1	= 18'h00000;
		end
	

	always@( posedge CLKA )
		if (~CEN1)
			begin
			AddrOut1 <= A1;
			end

	always@( posedge CLKA_d)
		if (~CEN1_d)
			begin
			QAreg <= ram[AddrOut1];
			end


	always@( posedge CLKA )
	begin
		WEN1_f	<= ~WEN1 & ~CEN1;
		A1_f<= A1;
		
	end
	
	always@( WENB2 or I2 or tmpData2 )
	begin
		for( k = 0; k < 9; k = k+1 )
		begin
			wrData2[k]	<= ( WENB2[0] ) ? tmpData2[k] : I2[k];
		end
		for( m = 9; m < 19; m = m+1 )
		begin
			wrData2[m]	<= ( WENB2[1] ) ? tmpData2[m] : I2[m];
		end
	end
	
	always@( posedge CLKB )
	begin
		if( ~WEN2 & ~CEN2 )
		begin
			ram[A2]	<= wrData2[`DATAWID-1:0];
		end
	end

	always@( posedge CLKB_d )
    if(~CEN2_d)
	    begin
	      O2	= 18'h3ffff;
        #100;
		    O2	= 18'h00000;
		end

	always@( posedge CLKB )
		if (~CEN2)
			begin
			AddrOut2 <= A2;
			end

	always@( posedge CLKB_d )
		if (~CEN2_d)
			begin
			QBreg <= ram[AddrOut2];
			end

	always@( posedge CLKB )
	begin
		WEN2_f	<= ~WEN2 & ~CEN2;
		A2_f<=A2;
		
	end

	always@( A1_f or A2_f or overlap)
	begin
		if( overlap )
		begin
			ram[A1_f]	<= 18'bxxxxxxxxxxxxxxxxxx;
		end
	end

endmodule

`timescale 1 ns /10 ps
`define DATAWID 18
`define WEWID 2
module x2_model(
									Concat_En,
									
									ram0_WIDTH_SELA,
									ram0_WIDTH_SELB,
									ram0_PLRD,
									
									ram0_CEA,
									ram0_CEB,
									ram0_I,
									ram0_O,
									ram0_AA,
									ram0_AB,
									ram0_CSBA,
									ram0_CSBB,
									ram0_WENBA,
									
									ram1_WIDTH_SELA,
									ram1_WIDTH_SELB,
									ram1_PLRD,
									
									ram1_CEA,
									ram1_CEB,
									ram1_I,
									ram1_O,
									ram1_AA,
									ram1_AB,
									ram1_CSBA,
									ram1_CSBB,
									ram1_WENBA
								);

parameter ADDRWID = 10;								

	input										Concat_En;      
	
	input		[1:0]						ram0_WIDTH_SELA;
	input		[1:0]						ram0_WIDTH_SELB;
	input										ram0_PLRD;      
	input										ram0_CEA;
	input										ram0_CEB;
	input		[`DATAWID-1:0]	ram0_I;
	output	[`DATAWID-1:0]	ram0_O;
	input		[ADDRWID-1:0]	ram0_AA;
	input		[ADDRWID-1:0]	ram0_AB;
	input										ram0_CSBA;
	input										ram0_CSBB;
	input		[`WEWID-1:0]		ram0_WENBA;
	
	input		[1:0]						ram1_WIDTH_SELA;
	input		[1:0]						ram1_WIDTH_SELB;
	input										ram1_PLRD;
	input										ram1_CEA;
	input										ram1_CEB;
	input		[`DATAWID-1:0]	ram1_I;
	output	[`DATAWID-1:0]	ram1_O;
	input		[ADDRWID-1:0]	ram1_AA;
	input		[ADDRWID-1:0]	ram1_AB;
	input										ram1_CSBA;
	input										ram1_CSBB;
	input 	[`WEWID-1:0]		ram1_WENBA;
	
	reg										ram0_PLRDA_SEL;
	reg										ram0_PLRDB_SEL;
	reg										ram1_PLRDA_SEL;
	reg										ram1_PLRDB_SEL;
	reg										ram_AA_ram_SEL; 
	reg										ram_AB_ram_SEL;

	reg		[`WEWID-1:0]		ram0_WENBA_SEL;
	reg		[`WEWID-1:0]		ram0_WENBB_SEL;
	reg		[`WEWID-1:0]		ram1_WENBA_SEL;
	reg		[`WEWID-1:0]		ram1_WENBB_SEL;
	
  reg										ram0_A_x9_SEL;
  reg										ram0_B_x9_SEL;
  reg										ram1_A_x9_SEL;
  reg										ram1_B_x9_SEL;
  
	reg		[ADDRWID-3:0]	ram0_AA_SEL;
	reg		[ADDRWID-3:0]	ram0_AB_SEL;
	reg		[ADDRWID-3:0]	ram1_AA_SEL;
	reg		[ADDRWID-3:0]	ram1_AB_SEL;

	reg										ram0_AA_byte_SEL;
	reg										ram0_AB_byte_SEL;
	reg										ram1_AA_byte_SEL;
	reg										ram1_AB_byte_SEL;
	
	reg										ram0_AA_byte_SEL_Q;
	reg										ram0_AB_byte_SEL_Q;
	reg										ram1_AA_byte_SEL_Q;
	reg										ram1_AB_byte_SEL_Q;
	reg										ram0_A_mux_ctl_Q;
	reg										ram0_B_mux_ctl_Q;
	reg										ram1_A_mux_ctl_Q;
	reg										ram1_B_mux_ctl_Q;
	
  reg										ram0_O_mux_ctrl_Q;
  reg										ram1_O_mux_ctrl_Q;
  
	reg										ram_AA_ram_SEL_Q;
	reg										ram_AB_ram_SEL_Q;

	wire	[`DATAWID-1:0]	QA_1_SEL3;
	wire	[`DATAWID-1:0]	QB_0_SEL2;
	wire	[`DATAWID-1:0]	QB_1_SEL2;
  
	reg		[`DATAWID-1:0]	QA_0_Q;
	reg		[`DATAWID-1:0]	QB_0_Q;
	reg		[`DATAWID-1:0]	QA_1_Q;
	reg		[`DATAWID-1:0]	QB_1_Q;
	
	wire	[`DATAWID-1:0]	QA_0;
	wire	[`DATAWID-1:0]	QB_0;
	wire	[`DATAWID-1:0]	QA_1;
	wire	[`DATAWID-1:0]	QB_1;

	wire									ram0_CSBA_SEL;
	wire									ram0_CSBB_SEL;
	wire									ram1_CSBA_SEL;
	wire									ram1_CSBB_SEL;
	
	wire	[`DATAWID-1:0]	ram0_I_SEL1;
	wire	[`DATAWID-1:0]	ram1_I_SEL1;
	
	wire									dual_port;
	
	wire									ram0_WEBA_SEL;
	wire									ram0_WEBB_SEL;
	wire									ram1_WEBA_SEL;
	wire									ram1_WEBB_SEL;
	
	wire	[`DATAWID-1:0]	ram1_I_SEL2;
	
	wire	[`DATAWID-1:0]	QA_1_SEL2;
	wire	[`DATAWID-1:0]	QA_0_SEL1;
	wire	[`DATAWID-1:0]	QB_0_SEL1;
	wire	[`DATAWID-1:0]	QA_1_SEL1;
	wire	[`DATAWID-1:0]	QB_1_SEL1;

	wire	[`DATAWID-1:0]	QB_0_SEL3;
	wire	[`DATAWID-1:0]	QA_0_SEL2;

	initial
	begin
		QA_0_Q							<= 0;
		QB_0_Q							<= 0;
		QA_1_Q							<= 0;
		QB_1_Q							<= 0;
		ram0_AA_byte_SEL_Q	<= 0;
		ram0_A_mux_ctl_Q		<= 0;
		ram0_AB_byte_SEL_Q	<= 0;
		ram0_B_mux_ctl_Q		<= 0;
		ram1_AA_byte_SEL_Q	<= 0;
		ram1_A_mux_ctl_Q		<= 0;
		ram1_AB_byte_SEL_Q	<= 0;
		ram1_B_mux_ctl_Q		<= 0;
		ram_AA_ram_SEL_Q		<= 0;
		ram1_O_mux_ctrl_Q		<= 0;
		ram_AB_ram_SEL_Q		<= 0;
		ram0_O_mux_ctrl_Q		<= 0;
	end

	assign dual_port	= Concat_En & ~( ram0_WIDTH_SELA[1] | ram0_WIDTH_SELB[1] );
	
	assign ram0_CSBA_SEL	= ram0_CSBA;
	assign ram0_CSBB_SEL	= ram0_CSBB;
	assign ram1_CSBA_SEL	= Concat_En ? ram0_CSBA : ram1_CSBA;
	assign ram1_CSBB_SEL	= Concat_En ? ram0_CSBB : ram1_CSBB;

	assign ram0_O = QB_0_SEL3;
	assign ram1_O = dual_port ? QA_1_SEL3 : QB_1_SEL2;
	
	assign ram0_I_SEL1[8:0]		= ram0_I[8:0];
	assign ram1_I_SEL1[8:0]		= ram1_I[8:0];
	assign ram0_I_SEL1[17:9]	= ram0_AA_byte_SEL ? ram0_I[8:0] : ram0_I[17:9];
	assign ram1_I_SEL1[17:9]	= ( ( ~Concat_En & ram1_AA_byte_SEL ) | ( dual_port & ram0_AB_byte_SEL ) ) ? ram1_I[8:0] : ram1_I[17:9];
	
	assign ram1_I_SEL2	= ( Concat_En & ~ram0_WIDTH_SELA[1] ) ? ram0_I_SEL1 : ram1_I_SEL1;
	
	assign ram0_WEBA_SEL	= &ram0_WENBA_SEL;
	assign ram0_WEBB_SEL	= &ram0_WENBB_SEL;
	assign ram1_WEBA_SEL	= &ram1_WENBA_SEL;
	assign ram1_WEBB_SEL	= &ram1_WENBB_SEL;

	assign QA_0_SEL1	= ( ram0_PLRDA_SEL ) ? QA_0_Q : QA_0 ;
	assign QB_0_SEL1	= ( ram0_PLRDB_SEL ) ? QB_0_Q : QB_0 ;
	assign QA_1_SEL1	= ( ram1_PLRDA_SEL ) ? QA_1_Q : QA_1 ;
	assign QB_1_SEL1	= ( ram1_PLRDB_SEL ) ? QB_1_Q : QB_1 ;
	
    assign QA_1_SEL3	= ram1_O_mux_ctrl_Q ? QA_1_SEL2 : QA_0_SEL2;
	
	assign QA_0_SEL2[8:0]	= ram0_A_mux_ctl_Q ? QA_0_SEL1[17:9] : QA_0_SEL1[8:0] ;
	assign QB_0_SEL2[8:0]	= ram0_B_mux_ctl_Q ? QB_0_SEL1[17:9] : QB_0_SEL1[8:0] ;
	assign QA_1_SEL2[8:0]	= ram1_A_mux_ctl_Q ? QA_1_SEL1[17:9] : QA_1_SEL1[8:0] ;
	assign QB_1_SEL2[8:0]	= ram1_B_mux_ctl_Q ? QB_1_SEL1[17:9] : QB_1_SEL1[8:0] ;
	
	assign QA_0_SEL2[17:9]	= QA_0_SEL1[17:9];
	assign QB_0_SEL2[17:9]	= QB_0_SEL1[17:9];
	assign QA_1_SEL2[17:9]	= QA_1_SEL1[17:9];
	assign QB_1_SEL2[17:9]	= QB_1_SEL1[17:9];

	assign QB_0_SEL3 = ram0_O_mux_ctrl_Q ? QB_1_SEL2 : QB_0_SEL2;
	
	always@( posedge ram0_CEA )
	begin
		QA_0_Q <= QA_0;
	end
	always@( posedge ram0_CEB )
	begin
		QB_0_Q <= QB_0;
	end
	always@( posedge ram1_CEA )
	begin
		QA_1_Q <= QA_1;
	end
	always@( posedge ram1_CEB )
	begin
		QB_1_Q <= QB_1;
	end

	always@( posedge ram0_CEA )
	begin
		if( ram0_CSBA_SEL == 0 )
			ram0_AA_byte_SEL_Q	<= ram0_AA_byte_SEL;
		if( ram0_PLRDA_SEL || ( ram0_CSBA_SEL == 0 ) )
			ram0_A_mux_ctl_Q	<= ram0_A_x9_SEL & ( ram0_PLRDA_SEL ? ram0_AA_byte_SEL_Q : ram0_AA_byte_SEL );
	end
	
	always@( posedge ram0_CEB)
	begin
		if( ram0_CSBB_SEL == 0 )
			ram0_AB_byte_SEL_Q	<= ram0_AB_byte_SEL;
		if( ram0_PLRDB_SEL || ( ram0_CSBB_SEL == 0 ) )
			ram0_B_mux_ctl_Q	<= ram0_B_x9_SEL & ( ram0_PLRDB_SEL ? ram0_AB_byte_SEL_Q : ram0_AB_byte_SEL );
	end
	
	always@( posedge ram1_CEA )
	begin
		if( ram1_CSBA_SEL == 0 )
			ram1_AA_byte_SEL_Q	<= ram1_AA_byte_SEL;
		if( ram1_PLRDA_SEL || (ram1_CSBA_SEL == 0 ) )
			ram1_A_mux_ctl_Q	<= ram1_A_x9_SEL & ( ram1_PLRDA_SEL ? ram1_AA_byte_SEL_Q : ram1_AA_byte_SEL );
	end
	
	always@( posedge ram1_CEB )
	begin
		if( ram1_CSBB_SEL == 0 )
			ram1_AB_byte_SEL_Q	<= ram1_AB_byte_SEL;
		if( ram1_PLRDB_SEL || (ram1_CSBB_SEL == 0 ) )
			ram1_B_mux_ctl_Q	<= ram1_B_x9_SEL & ( ram1_PLRDB_SEL ? ram1_AB_byte_SEL_Q : ram1_AB_byte_SEL );
	end

	always@( posedge ram0_CEA )
	begin
		ram_AA_ram_SEL_Q	<= ram_AA_ram_SEL;
		ram1_O_mux_ctrl_Q	<= ( ram0_PLRDA_SEL ? ram_AA_ram_SEL_Q : ram_AA_ram_SEL );
	end

	always@( posedge ram0_CEB )
	begin
		ram_AB_ram_SEL_Q	<= ram_AB_ram_SEL;
		ram0_O_mux_ctrl_Q	<= ( ram0_PLRDB_SEL ? ram_AB_ram_SEL_Q : ram_AB_ram_SEL );
	end

	always@( Concat_En or ram0_WIDTH_SELA or ram0_WIDTH_SELB or ram0_AA or ram0_AB or ram0_WENBA or 
	         ram1_AA or ram1_AB or ram1_WENBA or ram0_PLRD or ram1_PLRD or ram1_WIDTH_SELA or ram1_WIDTH_SELB ) 
	begin
		ram0_A_x9_SEL			<= ( ~|ram0_WIDTH_SELA );
		ram1_A_x9_SEL			<= ( ~|ram0_WIDTH_SELA );
		ram0_B_x9_SEL			<= ( ~|ram0_WIDTH_SELB );
		ram0_AA_byte_SEL	<= ram0_AA[0] & ( ~|ram0_WIDTH_SELA );
		ram0_AB_byte_SEL	<= ram0_AB[0] & ( ~|ram0_WIDTH_SELB );
		if( ~Concat_En )
		begin
			ram_AA_ram_SEL	<= 1'b0;
			ram_AB_ram_SEL	<= 1'b0;
			ram1_B_x9_SEL		<= ( ~|ram1_WIDTH_SELB );
			
			ram0_PLRDA_SEL	<= ram0_PLRD;
			ram0_PLRDB_SEL	<= ram0_PLRD;
			ram1_PLRDA_SEL	<= ram1_PLRD;
			ram1_PLRDB_SEL	<= ram1_PLRD;
			ram0_WENBB_SEL	<= {`WEWID{1'b1}};
			ram1_WENBB_SEL	<= {`WEWID{1'b1}};
			
			ram0_AA_SEL				<= ram0_AA >> ( ~|ram0_WIDTH_SELA );
			ram0_WENBA_SEL[0]	<= ( ram0_AA[0] & ( ~|ram0_WIDTH_SELA ) ) | ram0_WENBA[0];
			ram0_WENBA_SEL[1]	<= ( ~ram0_AA[0] & ( ~|ram0_WIDTH_SELA ) ) | ram0_WENBA[( |ram0_WIDTH_SELA )];
			ram0_AB_SEL				<= ram0_AB >> ( ~|ram0_WIDTH_SELB );

			ram1_AA_SEL				<= ram1_AA >> ( ~|ram1_WIDTH_SELA );
			ram1_AA_byte_SEL	<= ram1_AA[0] & ( ~|ram1_WIDTH_SELA );
			ram1_WENBA_SEL[0]	<= ( ram1_AA[0] & ( ~|ram1_WIDTH_SELA ) ) | ram1_WENBA[0];
			ram1_WENBA_SEL[1]	<= ( ~ram1_AA[0] & ( ~|ram1_WIDTH_SELA ) ) | ram1_WENBA[( |ram1_WIDTH_SELA )];
			ram1_AB_SEL				<= ram1_AB >> ( ~|ram1_WIDTH_SELB );
			ram1_AB_byte_SEL	<= ram1_AB[0] & ( ~|ram1_WIDTH_SELB );
		end
		else
		begin
			ram_AA_ram_SEL	<= ~ram0_WIDTH_SELA[1] & ram0_AA[~ram0_WIDTH_SELA[0]];
			ram_AB_ram_SEL	<= ~ram0_WIDTH_SELB[1] & ram0_AB[~ram0_WIDTH_SELB[0]];
			ram1_B_x9_SEL	<= ( ~|ram0_WIDTH_SELB );

			ram0_PLRDA_SEL	<= ram1_PLRD;
			ram1_PLRDA_SEL	<= ram1_PLRD;
			ram0_PLRDB_SEL	<= ram0_PLRD;
			ram1_PLRDB_SEL	<= ram0_PLRD;
			
			ram0_AA_SEL				<= ram0_AA >> { ~ram0_WIDTH_SELA[1] & ~( ram0_WIDTH_SELA[1] ^ ram0_WIDTH_SELA[0] ), ~ram0_WIDTH_SELA[1] & ram0_WIDTH_SELA[0] };
			ram1_AA_SEL				<= ram0_AA >> { ~ram0_WIDTH_SELA[1] & ~( ram0_WIDTH_SELA[1] ^ ram0_WIDTH_SELA[0] ), ~ram0_WIDTH_SELA[1] & ram0_WIDTH_SELA[0] };
			ram1_AA_byte_SEL	<= ram0_AA[0] & ( ~|ram0_WIDTH_SELA );
			ram0_WENBA_SEL[0]	<= ram0_WENBA[0] | ( ~ram0_WIDTH_SELA[1] & ( ram0_AA[0] | ( ~ram0_WIDTH_SELA[0] & ram0_AA[1] ) ) );
			ram0_WENBA_SEL[1]	<= ( ( ~|ram0_WIDTH_SELA & ram0_WENBA[0] ) | ( |ram0_WIDTH_SELA & ram0_WENBA[1] ) ) | ( ~ram0_WIDTH_SELA[1] & ( ( ram0_WIDTH_SELA[0] & ram0_AA[0] ) | ( ~ram0_WIDTH_SELA[0] & ~ram0_AA[0] ) | ( ~ram0_WIDTH_SELA[0] & ram0_AA[1] ) ) );

			ram1_WENBA_SEL[0]	<= ( ( ~ram0_WIDTH_SELA[1] & ram0_WENBA[0] ) | ( ram0_WIDTH_SELA[1] & ram1_WENBA[0] ) ) | ( ~ram0_WIDTH_SELA[1] & ( ( ram0_WIDTH_SELA[0] & ~ram0_AA[0] ) | ( ~ram0_WIDTH_SELA[0] & ram0_AA[0] ) | ( ~ram0_WIDTH_SELA[0] & ~ram0_AA[1] ) ) );
			ram1_WENBA_SEL[1]	<= ( ( ( ram0_WIDTH_SELA == 2'b00 ) & ram0_WENBA[0] ) | ( ( ram0_WIDTH_SELA[1] == 1'b1 ) & ram1_WENBA[1] ) | ( ( ram0_WIDTH_SELA == 2'b01 ) & ram0_WENBA[1] ) ) | ( ~ram0_WIDTH_SELA[1] & ( ~ram0_AA[0] | ( ~ram0_WIDTH_SELA[0] & ~ram0_AA[1] ) ) );

			ram0_AB_SEL				<= ram0_AB >> { ~ram0_WIDTH_SELB[1] & ~( ram0_WIDTH_SELB[1] ^ ram0_WIDTH_SELB[0] ), ~ram0_WIDTH_SELB[1] & ram0_WIDTH_SELB[0] };
			ram1_AB_SEL				<= ram0_AB >> { ~ram0_WIDTH_SELB[1] & ~( ram0_WIDTH_SELB[1] ^ ram0_WIDTH_SELB[0] ), ~ram0_WIDTH_SELB[1] & ram0_WIDTH_SELB[0] };
			ram1_AB_byte_SEL	<= ram0_AB[0] & ( ~|ram0_WIDTH_SELB );
			ram0_WENBB_SEL[0]	<= ram0_WIDTH_SELB[1] | ( ram0_WIDTH_SELA[1] | ram1_WENBA[0] | ( ram0_AB[0] | ( ~ram0_WIDTH_SELB[0] & ram0_AB[1] ) ) );
			ram0_WENBB_SEL[1]	<= ram0_WIDTH_SELB[1] | ( ram0_WIDTH_SELA[1] | ( ( ~|ram0_WIDTH_SELB & ram1_WENBA[0] ) | ( |ram0_WIDTH_SELB & ram1_WENBA[1] ) ) | ( ( ram0_WIDTH_SELB[0] & ram0_AB[0] ) | ( ~ram0_WIDTH_SELB[0] & ~ram0_AB[0] ) | ( ~ram0_WIDTH_SELB[0] & ram0_AB[1] ) ) );
			ram1_WENBB_SEL[0]	<= ram0_WIDTH_SELB[1] | ( ram0_WIDTH_SELA[1] | ram1_WENBA[0] | ( ( ram0_WIDTH_SELB[0] & ~ram0_AB[0] ) | ( ~ram0_WIDTH_SELB[0] & ram0_AB[0] ) | ( ~ram0_WIDTH_SELB[0] & ~ram0_AB[1] ) ) );
			ram1_WENBB_SEL[1]	<= ram0_WIDTH_SELB[1] | ( ram0_WIDTH_SELA[1] | ( ( ~|ram0_WIDTH_SELB & ram1_WENBA[0] ) | ( |ram0_WIDTH_SELB & ram1_WENBA[1] ) ) | ( ~ram0_AB[0] | ( ~ram0_WIDTH_SELB[0] & ~ram0_AB[1] ) ) );
		end
	end
  


	ram	#(.ADDRWID(ADDRWID-2)) ram0_inst(
									.AA( ram0_AA_SEL ),
									.AB( ram0_AB_SEL ),
									.CLKA( ram0_CEA ),
									.CLKB( ram0_CEB ),
									.WENA( ram0_WEBA_SEL ),
									.WENB( ram0_WEBB_SEL ),
									.CENA( ram0_CSBA_SEL ),
									.CENB( ram0_CSBB_SEL ),
									.WENBA( ram0_WENBA_SEL ),
									.WENBB( ram0_WENBB_SEL ),
									.DA( ram0_I_SEL1 ),
									.QA( QA_0 ),
									.DB( ram1_I_SEL1 ),
									.QB( QB_0 )
								);

	ram	#(.ADDRWID(ADDRWID-2)) ram1_inst(
									.AA( ram1_AA_SEL ),
									.AB( ram1_AB_SEL ),
									.CLKA( ram1_CEA ),
									.CLKB( ram1_CEB ),
									.WENA( ram1_WEBA_SEL ),
									.WENB( ram1_WEBB_SEL ),
									.CENA( ram1_CSBA_SEL ),
									.CENB( ram1_CSBB_SEL ),
									.WENBA( ram1_WENBA_SEL ),
									.WENBB( ram1_WENBB_SEL ),
									.DA( ram1_I_SEL2 ),
									.QA( QA_1 ),
									.DB( ram1_I_SEL1 ),
									.QB( QB_1 )
								);


endmodule

`timescale 1 ns /10 ps
`define ADDRWID 11
`define DATAWID 18
`define WEWID 2

module ram_block_8K (  
                                CLK1_0,
                                CLK2_0,
                                WD_0,
                                RD_0,
                                A1_0,
                                A2_0,
                                CS1_0,
                                CS2_0,
                                WEN1_0,
                                POP_0,
                                Almost_Full_0,
                                Almost_Empty_0,
                                PUSH_FLAG_0,
                                POP_FLAG_0,
                                
                                FIFO_EN_0,
                                SYNC_FIFO_0,
                                PIPELINE_RD_0,
                                WIDTH_SELECT1_0,
                                WIDTH_SELECT2_0,
                                
                                CLK1_1,
                                CLK2_1,
                                WD_1,
                                RD_1,
                                A1_1,
                                A2_1,
                                CS1_1,
                                CS2_1,
                                WEN1_1,
                                POP_1,
                                Almost_Empty_1,
                                Almost_Full_1,
                                PUSH_FLAG_1,
                                POP_FLAG_1,
                                
                                FIFO_EN_1,
                                SYNC_FIFO_1,
                                PIPELINE_RD_1,
                                WIDTH_SELECT1_1,
                                WIDTH_SELECT2_1,
                                
                                CONCAT_EN_0,
                                CONCAT_EN_1,
				
								PUSH_0,
								PUSH_1,
								aFlushN_0,
								aFlushN_1
				
                              );

  input                   CLK1_0;
  input                   CLK2_0;
  input   [`DATAWID-1:0]  WD_0;
  output  [`DATAWID-1:0]  RD_0;
  input   [`ADDRWID-1:0]    A1_0; 
  input   [`ADDRWID-1:0]    A2_0; 
  input                   CS1_0;
  input                   CS2_0;
  input   [`WEWID-1:0]    WEN1_0;
  input                   POP_0;
  output                  Almost_Full_0;
  output                  Almost_Empty_0;
  output  [3:0]           PUSH_FLAG_0;
  output  [3:0]           POP_FLAG_0;
  input                   FIFO_EN_0;
  input                   SYNC_FIFO_0;
  input                   PIPELINE_RD_0;
  input   [1:0]           WIDTH_SELECT1_0;
  input   [1:0]           WIDTH_SELECT2_0;
  
  input                   CLK1_1;
  input                   CLK2_1;
  input   [`DATAWID-1:0]  WD_1;
  output  [`DATAWID-1:0]  RD_1;
  input   [`ADDRWID-1:0]    A1_1; 
  input   [`ADDRWID-1:0]    A2_1; 
  input                   CS1_1;
  input                   CS2_1;
  input   [`WEWID-1:0]    WEN1_1;
  input                   POP_1;
  output                  Almost_Full_1;
  output                  Almost_Empty_1;
  output  [3:0]           PUSH_FLAG_1;
  output  [3:0]           POP_FLAG_1;
  input                   FIFO_EN_1;
  input                   SYNC_FIFO_1;
  input                   PIPELINE_RD_1;
  input   [1:0]           WIDTH_SELECT1_1;
  input   [1:0]           WIDTH_SELECT2_1;
  
  input                   CONCAT_EN_0;
  input                   CONCAT_EN_1;
 				
  input                   PUSH_0;
  input                   PUSH_1;
  input                   aFlushN_0;
  input                   aFlushN_1;
  
  reg                   rstn;
    
  wire  [`WEWID-1:0]    RAM0_WENb1_SEL;
  wire  [`WEWID-1:0]    RAM1_WENb1_SEL;
  
  wire                  RAM0_CS1_SEL;
  wire                  RAM0_CS2_SEL;
  wire                  RAM1_CS1_SEL;
  wire                  RAM1_CS2_SEL;

  wire  [`ADDRWID-1:0]  Fifo0_Write_Addr;
  wire  [`ADDRWID-1:0]  Fifo0_Read_Addr;
                        
  wire  [`ADDRWID-1:0]  Fifo1_Write_Addr;
  wire  [`ADDRWID-1:0]  Fifo1_Read_Addr;

  wire  [`ADDRWID-1:0]  RAM0_AA_SEL;
  wire  [`ADDRWID-1:0]  RAM0_AB_SEL;
  wire  [`ADDRWID-1:0]  RAM1_AA_SEL;
  wire  [`ADDRWID-1:0]  RAM1_AB_SEL;
  
  wire                  Concat_En_SEL;
  
  initial
  begin
    rstn  = 1'b0;
    #30  rstn  = 1'b1;
  end
  
  assign fifo0_rstn = rstn & aFlushN_0;
  assign fifo1_rstn = rstn & aFlushN_1;

  assign Concat_En_SEL  = ( CONCAT_EN_0 | WIDTH_SELECT1_0[1] | WIDTH_SELECT2_0[1] )? 1'b1 : 1'b0;
  
  assign RAM0_AA_SEL  = FIFO_EN_0 ? Fifo0_Write_Addr : A1_0[`ADDRWID-1:0];
  assign RAM0_AB_SEL  = FIFO_EN_0 ? Fifo0_Read_Addr  : A2_0[`ADDRWID-1:0];
  assign RAM1_AA_SEL  = FIFO_EN_1 ? Fifo1_Write_Addr : A1_1[`ADDRWID-1:0];
  assign RAM1_AB_SEL  = FIFO_EN_1 ? Fifo1_Read_Addr  : A2_1[`ADDRWID-1:0];
  
  assign RAM0_WENb1_SEL = FIFO_EN_0 ? { `WEWID{ ~PUSH_0 } } : ~WEN1_0;
  assign RAM1_WENb1_SEL = ( FIFO_EN_1 & ~Concat_En_SEL ) ? { `WEWID{ ~PUSH_1 } } :
                          ( ( FIFO_EN_0 &  Concat_En_SEL ) ? ( WIDTH_SELECT1_0[1] ? { `WEWID{ ~PUSH_0 } } : { `WEWID{ 1'b1 } } ) : ~WEN1_1 );

  assign RAM0_CS1_SEL = ( FIFO_EN_0 ? CS1_0 : ~CS1_0 );
  assign RAM0_CS2_SEL = ( FIFO_EN_0 ? CS2_0 : ~CS2_0 );
  assign RAM1_CS1_SEL = ( FIFO_EN_1 ? CS1_1 : ~CS1_1 );
  assign RAM1_CS2_SEL = ( FIFO_EN_1 ? CS2_1 : ~CS2_1 );

  x2_model #(.ADDRWID(`ADDRWID)) x2_8K_model_inst(
                            .Concat_En( Concat_En_SEL ),
                            
                            .ram0_WIDTH_SELA( WIDTH_SELECT1_0 ),
                            .ram0_WIDTH_SELB( WIDTH_SELECT2_0 ),
                            .ram0_PLRD( PIPELINE_RD_0 ),
                            
                            .ram0_CEA( CLK1_0 ),
                            .ram0_CEB( CLK2_0 ),
                            .ram0_I( WD_0 ),
                            .ram0_O( RD_0 ),
                            .ram0_AA( RAM0_AA_SEL ),
                            .ram0_AB( RAM0_AB_SEL ),
                            .ram0_CSBA( RAM0_CS1_SEL ),
                            .ram0_CSBB( RAM0_CS2_SEL ),
                            .ram0_WENBA( RAM0_WENb1_SEL ),
                            
                            .ram1_WIDTH_SELA( WIDTH_SELECT1_1 ),
                            .ram1_WIDTH_SELB( WIDTH_SELECT2_1 ),
                            .ram1_PLRD( PIPELINE_RD_1 ),
                            
                            .ram1_CEA( CLK1_1 ),
                            .ram1_CEB( CLK2_1 ),
                            .ram1_I( WD_1 ),
                            .ram1_O( RD_1 ),
                            .ram1_AA( RAM1_AA_SEL ),
                            .ram1_AB( RAM1_AB_SEL ),
                            .ram1_CSBA( RAM1_CS1_SEL ),
                            .ram1_CSBB( RAM1_CS2_SEL ),
                            .ram1_WENBA( RAM1_WENb1_SEL )
                          );

  fifo_controller_model #(.MAX_PTR_WIDTH(`ADDRWID+1)) fifo_controller0_inst(
                                                .Push_Clk( CLK1_0 ),
                                                .Pop_Clk( CLK2_0 ),
                                                
                                                .Fifo_Push( PUSH_0 ),
                                                .Fifo_Push_Flush( CS1_0 ),
                                                .Fifo_Full( Almost_Full_0 ),
                                                .Fifo_Full_Usr( PUSH_FLAG_0 ),
                                                
                                                .Fifo_Pop( POP_0 ),
                                                .Fifo_Pop_Flush( CS2_0 ),
                                                .Fifo_Empty( Almost_Empty_0 ),
                                                .Fifo_Empty_Usr( POP_FLAG_0 ),
                                                
                                                .Write_Addr( Fifo0_Write_Addr ),
                                                
                                                .Read_Addr( Fifo0_Read_Addr ),
                                                                        
                                                .Fifo_Ram_Mode( Concat_En_SEL ),
                                                .Fifo_Sync_Mode( SYNC_FIFO_0 ),
                                                .Fifo_Push_Width( WIDTH_SELECT1_0 ),
                                                .Fifo_Pop_Width( WIDTH_SELECT2_0 ),
                                                .Rst_n( fifo0_rstn )
                                              );

  fifo_controller_model #(.MAX_PTR_WIDTH(`ADDRWID+1)) fifo_controller1_inst(
                                                .Push_Clk( CLK1_1 ),
                                                .Pop_Clk( CLK2_1 ),
                                                
                                                .Fifo_Push( PUSH_1 ),
                                                .Fifo_Push_Flush( CS1_1 ),
                                                .Fifo_Full( Almost_Full_1 ),
                                                .Fifo_Full_Usr( PUSH_FLAG_1 ),
                                                
                                                .Fifo_Pop( POP_1 ),
                                                .Fifo_Pop_Flush( CS2_1 ),
                                                .Fifo_Empty( Almost_Empty_1 ),
                                                .Fifo_Empty_Usr( POP_FLAG_1 ),
                                                
                                                .Write_Addr( Fifo1_Write_Addr ),
                                                
                                                .Read_Addr( Fifo1_Read_Addr ),
                                                                        
                                                .Fifo_Ram_Mode( 1'b0 ),
                                                .Fifo_Sync_Mode( SYNC_FIFO_1 ),
                                                .Fifo_Push_Width( { 1'b0, WIDTH_SELECT1_1[0] } ),
                                                .Fifo_Pop_Width( { 1'b0, WIDTH_SELECT2_1[0] } ),
                                                .Rst_n( fifo1_rstn )
                                              );

endmodule

module sw_mux (
	port_out,
	default_port,
	alt_port,
	switch
	);
	
	output port_out;
	input default_port;
	input alt_port;
	input switch;
	
	assign port_out = switch ? alt_port : default_port;
	
endmodule


`define ADDRWID_8k2 11
`define DATAWID 18
`define WEWID 2

module ram8k_2x1_cell_macro (  
        CLK1_0,
        CLK2_0,
		CLK1S_0,
        CLK2S_0,
        WD_0,
        RD_0,
        A1_0,
        A2_0,
        CS1_0,
        CS2_0,
        WEN1_0,
        CLK1EN_0,
        CLK2EN_0,
        P1_0,
        P2_0,
        Almost_Full_0,
        Almost_Empty_0,
        PUSH_FLAG_0,
        POP_FLAG_0,

        FIFO_EN_0,
        SYNC_FIFO_0,
        PIPELINE_RD_0,
        WIDTH_SELECT1_0,
        WIDTH_SELECT2_0,
        DIR_0,
        ASYNC_FLUSH_0,
		ASYNC_FLUSH_S0,

        CLK1_1,
        CLK2_1,
		CLK1S_1,
        CLK2S_1,
        WD_1,
        RD_1,
        A1_1,
        A2_1,
        CS1_1,
        CS2_1,
        WEN1_1,
        CLK1EN_1,
        CLK2EN_1,
        P1_1,
        P2_1,
        Almost_Empty_1,
        Almost_Full_1,
        PUSH_FLAG_1,
        POP_FLAG_1,

        FIFO_EN_1,
        SYNC_FIFO_1,
        PIPELINE_RD_1,
        WIDTH_SELECT1_1,
        WIDTH_SELECT2_1,
        DIR_1,
        ASYNC_FLUSH_1,
		ASYNC_FLUSH_S1,

        CONCAT_EN_0,
        CONCAT_EN_1

);

  input                   CLK1_0;
  input                   CLK2_0;
  input                   CLK1S_0;
  input                   CLK2S_0;
  input   [`DATAWID-1:0]  WD_0;
  output  [`DATAWID-1:0]  RD_0;
  input   [`ADDRWID_8k2-1:0]  A1_0;
  input   [`ADDRWID_8k2-1:0]  A2_0;
  input                   CS1_0;
  input                   CS2_0;
  input   [`WEWID-1:0]    WEN1_0;
  input  		  CLK1EN_0;
  input                   CLK2EN_0;
  input                   P1_0;
  input                   P2_0;
  output                  Almost_Full_0;
  output                  Almost_Empty_0;
  output  [3:0]           PUSH_FLAG_0;
  output  [3:0]           POP_FLAG_0;
  input                   FIFO_EN_0;
  input                   SYNC_FIFO_0;
  input                   DIR_0;
  input                   ASYNC_FLUSH_0;
  input                   ASYNC_FLUSH_S0;
  input                   PIPELINE_RD_0;
  input   [1:0]           WIDTH_SELECT1_0;
  input   [1:0]           WIDTH_SELECT2_0;
  
  input                   CLK1_1;
  input                   CLK2_1;
  input                   CLK1S_1;
  input                   CLK2S_1;
  input   [`DATAWID-1:0]  WD_1;
  output  [`DATAWID-1:0]  RD_1;
  input   [`ADDRWID_8k2-1:0]  A1_1;
  input   [`ADDRWID_8k2-1:0]  A2_1;
  input                   CS1_1;
  input                   CS2_1;
  input   [`WEWID-1:0]    WEN1_1;
  input  		  CLK1EN_1;
  input  		  CLK2EN_1;
  input  		  P1_1;
  input  		  P2_1;
  output                  Almost_Full_1;
  output                  Almost_Empty_1;
  output  [3:0]           PUSH_FLAG_1;
  output  [3:0]           POP_FLAG_1;
  input                   FIFO_EN_1;
  input                   SYNC_FIFO_1;
  input  		  DIR_1;
  input  		  ASYNC_FLUSH_1;
  input  		  ASYNC_FLUSH_S1;
  input                   PIPELINE_RD_1;
  input   [1:0]           WIDTH_SELECT1_1;
  input   [1:0]           WIDTH_SELECT2_1;
  
  input                   CONCAT_EN_0;
  input                   CONCAT_EN_1;

reg	RAM0_domain_sw;
reg	RAM1_domain_sw;

wire CLK1P_0, CLK1P_1, CLK2P_0, CLK2P_1, ASYNC_FLUSHP_1, ASYNC_FLUSHP_0;

assign WidSel1_1 = WIDTH_SELECT1_0[1];
assign WidSel2_1 = WIDTH_SELECT2_0[1];

assign CLK1P_0 = CLK1S_0 ? ~CLK1_0 : CLK1_0;
assign CLK1P_1 = CLK1S_1 ? ~CLK1_1 : CLK1_1;
assign CLK2P_0 = CLK2S_0 ? ~CLK2_0 : CLK2_0;
assign CLK2P_1 = CLK2S_1 ? ~CLK2_1 : CLK2_1;
assign ASYNC_FLUSHP_0 = ASYNC_FLUSH_S0? ~ASYNC_FLUSH_0 : ASYNC_FLUSH_0;
assign ASYNC_FLUSHP_1 = ASYNC_FLUSH_S1? ~ASYNC_FLUSH_1 : ASYNC_FLUSH_1;


always @( CONCAT_EN_0 or FIFO_EN_0 or FIFO_EN_1 or WidSel1_1 or WidSel2_1 or DIR_0 or DIR_1)

begin
	if (CONCAT_EN_0)                                               
		begin
		if (~FIFO_EN_0)                                            
			begin
			RAM0_domain_sw = 1'b0;                                 
			RAM1_domain_sw = 1'b0;
			end
		else                                                       
			begin
			RAM0_domain_sw = DIR_0;                                
			RAM1_domain_sw = DIR_0;
			end
		end
	else                                                           
		begin
			if (WidSel1_1 || WidSel2_1)        
				begin
				if (~FIFO_EN_0)                
					begin
					RAM0_domain_sw = 1'b0;     
					RAM1_domain_sw = 1'b0;
					end
				else                           
					begin
   		 		RAM0_domain_sw = DIR_0;        
			  	RAM1_domain_sw = DIR_0;
					end
				end
			else                               
				begin
				if (~FIFO_EN_0)                
					RAM0_domain_sw = 1'b0;
				else                           
					RAM0_domain_sw = DIR_0;
				if (~FIFO_EN_1)                
					RAM1_domain_sw = 1'b0;
				else                           
			  	RAM1_domain_sw = DIR_1;
				end
		end
end

assign RAM0_Clk1_gated = CLK1EN_0 & CLK1P_0;
assign RAM0_Clk2_gated = CLK2EN_0 & CLK2P_0;
assign RAM1_Clk1_gated = CLK1EN_1 & CLK1P_1;
assign RAM1_Clk2_gated = CLK2EN_1 & CLK2P_1;

sw_mux RAM0_clk_sw_port1 (.port_out(RAM0_clk_port1), .default_port(RAM0_Clk1_gated), .alt_port(RAM0_Clk2_gated), .switch(RAM0_domain_sw));
sw_mux RAM0_P_sw_port1 (.port_out(RAM0_push_port1), .default_port(P1_0), .alt_port(P2_0), .switch(RAM0_domain_sw));
sw_mux RAM0_Flush_sw_port1 (.port_out(RAM0CS_Sync_Flush_port1), .default_port(CS1_0), .alt_port(CS2_0), .switch(RAM0_domain_sw));
sw_mux RAM0_WidSel0_port1 (.port_out(RAM0_Wid_Sel0_port1), .default_port(WIDTH_SELECT1_0[0]), .alt_port(WIDTH_SELECT2_0[0]), .switch(RAM0_domain_sw));
sw_mux RAM0_WidSel1_port1 (.port_out(RAM0_Wid_Sel1_port1), .default_port(WIDTH_SELECT1_0[1]), .alt_port(WIDTH_SELECT2_0[1]), .switch(RAM0_domain_sw));

sw_mux RAM0_clk_sw_port2 (.port_out(RAM0_clk_port2), .default_port(RAM0_Clk2_gated), .alt_port(RAM0_Clk1_gated), .switch(RAM0_domain_sw));
sw_mux RAM0_P_sw_port2 (.port_out(RAM0_pop_port2), .default_port(P2_0), .alt_port(P1_0), .switch(RAM0_domain_sw));
sw_mux RAM0_Flush_sw_port2 (.port_out(RAM0CS_Sync_Flush_port2), .default_port(CS2_0), .alt_port(CS1_0), .switch(RAM0_domain_sw));
sw_mux RAM0_WidSel0_port2 (.port_out(RAM0_Wid_Sel0_port2), .default_port(WIDTH_SELECT2_0[0]), .alt_port(WIDTH_SELECT1_0[0]), .switch(RAM0_domain_sw));
sw_mux RAM0_WidSel1_port2 (.port_out(RAM0_Wid_Sel1_port2), .default_port(WIDTH_SELECT2_0[1]), .alt_port(WIDTH_SELECT1_0[1]), .switch(RAM0_domain_sw));

sw_mux RAM1_clk_sw_port1 (.port_out(RAM1_clk_port1), .default_port(RAM1_Clk1_gated), .alt_port(RAM1_Clk2_gated), .switch(RAM1_domain_sw));
sw_mux RAM1_P_sw_port1 (.port_out(RAM1_push_port1), .default_port(P1_1), .alt_port(P2_1), .switch(RAM1_domain_sw));
sw_mux RAM1_Flush_sw_port1 (.port_out(RAM1CS_Sync_Flush_port1), .default_port(CS1_1), .alt_port(CS2_1), .switch(RAM1_domain_sw));
sw_mux RAM1_WidSel0_port1 (.port_out(RAM1_Wid_Sel0_port1), .default_port(WIDTH_SELECT1_1[0]), .alt_port(WIDTH_SELECT2_1[0]), .switch(RAM1_domain_sw));
sw_mux RAM1_WidSel1_port1 (.port_out(RAM1_Wid_Sel1_port1), .default_port(WIDTH_SELECT1_1[1]), .alt_port(WIDTH_SELECT2_1[1]), .switch(RAM1_domain_sw));

sw_mux RAM1_clk_sw_port2 (.port_out(RAM1_clk_port2), .default_port(RAM1_Clk2_gated), .alt_port(RAM1_Clk1_gated), .switch(RAM1_domain_sw));
sw_mux RAM1_P_sw_port2 (.port_out(RAM1_pop_port2), .default_port(P2_1), .alt_port(P1_1), .switch(RAM1_domain_sw));
sw_mux RAM1_Flush_sw_port2 (.port_out(RAM1CS_Sync_Flush_port2), .default_port(CS2_1), .alt_port(CS1_1), .switch(RAM1_domain_sw));
sw_mux RAM1_WidSel0_port2 (.port_out(RAM1_Wid_Sel0_port2), .default_port(WIDTH_SELECT2_1[0]), .alt_port(WIDTH_SELECT1_1[0]), .switch(RAM1_domain_sw));
sw_mux RAM1_WidSel1_port2 (.port_out(RAM1_Wid_Sel1_port2), .default_port(WIDTH_SELECT2_1[1]), .alt_port(WIDTH_SELECT1_1[1]), .switch(RAM1_domain_sw));

ram_block_8K ram_block_8K_inst (  
                                .CLK1_0(RAM0_clk_port1),
                                .CLK2_0(RAM0_clk_port2),
                                .WD_0(WD_0),
                                .RD_0(RD_0),
                                .A1_0(A1_0),
                                .A2_0(A2_0),
                                .CS1_0(RAM0CS_Sync_Flush_port1),
                                .CS2_0(RAM0CS_Sync_Flush_port2),
                                .WEN1_0(WEN1_0),
                                .POP_0(RAM0_pop_port2),
                                .Almost_Full_0(Almost_Full_0),
                                .Almost_Empty_0(Almost_Empty_0),
                                .PUSH_FLAG_0(PUSH_FLAG_0),
                                .POP_FLAG_0(POP_FLAG_0),
                                
                                .FIFO_EN_0(FIFO_EN_0),
                                .SYNC_FIFO_0(SYNC_FIFO_0),
                                .PIPELINE_RD_0(PIPELINE_RD_0),
                                .WIDTH_SELECT1_0({RAM0_Wid_Sel1_port1,RAM0_Wid_Sel0_port1}),
                                .WIDTH_SELECT2_0({RAM0_Wid_Sel1_port2,RAM0_Wid_Sel0_port2}),
                                
                                .CLK1_1(RAM1_clk_port1),
                                .CLK2_1(RAM1_clk_port2),
                                .WD_1(WD_1),
                                .RD_1(RD_1),
                                .A1_1(A1_1),
                                .A2_1(A2_1),
                                .CS1_1(RAM1CS_Sync_Flush_port1),
                                .CS2_1(RAM1CS_Sync_Flush_port2),
                                .WEN1_1(WEN1_1),
                                .POP_1(RAM1_pop_port2),
                                .Almost_Empty_1(Almost_Empty_1),
                                .Almost_Full_1(Almost_Full_1),
                                .PUSH_FLAG_1(PUSH_FLAG_1),
                                .POP_FLAG_1(POP_FLAG_1),
                                
                                .FIFO_EN_1(FIFO_EN_1),
                                .SYNC_FIFO_1(SYNC_FIFO_1),
                                .PIPELINE_RD_1(PIPELINE_RD_1),
                                .WIDTH_SELECT1_1({RAM1_Wid_Sel1_port1,RAM1_Wid_Sel0_port1}),
                                .WIDTH_SELECT2_1({RAM1_Wid_Sel1_port2,RAM1_Wid_Sel0_port2}),
                                
                                .CONCAT_EN_0(CONCAT_EN_0),
                                .CONCAT_EN_1(CONCAT_EN_1),
				
								.PUSH_0(RAM0_push_port1),
								.PUSH_1(RAM1_push_port1),
								.aFlushN_0(~ASYNC_FLUSHP_0),
								.aFlushN_1(~ASYNC_FLUSHP_1)
				
                              );

endmodule

module ram8k_2x1_cell  (
    input A1_0_b0,
	input A1_0_b1,
	input A1_0_b2,
	input A1_0_b3,
	input A1_0_b4,
	input A1_0_b5,
	input A1_0_b6,
	input A1_0_b7,
	input A1_0_b8,
	input A1_0_b9,
	input A1_0_b10,
    input A2_0_b0,
	input A2_0_b1,
	input A2_0_b2,
	input A2_0_b3,
	input A2_0_b4,
	input A2_0_b5,
	input A2_0_b6,
	input A2_0_b7,
	input A2_0_b8,
	input A2_0_b9,
	input A2_0_b10,
    input A1_1_b0,
	input A1_1_b1,
	input A1_1_b2,
	input A1_1_b3,
	input A1_1_b4,
	input A1_1_b5,
	input A1_1_b6,
	input A1_1_b7,
	input A1_1_b8,
	input A1_1_b9,
	input A1_1_b10,
    input A2_1_b0,
	input A2_1_b1,
	input A2_1_b2,
	input A2_1_b3,
	input A2_1_b4,
	input A2_1_b5,
	input A2_1_b6,
	input A2_1_b7,
	input A2_1_b8,
	input A2_1_b9,
	input A2_1_b10, 
	input WD_0_b0, 
	input WD_0_b1, 
	input WD_0_b2, 
	input WD_0_b3, 
	input WD_0_b4, 
	input WD_0_b5, 
	input WD_0_b6, 
	input WD_0_b7, 
	input WD_0_b8, 
	input WD_0_b9,
	input WD_0_b10, 
	input WD_0_b11,
	input WD_0_b12,
	input WD_0_b13,
	input WD_0_b14,
	input WD_0_b15,
	input WD_0_b16,
	input WD_0_b17,
	input WD_1_b0, 
	input WD_1_b1, 
	input WD_1_b2, 
	input WD_1_b3, 
	input WD_1_b4, 
	input WD_1_b5, 
	input WD_1_b6, 
	input WD_1_b7, 
	input WD_1_b8, 
	input WD_1_b9,
	input WD_1_b10, 
	input WD_1_b11,
	input WD_1_b12,
	input WD_1_b13,
	input WD_1_b14,
	input WD_1_b15,
	input WD_1_b16,
	input WD_1_b17,
    input CLK1_0,
    input CLK1_1,
    input CLK2_0,
    input CLK2_1,
    output Almost_Empty_0, Almost_Empty_1, Almost_Full_0, Almost_Full_1,
    input ASYNC_FLUSH_0, ASYNC_FLUSH_1, CLK1EN_0, CLK1EN_1, CLK2EN_0, CLK2EN_1, CONCAT_EN_0, CONCAT_EN_1, CS1_0, CS1_1,CS2_0, CS2_1, DIR_0, DIR_1, FIFO_EN_0, FIFO_EN_1, P1_0, P1_1, P2_0,P2_1, PIPELINE_RD_0, PIPELINE_RD_1,
    output POP_FLAG_0_b0,
	output POP_FLAG_0_b1,
	output POP_FLAG_0_b2,
	output POP_FLAG_0_b3,
    output POP_FLAG_1_b0,
	output POP_FLAG_1_b1,
	output POP_FLAG_1_b2,
	output POP_FLAG_1_b3,
    output PUSH_FLAG_0_b0,
	output PUSH_FLAG_0_b1,
	output PUSH_FLAG_0_b2,
	output PUSH_FLAG_0_b3,
    output PUSH_FLAG_1_b0,
	output PUSH_FLAG_1_b1,
	output PUSH_FLAG_1_b2,
	output PUSH_FLAG_1_b3,
    output RD_0_b0,
	output RD_0_b1,
	output RD_0_b2,
	output RD_0_b3,
	output RD_0_b4,
	output RD_0_b5,
	output RD_0_b6,
	output RD_0_b7,
	output RD_0_b8,
	output RD_0_b9,
	output RD_0_b10,
	output RD_0_b11,
	output RD_0_b12,
	output RD_0_b13,
	output RD_0_b14,
	output RD_0_b15,
	output RD_0_b16,
	output RD_0_b17,
    output RD_1_b0,
	output RD_1_b1,
	output RD_1_b2,
	output RD_1_b3,
	output RD_1_b4,
	output RD_1_b5,
	output RD_1_b6,
	output RD_1_b7,
	output RD_1_b8,
	output RD_1_b9,
	output RD_1_b10,
	output RD_1_b11,
	output RD_1_b12,
	output RD_1_b13,
	output RD_1_b14,
	output RD_1_b15,
	output RD_1_b16,
	output RD_1_b17,
    input  SYNC_FIFO_0, SYNC_FIFO_1,
    input  WEN1_0_b0,   
	input  WEN1_0_b1,
	input  WEN1_1_b0,
	input  WEN1_1_b1,
    input [1:0] WIDTH_SELECT1_0,
    input [1:0] WIDTH_SELECT1_1,
    input [1:0] WIDTH_SELECT2_0,
    input [1:0] WIDTH_SELECT2_1,
    input RMA_b0,
	input RMA_b1,
	input RMA_b2,
	input RMA_b3,
	input RMB_b0,
	input RMB_b1,
	input RMB_b2,
    input RMB_b3
	);
	
	ram8k_2x1_cell_macro I1 ( 	
							.A1_0({A1_0_b10,A1_0_b9,A1_0_b8,A1_0_b7,A1_0_b6,A1_0_b5,A1_0_b4,A1_0_b3,A1_0_b2,A1_0_b1,A1_0_b0}), 
							.A1_1({A1_1_b10,A1_1_b9,A1_1_b8,A1_1_b7,A1_1_b6,A1_1_b5,A1_1_b4,A1_1_b3,A1_1_b2,A1_1_b1,A1_1_b0}),
							.A2_0({A2_0_b10,A2_0_b9,A2_0_b8,A2_0_b7,A2_0_b6,A2_0_b5,A2_0_b4,A2_0_b3,A2_0_b2,A2_0_b1,A2_0_b0}), 
							.A2_1({A2_1_b10,A2_1_b9,A2_1_b8,A2_1_b7,A2_1_b6,A2_1_b5,A2_1_b4,A2_1_b3,A2_1_b2,A2_1_b1,A2_1_b0}),
							.Almost_Empty_0(Almost_Empty_0),
							.Almost_Empty_1(Almost_Empty_1),
							.Almost_Full_0(Almost_Full_0),
							.Almost_Full_1(Almost_Full_1),
							.ASYNC_FLUSH_0(ASYNC_FLUSH_0),
							.ASYNC_FLUSH_1(ASYNC_FLUSH_1),
							.ASYNC_FLUSH_S0(1'b0),
							.ASYNC_FLUSH_S1(1'b0), 
							.CLK1_0(CLK1_0),
							.CLK1_1(CLK1_1) , 
							.CLK1EN_0(CLK1EN_0) , 
							.CLK1EN_1(CLK1EN_1),
							.CLK1S_0(1'b0) , 
							.CLK1S_1(1'b0) , 
							.CLK2_0(CLK2_0),
							.CLK2_1(CLK2_1) , 
							.CLK2EN_0(CLK2EN_0) , 
							.CLK2EN_1(CLK2EN_1),
							.CLK2S_0(1'b0) , 
							.CLK2S_1(1'b0),
							.CONCAT_EN_0(CONCAT_EN_0) , 
							.CONCAT_EN_1(CONCAT_EN_1),
							.CS1_0(CS1_0) , 
							.CS1_1(CS1_1) , 
							.CS2_0(CS2_0) , 
							.CS2_1(CS2_1),
							.DIR_0(DIR_0) , 
							.DIR_1(DIR_1) , 
							.FIFO_EN_0(FIFO_EN_0),
							.FIFO_EN_1(FIFO_EN_1) , 
							.P1_0(P1_0) , 
							.P1_1(P1_1) , 
							.P2_0(P2_0),
							.P2_1(P2_1) , 
							.PIPELINE_RD_0(PIPELINE_RD_0),
							.PIPELINE_RD_1(PIPELINE_RD_1),
							.POP_FLAG_0({POP_FLAG_0_b3,POP_FLAG_0_b2,POP_FLAG_0_b1,POP_FLAG_0_b0}),
							.POP_FLAG_1({POP_FLAG_1_b3,POP_FLAG_1_b2,POP_FLAG_1_b1,POP_FLAG_1_b0}),
							.PUSH_FLAG_0({PUSH_FLAG_0_b3,PUSH_FLAG_0_b2,PUSH_FLAG_0_b1,PUSH_FLAG_0_b0}),
							.PUSH_FLAG_1({PUSH_FLAG_1_b3,PUSH_FLAG_1_b2,PUSH_FLAG_1_b1,PUSH_FLAG_1_b0}), 
							.RD_0({RD_0_b17,RD_0_b16,RD_0_b15,RD_0_b14,RD_0_b13,RD_0_b12,RD_0_b11,RD_0_b10,RD_0_b9,RD_0_b8,RD_0_b7,RD_0_b6,RD_0_b5,RD_0_b4,RD_0_b3,RD_0_b2,RD_0_b1,RD_0_b0}),
							.RD_1({RD_1_b17,RD_1_b16,RD_1_b15,RD_1_b14,RD_1_b13,RD_1_b12,RD_1_b11,RD_1_b10,RD_1_b9,RD_1_b8,RD_1_b7,RD_1_b6,RD_1_b5,RD_1_b4,RD_1_b3,RD_1_b2,RD_1_b1,RD_1_b0}) , 
							.SYNC_FIFO_0(SYNC_FIFO_0),
							.SYNC_FIFO_1(SYNC_FIFO_1) , 
							.WD_0({WD_0_b17,WD_0_b16,WD_0_b15,WD_0_b14,WD_0_b13,WD_0_b12,WD_0_b11,WD_0_b10,WD_0_b9,WD_0_b8,WD_0_b7,WD_0_b6,WD_0_b5,WD_0_b4,WD_0_b3,WD_0_b2,WD_0_b1,WD_0_b0}),
							.WD_1({WD_1_b17,WD_1_b16,WD_1_b15,WD_1_b14,WD_1_b13,WD_1_b12,WD_1_b11,WD_1_b10,WD_1_b9,WD_1_b8,WD_1_b7,WD_1_b6,WD_1_b5,WD_1_b4,WD_1_b3,WD_1_b2,WD_1_b1,WD_1_b0}), 
							.WEN1_0({WEN1_0_b1,WEN1_0_b0}),
							.WEN1_1({WEN1_1_b1,WEN1_1_b0}),
							.WIDTH_SELECT1_0({ WIDTH_SELECT1_0[1:0] }),
							.WIDTH_SELECT1_1({ WIDTH_SELECT1_1[1:0] }),
							.WIDTH_SELECT2_0({ WIDTH_SELECT2_0[1:0] }),
							.WIDTH_SELECT2_1({ WIDTH_SELECT2_1[1:0] }) 
							);
					 
endmodule
// ../../../quicklogic/pp3/primitives/ramfifo/ramfifo.sim.v }}}
