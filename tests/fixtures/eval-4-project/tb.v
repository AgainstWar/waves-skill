module tb_simple;
  reg clk = 0;
  reg rst = 1;
  reg [3:0] count = 0;

  always #5 clk = ~clk;

  always @(posedge clk) begin
    if (!rst)
      count <= count + 1;
  end

  initial begin
    $dumpfile("waveform.vcd");
    $dumpvars(0, tb_simple);
    #10 rst = 0;
    #100 $finish;
  end
endmodule
