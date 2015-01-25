#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "memchunk.h"

int get_mem_layout(struct memchunk*, int);
pid_t getpid(void);

int main(int argc, char *argv[])
{
        int arraysize = 10;
        int count = 0;
        int i = 0;

        struct memchunk *chunk_list = (struct memchunk *)
                malloc(arraysize*sizeof(struct memchunk));
        
        printf("Pagesize %d\n", getpagesize());

        printf("Calling get_mem_layout\n");
        count = get_mem_layout(chunk_list, arraysize);

        printf("Memchunks: %d\n", count);

        /* Iterate through the chunk_list */
        for(i = 0; i < arraysize; i++){
            printf("Address %p ", chunk_list[i].start);
            printf("Length %lu ", chunk_list[i].length);
            printf("RW %d\n", chunk_list[i].RW);
        }

        printf ("Memory file: /proc/%d/maps\n", getpid());
        printf("Going to sleep to check mem layout\n");
        sleep(10);
        fflush(NULL);
        return 0;
}
