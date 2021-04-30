#include "sim_mem.h"
#include <stdio.h>

int main(){

    char* exe = "exe.txt";
    char * swap = "swap";
    sim_mem test(exe, swap, 25,50,25,25,25,5);
    test.store(98, 'x');
    char val = test.load(98);
    char val2 = test.load(2);
    char val3 = test.load(14);
    char val4 = test.load(6);
    char val5 = test.load(16);
    char val6 = test.load(22);
    test.store(29, 'a');
    test.store(31, 'b');
    test.store(38, 'c');
    test.store(40,'d');
    test.store(47,'e');
    test.store(50,'f');
    test.store(56, 'g');
    test.store(60,'h');
    test.store(67,'i');
    test.store(71,'j');
    test.store(79,'k');
    test.store(80,'l');
    test.store(88,'m');
    test.store(90,'n');
    test.store(101,'o');
    test.store(106,'p');
    test.store(111, 'q');
    test.store(116, 'r');
    test.store(121, 's');
    printf("%c,%c,%c", val, val2, val3);
    test.print_memory();
    test.print_page_table();
    test.print_swap();
    return 0;
}