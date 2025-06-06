
# HwAccel

HwAccel is a partial design and implementation of a LLM inference hardware core. The goal is to develop a design that could (in theory) be used to create a real ASIC.

Main design goals:
- 16 nm process (scaled from 45 nm)
- 1.4 GHz clock speed
- 4 GB LPDDR5 memory per core (50 GB/s)
- Internal bus width of 256 bits (45 GB/s)
- Compute: 40 GMAC

Project summary:
- RTL: Verilog
- Simulation: C++ / Verilator
- Synthesis: Yosys + NandgateOpenCellLibrary (FreePDK45)
- Build system: Make

## Project Status

The project is in its very early stages. The build system works, and could be used as an example on how to structure such project. There is a simple (sequential) 8 bit matmul core + test, and a C++ DRAM implementation + test.

## General design

Effectively this is a memory controller with a small amount of logic attached to it. The matrix multiplications (and other LLM operations) are simple and easy to paralellize. The main challenge is to get the data from/to memory.

Data which is used more than once for a single matrix multiplication (e.g. the input vector) is stored in SRAM, with a latency of 1 cycle. Other data is streamed from DRAM through a 256 bit AXI4 bus. The DRAM controller itself is not part of this project. This can either be commercial IP or an open source one (though availability of fast DRAM controllers is limited).

This hardware design doesn't use tiling, this is expected to be done by the controlling software. The hardware is expected to be used in a tensor parallel way (e.g. like [Diolkos](https://github.com/gckrol/diolkos)).

# Usage

Install dependencies from your package manager. For Ubuntu/Debian use:

````bash
sudo apt install clang verilator yosys
````

## Testing

To build and run the (C++/Verilator) tests use:

````
make test
````

## Synthesis

To synthesize and analyse the ASIC design use:

````bash
make asic
````

This will print a summary:

````
=== Performance Summary ===
   Gates                : 564
   Memory Bandwidth     : 50.00 GB/s
   Target Performance   : 40.00 GMAC

-> 45nm:
   Area        : 682.82 µm²
   Freq        : 0.83 GHz
   GMAC        : 0.83
   Cores needed: 47.90
   ASIC cost   : $0.114
   Packaging   : $10.000
   PCB         : $30.000
   RAM (4GB)   : $20.000
   Total cost  : $60.114

-> 16nm:
   Area        : 144.77 µm²
   Freq        : 1.40 GHz
   GMAC        : 1.40
   Cores needed: 28.56
   ASIC cost   : $0.062
   Packaging   : $7.333
   PCB         : $30.000
   RAM (4GB)   : $20.000
   Total cost  : $57.395

Cost comparison: 16nm is 95.48% of 45nm cost
````

Note that these numbers are quite rough approximations. These also don't include the area/cost of the DRAM controller.

