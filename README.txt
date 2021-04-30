bensg
Ben Sagir ID 206222200

The following program is a simple simulation of Paging Memory Management.
the virtual memory is dividing to pages and in every single time only some of the pages is boarded in the physical memory.
This program simulate the operation system action when manageing the memory with paging.

How to compile:
	Option 1:
		Open Linux.
		Open the attached folder.
		Right click and chose "Open in Treminal".
		Type in "g++ sim_mem.cpp main.cpp -o ex4"

	Option 2:
		Open Linux.
		Open Visual Studio Code.
		Open the attached folder in Visual Studio Code.
		Press Ctrl+Shift+B.
		Press Enter twice.


How to run:
	Option 1:
		Open Llinux.
		Open the attached folder in "Files".
		Right click and chose "Open in Treminal".
		On the treminal type in "./ex4" and press enter.
	
	Option 2:
		Open Linux.
		Open Visual Studio Code.
		Open the attached folder in Visual Studio Code.
		Press Ctrl+F5.
	
	
	When running, the program will handle page table.
	The supported action are load a char from the memory or store a char into the memory.
	In the specific main code, there will be 6 load calls and 18 store calls.
	The pages that has been chaged during the run be store function, will be saved in "swap" file.
	If and when the chaged page needed to go back into the main memory, it will be read from "swap" file.
		

The folder contain 3 differnt files:
	sim_mem.h - header, contain class definiton, declaring of function.
	sim_mem.cpp - class implementation, contain the actual code of the function.
	main.cpp - the main program, contain load and store calls.
	exe.txt - used as logical memory. in real life (out of the simulation) this is an executable program file.
	
	
The input required is an executable file to be considered as logical memory.
the name of the exe file must be chosed in the main file.
	

The output of the program is a print the following data structures at the final state of the run:
	1. The physical memory.
	2. The Page table.
	3. The swap file.
