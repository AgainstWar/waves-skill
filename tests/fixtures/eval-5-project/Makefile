# Simple Verilator project without tracing
.PHONY: all run clean

all: obj_dir/Vtop

obj_dir/Vtop: top.v tb.cpp
	verilator --cc --exe --build --trace -j 0 -Mdir obj_dir top.v tb.cpp

run: obj_dir/Vtop
	./obj_dir/Vtop

clean:
	rm -rf obj_dir
