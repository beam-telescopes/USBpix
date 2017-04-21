
//`include  "output_data.sv"

module DOB(   input reset, clk, emptyFifo,
	      input [23:0] data,
	      output readFifo,
	      output clkReadFifo,
	      output out,
	      input no8b10bModeCnfg,
	      input clkToOutCnfg,
	      input [7:0] emptyRecordCnfg,
	      input test_clk, test_en, test_se, test_si,
	      output test_so
);

  output_data core( .* );

endmodule 