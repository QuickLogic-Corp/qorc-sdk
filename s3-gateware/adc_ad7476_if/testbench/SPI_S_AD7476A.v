`timescale 1ns / 100ps

`define SINGLE_SPI
module SPI_s_LTC1857(rst_i, sck_i, mosi_i, miso_o, conv_i, RDbar_i, BUSYbar_o );
	input  rst_i, sck_i, conv_i, RDbar_i;	 
	input  mosi_i;
	output miso_o;
	output BUSYbar_o;

	wire	    miso_o;
	reg [7:0]   data_received ;
	reg [15:0]  data_transmit;
	
	reg 	    BUSYbar_o;

	reg [15:0]  mem_adc_sens_data [0:7];

	reg         miso_r;

	integer 	index;
	integer 	ltc_sample_dat_val_file_desc;
	integer 	ltc_cmd_val_file_desc;
	
	integer     addrindex;
	integer     l_count;
	
	reg [2:0]   mem_index;
	
	parameter 		p_CHK_CONV_PULSE = 0;
	parameter 		p_LATCH_CMD 	 = 1;
	
	reg    [2:0] 	s_data_state;	
	
	integer         conv_width_cntr;
	integer         busy_chk_delay_cntr;	

	
	assign miso_o = miso_r;
	
	initial
    begin
	    data_received 				 = 0;
	    miso_r 						 = 0;

	    #10;
		for (index=0;index<8; index=index+1)
		begin
			mem_adc_sens_data[index] = 12'hAA5+index;
		end
    end	

initial
begin
    s_data_state =  p_CHK_CONV_PULSE; 
	conv_width_cntr = 0;
	mem_index = 0;
	busy_chk_delay_cntr = 0;
    #100
    forever 
	begin
	    
		case (s_data_state)

		  p_CHK_CONV_PULSE : begin
								  wait (RDbar_i==0);

                                  conv_width_cntr = 0;								  
		                          s_data_state =  p_LATCH_CMD; 
								  data_transmit = mem_adc_sens_data[mem_index];
								  
								  miso_r 	= data_transmit[15];
								  data_transmit[15:1] = data_transmit[14:0];
		                     end
		
		  
		  p_LATCH_CMD : begin

		                       for (l_count=0;l_count<15;l_count=l_count+1)
							   begin
			
								@ (negedge sck_i);
                                #1;								
								miso_r 				= data_transmit[15];
								data_transmit[15:1] = data_transmit[14:0];  								
		     
		                       end

                               wait (RDbar_i==1);
								mem_index = mem_index + 1;
							    s_data_state =  p_CHK_CONV_PULSE;	    
						 end
						  
				  
           endcase

    end
end

	
endmodule
