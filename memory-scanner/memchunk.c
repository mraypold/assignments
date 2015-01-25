/*
 * Skeleton of mem.c follows closely Bob Beck's sig2.c example
 */

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memchunk.h"

static sigjmp_buf jumpbuf;
static struct sigaction sa;
static void *ptr;
static void *start;
static int chunksize = 1;
static int chunkcount = 0;
static int assumedsize = 0;
static int rw = -1;
static int old_rw = -1;

/* Return to jump point */
void handler(int signum) {
        siglongjmp(jumpbuf, 1);
}   

void build_chunk(struct memchunk *chunk_list) {
        /* Insert the chunk into the list */
        struct memchunk new_mem_chunk;

        new_mem_chunk.start = start - (int)0x1000;
        new_mem_chunk.length = (chunksize - 1)*getpagesize();
        new_mem_chunk.RW = old_rw;
        
        chunk_list[chunkcount] = new_mem_chunk;
                
        chunkcount++;
        chunksize = 1;
        start = ptr;
}

void check_chunk(struct memchunk *chunk_list) {
        
        /* New block; reset chunk size */
        if(old_rw == -1){
            chunksize = 1;
            start = ptr;
        }

        if(old_rw != rw){
            /* End of write block, start of read block */
            if((rw == 0) && (old_rw == 1)){
                if(assumedsize > chunkcount){
                    build_chunk(chunk_list);
                }
                else{
                    chunkcount++;
                }
            }

            /* End of read block, start of write block */
            if((rw == 1) && (old_rw == 0)){
                chunksize--;
                if(assumedsize > chunkcount){
                    build_chunk(chunk_list);
                }
                else{
                    chunkcount++;
                }
            }

            /* End of read block */
            if((rw == -1) && (old_rw == 0)){
                if(assumedsize > chunkcount){
                    build_chunk(chunk_list);
                }
                else{
                    chunkcount++;
                }
            }

            /* End of write block */
            if((rw == -1) && (old_rw == 1)){
                if(assumedsize > chunkcount){
                    build_chunk(chunk_list);
                }
                else{
                    chunkcount++;
                }
            }
        }
        else{
            chunksize++;
        }
         
        old_rw = rw;
        rw = -1;
       
}

/* Check if the memory can be read */
void readmem() {
        char * read = malloc(sizeof(char));
        memcpy(read, ptr, 1);

        rw = 0;
}

/* Checks if the memory is writable */
void writemem() {
        readmem();
        memset(ptr, 0, 1);

        rw = 1;
        
        /* Return to handler to replace stack */
        raise(SIGSEGV);
}

int get_mem_layout(struct memchunk *chunk_list, int size)
{       
        assumedsize = size;
        
        /* Set up the signal handler */
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGSEGV, &sa, NULL);

        ptr = (void *)0x00000000;
        char * read = malloc(sizeof(char));

        start = ptr;

        /* While loop ends one short so not to overflow */
        while(ptr <= (void *)0xffffe000){
            if (sigsetjmp(jumpbuf, 1) == 0){
                check_chunk(chunk_list);
              
                writemem();

                ptr = ptr + (int)0x1000;
            }
            /* No segfault occured, increment to next pointer */
            else{
                ptr = ptr + (int)0x1000;                
                chunksize++;
            }
        }

        /* check if 0xFFFFFFFF is a mem block */
        check_chunk(chunk_list);

        return chunkcount;
}

