//#include <stdio.h>

#ifndef BITVECTOR_H
#define BITVECTOR_H

typedef int * bitvector;

struct bit_structure {
	int size;
	bitvector bv;
};
typedef struct bit_structure * bs_ptr;


bs_ptr bs_alloc(int num_bits);
void bs_free(bs_ptr bsptr);

void set_bit(bs_ptr bsptr, int bit);
void clear_bit(bs_ptr bsptr, int bit);
int  get_bit(bs_ptr bsptr, int bit);

//void printBV(FILE *fptr, bitvector bv);



#endif

