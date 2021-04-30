all: sim_mem.h sim_mem.cpp main.cpp
	g++ sim_mem.cpp main.cpp -o ex4
all-GDB: sim_mem.h sim_mem.cpp main.cpp
	g++ -g sim_mem.cpp main.cpp -o ex4