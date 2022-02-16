/* ----------------------------------------------------------------------------
LED_controller.v
LED controller for a 3-color LED.
Capable of driving a single color to the LED.
Capable of a 2, 3 or 4 stage blink pattern.
Capable of a PWM to create a blend of colors, and/or a dimmer version of
any color.
Contains a 4-stage sequencer, with a programmable color and time duration
for each stage.

Revision History:
----------------------------------------------------------------------------
2020.05.18 - Randy O. - initial version.
---------------------------------------------------------------------------- */


`timescale 1ns/1ns

module LED_controller (
    clk,
    rst,

    duration0,
    duration1,
    duration2,
    duration3,

    color0,
    color1,
    color2,
    color3,

    led_r,
    led_g,
    led_b
);

input           clk;
input           rst;

input   [11:0]  duration0;
input   [11:0]  duration1;
input   [11:0]  duration2;
input   [11:0]  duration3;

input   [2:0]   color0;
input   [2:0]   color1;
input   [2:0]   color2;
input   [2:0]   color3;

output          led_r;
output          led_g;
output          led_b;

parameter   [13:0]   TERMINAL_CNT_1MS = (12000-1);  // assume 12MHz (decimal) clock input

reg     [13:0]  timer_1ms_cnt;
reg             time_1ms;

reg             duration0is0;
reg             duration1is0;
reg             duration2is0;
reg             duration3is0;

reg     [11:0]  sequencer_cnt;
reg     [4:0]   sequencer_state;

localparam  [4:0]   SEQ_IDLE  = 5'h10;
localparam  [4:0]   SEQ_SLOT0 = 5'h01;
localparam  [4:0]   SEQ_SLOT1 = 5'h02;
localparam  [4:0]   SEQ_SLOT2 = 5'h04;
localparam  [4:0]   SEQ_SLOT3 = 5'h08;

reg     [2:0]   LED_color;


// decode duration = 0 to simplify other logic
always @(posedge rst or posedge clk)
    if (rst) begin
        duration0is0 <= 1;
        duration1is0 <= 1;
        duration2is0 <= 1;
        duration3is0 <= 1;
    end else begin
        duration0is0 <= (duration0 == 0);
        duration1is0 <= (duration1 == 0);
        duration2is0 <= (duration2 == 0);
        duration3is0 <= (duration3 == 0);
    end

// 1ms timer
always @(posedge rst or posedge clk)
    if (rst) begin
        timer_1ms_cnt <= 0;
        time_1ms <= 0;
    end else begin
        if ((sequencer_state == SEQ_IDLE) && (duration0is0 || duration1is0)) begin    // if duration0 or duration1 are zero, disable the timer
            timer_1ms_cnt <= 0;
            time_1ms <= 0;
        end else begin
            if (timer_1ms_cnt == TERMINAL_CNT_1MS) begin
                timer_1ms_cnt <= 0;
                time_1ms <= 1;
            end else begin
                timer_1ms_cnt <= timer_1ms_cnt + 1;
                time_1ms <= 0;
            end
        end
    end


// sequencer state machine
always @(posedge rst or posedge clk)
    if (rst) begin
        sequencer_cnt <= 0;
        sequencer_state <= SEQ_IDLE;
    end else begin
        case (sequencer_state)
            SEQ_IDLE  :
                        if (duration0is0 || duration1is0) begin             // remain in idle
                            sequencer_state <= SEQ_IDLE;
                            sequencer_cnt <= 0;
                        end else begin
                            if (time_1ms) begin                             // goto slot0
                                sequencer_state <= SEQ_SLOT0;
                                sequencer_cnt <= duration0;
                            end else begin                                  // wait for 1ms
                                sequencer_state <= SEQ_IDLE;
                                sequencer_cnt <= 0;
                            end
                        end

            SEQ_SLOT0 :
                        if (time_1ms) begin
                            if (sequencer_cnt == 1) begin                   // transition...
                                if (duration1is0) begin                     //      goto idle
                                    sequencer_state <= SEQ_IDLE;
                                    sequencer_cnt <= 0;
                                end else begin                              //      goto slot1
                                    sequencer_state <= SEQ_SLOT1;
                                    sequencer_cnt <= duration1;
                                end
                            end else begin                                  // decrement count
                                sequencer_state <= SEQ_SLOT0;
                                sequencer_cnt <= sequencer_cnt - 1;
                            end
                        end else begin                                      // wait for next 1ms
                            sequencer_state <= SEQ_SLOT0;
                            sequencer_cnt <= sequencer_cnt;
                        end

            SEQ_SLOT1 :
                        if (time_1ms) begin
                            if (sequencer_cnt == 1) begin                   // transition...
                                if (duration2is0) begin
                                    if (duration0is0 || duration1is0) begin //      goto idle
                                        sequencer_state <= SEQ_IDLE;
                                        sequencer_cnt <= 0;
                                    end else begin                          //      goto slot0
                                        sequencer_state <= SEQ_SLOT0;
                                        sequencer_cnt <= duration0;
                                    end
                                end else begin                              //      goto slot2
                                    sequencer_state <= SEQ_SLOT2;
                                    sequencer_cnt <= duration2;
                                end
                            end else begin                                  // decrement count
                                sequencer_state <= SEQ_SLOT1;
                                sequencer_cnt <= sequencer_cnt - 1;
                            end
                        end else begin                                      // wait for next 1ms
                            sequencer_state <= SEQ_SLOT1;
                            sequencer_cnt <= sequencer_cnt;
                        end

            SEQ_SLOT2 :
                        if (time_1ms) begin
                            if (sequencer_cnt == 1) begin                   // transition...
                                if (duration3is0) begin
                                    if (duration0is0 || duration1is0) begin //      goto idle
                                        sequencer_state <= SEQ_IDLE;
                                        sequencer_cnt <= 0;
                                    end else begin                          //      goto SLOT0
                                        sequencer_state <= SEQ_SLOT0;
                                        sequencer_cnt <= duration0;
                                    end
                                end else begin                              //      goto slot3
                                    sequencer_state <= SEQ_SLOT3;
                                    sequencer_cnt <= duration3;
                                end
                            end else begin                                  // decrement count
                                sequencer_state <= SEQ_SLOT2;
                                sequencer_cnt <= sequencer_cnt - 1;
                            end
                        end else begin                                      // wait for next 1ms
                            sequencer_state <= SEQ_SLOT2;
                            sequencer_cnt <= sequencer_cnt;
                        end

            SEQ_SLOT3 :
                        if (time_1ms) begin
                            if (sequencer_cnt == 1) begin                   // transition...
                                if (duration0is0 || duration1is0) begin     //      goto idle
                                    sequencer_state <= SEQ_IDLE;
                                    sequencer_cnt <= 0;
                                end else begin                              //      goto slot0
                                    sequencer_state <= SEQ_SLOT0;
                                    sequencer_cnt <= duration0;
                                end
                            end else begin                                  // decrement count
                                sequencer_state <= SEQ_SLOT3;
                                sequencer_cnt <= sequencer_cnt - 1;
                            end
                        end else begin                                      // wait for next 1ms
                            sequencer_state <= SEQ_SLOT3;
                            sequencer_cnt <= sequencer_cnt;
                        end

            default   : 
                        begin
                            sequencer_state <= SEQ_IDLE;
                            sequencer_cnt <= 0;
                        end
        endcase
    end



always @(posedge rst or posedge clk)
    if (rst)
        LED_color <= 0;
    else
        case (sequencer_state)
            SEQ_IDLE    : LED_color <= color0;
            SEQ_SLOT0   : LED_color <= color0;
            SEQ_SLOT1   : LED_color <= color1;
            SEQ_SLOT2   : LED_color <= color2;
            SEQ_SLOT3   : LED_color <= color3;
            default     : LED_color <= 0;
        endcase



assign led_r = LED_color[2];
assign led_g = LED_color[1];
assign led_b = LED_color[0];


endmodule

