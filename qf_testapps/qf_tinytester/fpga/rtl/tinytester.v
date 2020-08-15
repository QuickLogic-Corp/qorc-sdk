/* ----------------------------------------------------------------------------
tinytester.v

Revision History:
----------------------------------------------------------------------------
2020.08.14 - Tim S. - initial version.
---------------------------------------------------------------------------- */


`timescale 1ns/1ns

module tinytester (
    clk,
    rst,

    control_i,
    dataout_i,
    datain_o,
    oe_i,
    active_on_p0_i,
    active_on_p1_i,
    active_on_p2_i,
    active_on_p3_i,

    padout_o,
    padoe_o,
    padin_i,
    
    sequencer_state
);

input           clk;
input           rst;

input   [31:0]  control_i;
input   [31:0]  dataout_i;
output  [31:0]  datain_o;
input   [31:0]  oe_i;
input   [31:0]  active_on_p0_i;
input   [31:0]  active_on_p1_i;
input   [31:0]  active_on_p2_i;
input   [31:0]  active_on_p3_i;

output  [31:0]  padout_o;
output  [31:0]  padoe_o;
input   [31:0]  padin_i;

output  [06:0]  sequencer_state;

//---------- Local variables ---------------
reg     [31:0]  datain_reg;
reg     [31:0]  padout_reg;
reg     [31:0]  padoe_reg;

reg     [6:0]   sequencer_state;

localparam  [6:0]   SEQ_IDLE   = 6'h20;
localparam  [6:0]   SEQ_PHASE0 = 6'h01;
localparam  [6:0]   SEQ_PHASE1 = 6'h02;
localparam  [6:0]   SEQ_PHASE2 = 6'h04;
localparam  [6:0]   SEQ_PHASE3 = 6'h08;
localparam  [6:0]   SEQ_WAIT   = 6'h10;

assign datain_o = datain_reg;
assign padout_o = padout_reg;
assign padoe_o  = padoe_reg;


// sequencer state machine
always @(posedge rst or posedge clk)
    if (rst) begin
        sequencer_state <= SEQ_IDLE;
        datain_reg <= 32'h00000000;
    end else begin
        case (sequencer_state)
            SEQ_IDLE  :
                        if (control_i[0] == 1'b0) begin             // remain in idle
                            sequencer_state <= SEQ_IDLE;
                        end else begin
                            padout_reg <= dataout_i & active_on_p0_i;
                            padoe_reg  <= oe_i;
                            sequencer_state <= SEQ_PHASE0;          // Go to phase 0
                        end

            SEQ_PHASE0 :
                        begin
                            padout_reg <= dataout_i & active_on_p1_i;
                            sequencer_state <= SEQ_PHASE1;              // Go to phase 1
                        end
                        
            SEQ_PHASE1 :
                        begin
                            padout_reg <= dataout_i & active_on_p2_i;
                            sequencer_state <= SEQ_PHASE2;              // Go to phase 2
                        end

            SEQ_PHASE2 :
                        begin
                            padout_reg <= dataout_i & active_on_p3_i;
                            sequencer_state <= SEQ_PHASE3;              // Go to phase 3
                        end

            SEQ_PHASE3 :
                        begin
                            datain_reg <= padin_i;
                            sequencer_state <= SEQ_WAIT;              // Go to wait
                        end
                        
            SEQ_WAIT :
                        if (control_i[0] == 1'b1) begin             // Remain in wait
                            sequencer_state <= SEQ_WAIT;
                        end else begin
                            sequencer_state <= SEQ_IDLE;          // Go to idle
                        end

            default   : 
                        begin
                            sequencer_state <= SEQ_IDLE;
                        end
        endcase
    end


endmodule

