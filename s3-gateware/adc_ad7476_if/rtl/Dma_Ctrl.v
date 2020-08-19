`timescale 1ns / 10ps
module Dma_Ctrl ( 
            input  			clk_i	,
            input  			rst_i	,
			
			input           trig_i,
			
            input  			DMA_Active_i	,
			input           ASSP_DMA_Done_i,
            output 			DMA_Done_o	,
			output          DMA_Clr_o,
            input  			DMA_Enable_i	,
			output 			DMA_REQ_o  	 				
	

			);

reg     dma_req;  
reg     dma_active_i_1ff;
reg     dma_active_i_2ff;

wire    dreq_reset;
assign  dreq_reset = rst_i | DMA_Active_i | dma_active_i_1ff | dma_active_i_2ff | ASSP_DMA_Done_i;

assign  DMA_REQ_o = dma_req ;

always @(posedge clk_i or posedge dreq_reset) 
begin
    if (dreq_reset)
    begin
        dma_req           <=  1'b0 ;
    end
    else 
    begin 
		if (trig_i && ~dma_req)
			dma_req           <=  1'b1;
		else 
			dma_req           <=  dma_req;
 	end	
end	

always @(posedge clk_i or posedge rst_i) 
begin
    if (rst_i)
    begin
		dma_active_i_1ff	<=  1'b0;
		dma_active_i_2ff	<=  1'b0;
    end
    else 
    begin  
		dma_active_i_1ff	<= DMA_Active_i;
		dma_active_i_2ff	<= dma_active_i_1ff;
 	end
end  

assign DMA_Clr_o   = ASSP_DMA_Done_i;
assign DMA_Done_o  = ASSP_DMA_Done_i;

endmodule