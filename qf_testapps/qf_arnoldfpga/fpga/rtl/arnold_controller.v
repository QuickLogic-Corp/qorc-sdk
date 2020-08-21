/* ----------------------------------------------------------------------------
arnold_controller.v

Controller for Salinas board

Delivers 12MHz clock
Controls SYS_RST_N

Revision History:
----------------------------------------------------------------------------
2020.08.05 - TimS - initial version.
---------------------------------------------------------------------------- */


`timescale 1ns/1ns

module arnold_controller (
    clk,
    rst,

    control_i,
    clkdiv_i,

    arnold_clk_o,
    arnold_rst_o
);

input           clk;
input           rst;

input   [31:0]  control_i;
input   [31:0]  clkdiv_i;

output          arnold_clk_o;
output          arnold_rst_o;

reg     [31:0]  countreg;
reg             clkout;


assign arnold_clk_o = clkout;
assign arnold_rst_o = control_i[0];

always @( posedge clk or posedge rst) begin
  if (rst) begin
    countreg <= 32'h0;
    clkout <= 0;
  end else begin
    if (countreg >= clkdiv_i) begin
      countreg <= 0;
      clkout <= ~clkout;
    end else begin
      countreg <= countreg + 1;
    end
   end  
end


endmodule

