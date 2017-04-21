//
// Verilog Module ReadOutControl_lib.deglitcher_And
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 13:39:47 03/18/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//


module deglitcher_And  (In, Out);
// 
// Verilog Module deglitcher.v
//

//
// Developed with Library: DC 2009.06-SP2 Artisan 2009.q1.v1


input In;
output Out;
wire net1, net2, net3, net4, net5, net6, net7, net8, net9, net10, net11, delayed_input;

DLY4X4TF delaycell1 (.Y(net1), .A(In) );
DLY4X4TF delaycell2 (.Y(net2), .A(net1) );
DLY4X4TF delaycell3 (.Y(net3), .A(net2) );
DLY4X4TF delaycell4 (.Y(net4), .A(net3) );
DLY4X4TF delaycell5 (.Y(net5), .A(net4) );
DLY4X4TF delaycell6 (.Y(net6), .A(net5) );
DLY4X4TF delaycell7 (.Y(net7), .A(net6) );
DLY4X4TF delaycell8 (.Y(net8), .A(net7) );
DLY4X4TF delaycell9 (.Y(net9), .A(net8) );
DLY4X4TF delaycell10 (.Y(net10), .A(net9) );
DLY4X4TF delaycell11 (.Y(net11), .A(net10) );
DLY4X4TF delaycell12 (.Y(delayed_input), .A(net11) );

assign Out = In && delayed_input;
endmodule
