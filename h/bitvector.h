
// The following will define NPROC if we are just testing the
// bitvector library on a standard linux machine (without XINU).
#ifdef TEST
#define NPROC 75
#else
#include <kernel.h>
#include <proc.h>
#endif

#ifndef BITVECTOR_H
#define BITVECTOR_H

typedef int * bitvector;

struct bit_structure {
    int bv[NPROC/8 + 1];
};
typedef struct bit_structure * bs_ptr;


void set_bit(bs_ptr bsptr, int bit);
void clear_bit(bs_ptr bsptr, int bit);
int  get_bit(bs_ptr bsptr, int bit);

//void printBV(FILE *fptr, bitvector bv);



#endif

