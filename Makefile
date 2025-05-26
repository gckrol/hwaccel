WARN = -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Wwrite-strings \
-Wconversion -Wshadow -Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
-Wmissing-declarations -Wuninitialized \
-Wdouble-promotion -Wfloat-equal -Wnull-dereference -Wformat=2 \
\
-Wno-sign-conversion -Wno-implicit-float-conversion -Wno-shorten-64-to-32 -Wno-double-promotion \
-Wno-float-conversion -Wno-string-conversion \
-Wno-gnu-folding-constant -fno-unroll-loops \
-Wno-implicit-int-conversion \
\
-Werror=implicit-function-declaration -Werror=return-type

SANITIZE = # -fsanitize=undefined -fsanitize=address

CC = clang++ $(WARN) $(SANITIZE) # -Rpass=loop-vectorize  -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize

# Source files and object files
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp,obj/%.o,$(SRC))
OPT = -Ofast -march=native -flto -fopenmp-simd -pthread # -fopt-info-vec-missed # -fopenmp # -fopt-info-vec-missed
INC = -Isrc

VERILOG_SOURCES = rtl/matmul.v
VERILATOR_FLAGS = -Wall -CFLAGS -std=c++17

.PHONY: compile_commands
compile_commands:
	bear make clean all

.PHONY: all
all: bin/test obj_dir/Vmatmul

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

obj_dir/Vmatmul: $(VERILOG_SOURCES) src/verilator/main.cpp
	verilator $(VERILATOR_FLAGS) \
	  --cc $(VERILOG_SOURCES) \
	  --exe src/verilator/main.cpp \
	  --top-module matmul
	make -C obj_dir -f Vmatmul.mk Vmatmul

.PHONY: synth
synth:
	yosys -s yosys/synth.ys

.PHONY: clean
clean:
	rm -rf bin obj obj_dir
