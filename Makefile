WARN = -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Wwrite-strings \
-Wconversion -Wshadow -Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
-Wmissing-declarations -Wuninitialized \
-Wdouble-promotion -Wfloat-equal -Wnull-dereference -Wformat=2 \
\
-Wno-sign-conversion -Wno-implicit-float-conversion -Wno-shorten-64-to-32 -Wno-double-promotion \
-Wno-float-conversion -Wno-string-conversion \
-Wno-gnu-folding-constant -fno-unroll-loops \
-Wno-implicit-int-conversion -Wno-float-equal \
\
-Werror=implicit-function-declaration -Werror=return-type

SANITIZE = # -fsanitize=undefined -fsanitize=address

CC = clang++ $(WARN) $(SANITIZE) # -Rpass=loop-vectorize  -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize

# Source files and object files
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp,obj/%.o,$(SRC))
OPT = -Ofast -march=native -flto -fopenmp-simd -pthread # -fopt-info-vec-missed # -fopenmp # -fopt-info-vec-missed
INC = -Isrc

# Verilog for testing. Synthesized verilog is defined in yosys/synth.ys
VERILOG_MAIN = rtl/matmul_tb.v
VERILOG_SOURCES = src/verilator/test.cpp rtl/matmul.v rtl/sram.v rtl/matmul_tb.v
VERILATOR_FLAGS = -Wall -CFLAGS -std=c++17

.PHONY: compile_commands
compile_commands:
	bear -- make clean all

.PHONY: all
all: test asic

.PHONY: test
test: obj_dir/Vmatmul_tb bin/dram_test
	bin/dram_test
	obj_dir/Vmatmul_tb


bin/%: obj/bin/%.o $(OBJ)
	@mkdir -p bin
	$(CC) $(OPT) $(INC) -g -o $@ $^ -lm

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CC) $(OPT) $(INC) -g -c -o $@ $<

obj/bin/%.o: src/bin/%.cpp
	@mkdir -p obj
	@mkdir -p obj/bin
	$(CC) $(OPT) $(INC) -g -c -o $@ $<

obj_dir/Vmatmul_tb: $(VERILOG_SOURCES)
	verilator $(VERILATOR_FLAGS) \
	  --cc $(VERILOG_MAIN) \
	  --exe src/verilator/test.cpp \
	  --top-module matmul_tb \
	  -Irtl
	make -C obj_dir -f Vmatmul_tb.mk Vmatmul_tb

.PHONY: asic
asic: bin/analyze_yosys $(VERILOG_SOURCES)
	yosys -s yosys/synth.ys | tee obj_dir/yosys.log | bin/analyze_yosys

.PHONY: clean
clean:
	rm -rf bin obj obj_dir
