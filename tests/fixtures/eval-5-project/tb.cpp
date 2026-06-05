#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtop.h"

int main(int argc, char** argv) {
  VerilatedContext* contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  Verilated::traceEverOn(true);

  Vtop* top = new Vtop{contextp};
  VerilatedVcdC* tfp = new VerilatedVcdC;
  top->trace(tfp, 99);
  tfp->open("wave.vcd");

  for (int i = 0; i < 20; i++) {
    top->clk = i % 2;
    top->rst = i < 2 ? 1 : 0;
    top->eval();
    tfp->dump(contextp->time());
    contextp->timeInc(1);
  }

  top->final();
  tfp->close();
  delete tfp;
  delete top;
  delete contextp;
  return 0;
}
