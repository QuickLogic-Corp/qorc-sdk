`timescale 1ns / 10ps
module Serializer_Deserializer_Test ( 

			clk_i,          
			rst_i,   
			
			count_up_i,
		
			spi_ss_i,        
			spi_sck_i,        
    		spi_miso_o  


			);
			
input			clk_i;
input			rst_i; 

input			spi_ss_i;
input			spi_sck_i; 
inout	    	spi_miso_o;
input			count_up_i; 

wire			clk_i;
wire			rst_i; 

wire			spi_ss_i;
wire			spi_sck_i;
wire	    	spi_miso_o;
wire			count_up_i; 
wire			spi_sck;

wire			dummy;

reg	    [15:0]	Shift_Reg; 
wire    [15:0]	SPIDR;

reg	    [11:0]	tcounter; 

gclkbuff u_gclkbuff_sck ( .A(spi_sck_i             ) , .Z(spi_sck       ) );

//assign spi_miso_o = (spi_ss_i)? 1'bz : Shift_Reg[15];

bipad u_bipad_I7   ( .A( Shift_Reg[15]   ), .EN( spi_ss_i), .Q( dummy    ), .P( spi_miso_o ) );

assign SPIDR = {4'h0, tcounter};
always @ (negedge spi_sck or posedge spi_ss_i)
begin
	if(spi_ss_i)
		Shift_Reg <= SPIDR;
	else 
		Shift_Reg <= {Shift_Reg[14:0], Shift_Reg[15]};
end	

always @( posedge clk_i or posedge rst_i ) 
begin
    if (rst_i)
	    tcounter   <= 12'h0;
	else
	    if (count_up_i)
		    tcounter   <= tcounter + 1;
		else
			tcounter   <= tcounter;
end	

endmodule