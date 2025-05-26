#include "Vmatmul.h"
#include "verilated.h"
#include <iostream>

int main() {
    Verilated::traceEverOn(true);
    Vmatmul* dut = new Vmatmul;

    // Reset
    dut->clk = 0; dut->rst = 1; dut->eval();
    dut->clk = 1; dut->eval();
    dut->clk = 0; dut->rst = 0; dut->eval();

    // Start computation
    dut->a = 7;
    dut->b = 3;
    dut->start = 1;

    for (int cycle = 0; cycle < 10; ++cycle) {
        dut->clk ^= 1;
        dut->eval();

        if (dut->done) {
            std::cout << "Result: " << dut->y << std::endl;
            break;
        }
    }

    delete dut;
    return 0;
}
