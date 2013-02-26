
// The following will define NPROC if we are just testing the
// bitvector library on a standard linux machine (without XINU).
////#ifdef TEST
////#define NPROC 75
////#else
////#include <kernel.h>
////#include <proc.h>
////#endif

#ifndef BITVECTOR_H
#define BITVECTOR_H

typedef int * bitvector;

struct bit_structure {
	int size;
	bitvector bv;
};
typedef struct bit_structure * bs_ptr;

bs_ptr bs_alloc(int num_bits);

void set_bit(bs_ptr bsptr, int bit);
void clr_bit(bs_ptr bsptr, int bit);
int  get_bit(bs_ptr bsptr, int bit);

#endif

