#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "sim_mem.h"

#define READONLY 0
#define READWRITE 1

char main_memory[MEMORY_SIZE];
/*************************************  constructor ***********************************************************/
sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size,
                 int bss_size, int heap_stack_size, int num_of_pages, int page_size)
{
    this->text_size = text_size;                            //set all the variables given in the ctor
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    this->program_fd = open(exe_file_name, O_RDONLY, 0);    //open exe file
    if (this->program_fd == -1) {                           //if open failed
        perror("exe file open failed");
        this->~sim_mem();
        exit(EXIT_FAILURE);
    }
    this->swapfile_fd = open(swap_file_name, O_CREAT|O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO); //open swap file
    if (this->swapfile_fd == -1)
        perror("swap open failed");
    for (int i = 0; i < MEMORY_SIZE; i++)                   //init main memory
        main_memory[i] = '0';
    this->page_table = (page_descriptor *)malloc(num_of_pages * sizeof(page_descriptor));
    if (this->page_table == NULL) {                         //if malloc failed
        printf("page table malloc failed");
        this->~sim_mem();
        exit(EXIT_FAILURE);
    }
    int text_pages = text_size / page_size;
    for (int i = 0; i < num_of_pages; i++) {                //page_table inint
        if (i < text_pages)
            this->page_table[i].P = READONLY;
        else
            this->page_table[i].P = READWRITE;
        this->page_table[i].V = 0;
        this->page_table[i].D = 0;
        this->page_table[i].frame = -1;
    }
    char *zero = (char *)malloc(page_size * sizeof(char));  //init swap with 0's
    if (zero == NULL) {                                     //if malloc failed
        printf("malloc failed");
        this->~sim_mem();
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < page_size; i++)                     //hold a buf with zeroes in page size
        zero[i] = '0';
    int wrt_swap;
    for (int i = 0; i < num_of_pages; i++) {                //fill swap file with 0 in every logial adrress
        wrt_swap = write(swapfile_fd, zero, page_size);
        if(wrt_swap == -1)
            perror("write to swap ctor failed");
    }
    for (int i = 0; i < MEMORY_SIZE / page_size; i++)       //init availble frame queue
        avlFrame.push(i);
    free(zero);
}
/*********************************************** destructor ***************************************************/
sim_mem::~sim_mem()
{
    free(this->page_table);
    close(program_fd);
    close(swapfile_fd);
}
/************************************************ load *************************************************************/
char sim_mem::load(int address)
{
    char buf[page_size];
    int page = address / page_size;
    int offset = address % page_size;
    if (page_table[page].V == 1)                            //in case page is valid
        return main_memory[(page_table[page].frame * page_size) + offset];
    else {                                                  //if not valid
        int newFrame, rd;
        make_frame(buf);                                    //make sure there is a frame in avlFrame
        newFrame = avlFrame.front();                        //going to put the loaded page into newFrame
        if (page_table[page].P == READONLY)                 //if page is in text
        { 
            page_table[page].V = 1;
            page_table[page].frame = newFrame;
            avlFrame.pop();                                 //take out from available queue
            naFrame.push(newFrame);                         //push into used queue
            lseek(program_fd, page * page_size, SEEK_SET);  //lseek to the right place in the file
            rd = read(program_fd, buf, page_size);
            if (rd == -1) {                                 //if read was unsucsessful
                perror("read program_fd failed");
                this->~sim_mem();
                exit(EXIT_FAILURE);
            }
            for (int i = page_table[page].frame * page_size, j = 0; j < page_size; i++, j++)                                          
                main_memory[i] = buf[j];                    //copy the buf into the main memory
            return main_memory[(page_table[page].frame * page_size) + offset];
        }
        else if (page_table[page].P == READWRITE)       
        {                                                   //if page is in stack/heap/bss
            if (page_table[page].D == 0 && page > (text_size+data_size)/page_size) {   //if the page never been created yet
                fprintf(stderr, "can not load unstored stack/heap");
                return '\0';
            }
            else if (page_table[page].D == 0 && page > text_size/page_size && page <= (text_size+data_size)/page_size){
                page_table[page].V = 1;                         //in case page in data
                page_table[page].frame = newFrame;
                avlFrame.pop();                                 //take out from available queue
                naFrame.push(newFrame);                         //push into used queue
                lseek(program_fd, page * page_size, SEEK_SET);  //lseek to the right place in the file
                rd = read(program_fd, buf, page_size);
                if (rd == -1) {                                 //if read was unsucsessful
                    perror("read program_fd failed");
                    this->~sim_mem();
                    exit(EXIT_FAILURE);
                }
                for (int i = page_table[page].frame * page_size, j = 0; j < page_size; i++, j++)                                          
                    main_memory[i] = buf[j];                    //copy the buf into the main memory
                return main_memory[(page_table[page].frame * page_size) + offset];
            }
            else
            {                                               //if page is dirty
                page_table[page].V = 1;
                page_table[page].frame = newFrame;
                avlFrame.pop();                             //take out from available queue
                naFrame.push(newFrame);                     //push into used queue
                lseek(swapfile_fd, page * page_size, SEEK_SET); //lseek to the right place in the file
                rd = read(swapfile_fd, buf, page_size);
                if (rd == -1) {                             //if read was unsucsessful                
                    perror("read swapfile_fd failed");
                    this->~sim_mem();
                    exit(EXIT_FAILURE);
                }
                for (int i = page_table[page].frame * page_size, j = 0; j < page_size; i++, j++)
                    main_memory[i] = buf[j];                //copy the buf into the main memory
                return main_memory[(page_table[page].frame * page_size) + offset];
            }
        }
    }
}
/****************************************************** store ***************************************************/
void sim_mem::store(int address, char value)
{
    char buf[page_size];
    int page = address / page_size;
    int offset = address % page_size;
    if (page_table[page].P == READONLY) {                   //if want to store into read only page
        fprintf(stderr, "can not store to read only file");
        return;
    }
    if (page_table[page].V == 1) {                          //if page is vaild
        main_memory[(page_table[page].frame * page_size) + offset] = value;
        page_table[page].D = 1;
        return;
    }
    else                                                    //if page is not valid
    {
        int newFrame, rd;
        make_frame(buf);                                    //make sure there is a frame in avlFrame
        newFrame = avlFrame.front();                        //going to put the loaded page into newFrame
        if (page_table[page].D == 1)
        {                                                   //and dirty therefor in the swap
            page_table[page].V = 1;
            page_table[page].frame = newFrame;
            avlFrame.pop();                                 //take out from available queue
            naFrame.push(newFrame);                         //push into used queue
            lseek(swapfile_fd, page * page_size, SEEK_SET); //lseek to the right place in the file
            rd = read(swapfile_fd, buf, page_size);
            if (rd == -1) {                                 //if read was unsucsessful
                perror("read swapfile_fd failed");
                this->~sim_mem();
                exit(EXIT_FAILURE);
            }
            for (int i = page * page_size, j = 0; j < page_size; i++, j++)
                main_memory[i] = buf[j];                    //copy the buf into the main memory
            main_memory[(page_table[page].frame * page_size) + offset] = value;
            return;
        }
        else                                                //if not valid and not dirty
        {                                                   //if store to data
            if(page > text_size/page_size && page <= (text_size+data_size)/page_size){
                page_table[page].V = 1;
                page_table[page].frame = newFrame;
                avlFrame.pop();                                 //take out from available queue
                naFrame.push(newFrame);                         //push into used queue
                lseek(program_fd, page * page_size, SEEK_SET);  //lseek to the right place in the file
                rd = read(program_fd, buf, page_size);
                if (rd == -1) {                                 //if read was unsucsessful
                    perror("read program_fd failed");
                    this->~sim_mem();
                    exit(EXIT_FAILURE);
                }
                for (int i = page_table[page].frame * page_size, j = 0; j < page_size; i++, j++)                                          
                    main_memory[i] = buf[j];
                main_memory[(page_table[page].frame * page_size) + offset] = value;
                return;
            }
            else{                                           //else store into stack/heap/bss
                page_table[page].V = 1;
                page_table[page].frame = newFrame;
                avlFrame.pop();                                 //take out from available queue
                naFrame.push(newFrame);                         //push into used queue
                main_memory[(page_table[page].frame * page_size) + offset] = value;
                page_table[page].D = 1;
                return;
            }
        }
    }
}
/*********************************************************************************************************************
=======================================  print functions ============================================================
**********************************************************************************************************************/
void sim_mem::print_memory()
{
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++)
        printf("[%c]\n", main_memory[i]);
}
void sim_mem::print_swap()
{
    char *str = (char *)malloc(this->page_size * sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET);
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < page_size; i++)
            printf("%d - [%c]\t", i, str[i]);
        printf("\n");
    }
    free(str);
}
void sim_mem::print_page_table()
{
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission\t Frame\n");
    for (i = 0; i < num_of_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[i].V,
               page_table[i].D,
               page_table[i].P,
               page_table[i].frame);
    }
}
/******************************************* make frame ***************************************************************/
void sim_mem::make_frame(char *buf)
{
    if (avlFrame.empty())
    {
        int tempFrame = naFrame.front();                    //keep the first element of the used frame as temp
        for (int i = 0; i < num_of_pages; i++) {
            if (page_table[i].frame == tempFrame) {
                page_table[i].V = 0;
                page_table[i].frame = -1;
                if (page_table[i].D == 1) {                 //if dirty
                    for (int j = tempFrame * page_size, k = 0; k < page_size; j++, k++) {
                        buf[k] = main_memory[j];            //copy the certian page into the buf
                        main_memory[j] = '0';               //clear the frame in the main memory 
                    }
                    int wrt_swap;
                    lseek(swapfile_fd, i * page_size, SEEK_SET);
                    wrt_swap = write(swapfile_fd, buf, page_size);
                    if(wrt_swap == -1)
                        perror("write to swap failed in clearing memory");
                }
                else {                                      //if not dirty
                    for (int g = tempFrame * page_size; g < (tempFrame + 1) * page_size; g++)
                        main_memory[g] = '0';               //clear the frame in the main memory
                }
            }
        }
        naFrame.pop();                                      //remove the tempframe from the used freame queue
        avlFrame.push(tempFrame);                           //push it to the available frame queue
    }
}