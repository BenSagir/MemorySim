#ifndef _SIM_MEM
#define _SIM_MEM
#include <queue>

#define MEMORY_SIZE 100
extern char main_memory[MEMORY_SIZE];
using namespace std;
typedef struct page_descriptor
{
    unsigned int V;             //Valid
    unsigned int D;             //Dirty
    unsigned int P;             //Permission
    unsigned int frame;         //frame

}   page_descriptor;

class sim_mem{
    int swapfile_fd;
    int program_fd;
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    
    queue <int> avlFrame;       //available frame queue
    queue <int> naFrame;        //not available - in use frame queue

    page_descriptor * page_table;

    public:
        sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
                        int data_size, int bss_size, int heap_stack_size,
                        int num_of_pages, int page_size);
        ~sim_mem();

        char load(int address);
        void store(int address, char value);
        void make_frame(char * buf);        //in case there is not availble frame in the memory, make one.
        void print_memory();
        void print_swap();
        void print_page_table();                

};
#endif