#ifndef MEMCHUNK_H
#define MEMCHUNK_H

#include <setjmp.h> 

struct memchunk {
        void *start;
        unsigned long length;
        int RW;
};

/* Non standard C declarations */
void siglongjmp(sigjmp_buf env, int val);
int getpagesize(void);

/* mem.c definitions */
int get_mem_layout(struct memchunk*, int);
void handler(int);
void build_chunk(struct memchunk*);
void check_chunk(struct memchunk*);
void redmem();
void writemem();

#endif
