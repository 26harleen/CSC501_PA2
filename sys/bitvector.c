
// The following will set up a few things depending on whether we are
// compiling for XINU or compiling to test on a Linux system.
#ifdef TEST
#include "bitvector.h"
#include <stdlib.h>
#define MALLOC(x) malloc(x);
#else
#include <kernel.h>
#include <bitvector.h>
#define MALLOC(x) getmem(x); // XINU has getmem(), not malloc()
#endif

#define BIT_SET 1
#define BIT_CLR 2
#define BIT_GET 3


bs_ptr bs_alloc(int num_bits) {

	// local variables
    int i;
	bs_ptr bsptr;
	bitvector bv;

	// Allocate space for the bit_structure
	bsptr = (bs_ptr) MALLOC(sizeof(struct bit_structure));
    if (!bsptr)
        return NULL;

	// Allocate space for the bitvector. In order to know how many 
	// bytes we need, we need to allocate num_bits/8 + 1. 
	bv = (bitvector) MALLOC((num_bits / 8) + 1);
    if (!bv)
        return NULL;

	// Assign bvptr into the bit_structure
	bsptr->bv = bv;

	// Initialize size for bit_structure
	bsptr->size = num_bits;

    // Make sure all bits are 0
    for(i=0; i < num_bits; i++)
        clr_bit(bsptr, i);

	return bsptr;
}

/*
 * Generic function to operate on bit structure.
 */
int _access_bit(bs_ptr bsptr, int bit, int op) {
	int block;
	int offset;
    bitvector bv = bsptr->bv;

	// 
	// Since bitvector is based off the int data type then
	// we must account for the fact that array indexing will
	// skip sizeof(int) every time we increment the index. We
	// must figure out which block the bit is in. To do this
	// we will take the bit number and divide by the number
	// of bits in a basic block (sizeof(int)*8). 
	//
	// For example, if we want bit 100 and sizeof(int) is 4
	// then it will be in block
	// 		
	//		100/(sizeof(int)*8) = 3
	//		
	// This actually means that it is in the 4th basic block 
	// since array indexing starts at 0. 
	// 
	block = bit / (sizeof(int)*8); 


	// 
	// We already know the block, now find the offset local
	// to the block.
	// 
	offset = bit % (sizeof(int)*8);


    switch(op) {

        case BIT_GET:
            return (bv[block] & (1 << offset)) ? 1 : 0;
        case BIT_SET:
            bv[block] |= (1 << offset);
            return 1; // Doesn't matter what we return
        case BIT_CLR:
            bv[block] &= ~(1 << offset);
    }

    // Should never get here
    return 0;
}

/*
 * Function to set a bit in a vector
 */
void set_bit(bs_ptr bsptr, int bit) {
    _access_bit(bsptr, bit, BIT_SET);
}

/*
 * Function to clear a bit in a vector
 */
void clr_bit(bs_ptr bsptr, int bit) {
    _access_bit(bsptr, bit, BIT_CLR);
}

/*
 * Function to get a bit value from a bit vector
 */
int get_bit(bs_ptr bsptr, int bit) {
    return _access_bit(bsptr, bit, BIT_GET);
}
