////#include <assert.h>
#include <stdlib.h>
////#include <stdio.h>

////#include "s3inst.h"
////#include "def.h"

#include "bitvector.h"


bs_ptr bs_alloc(int num_bits) {

	// local variables
	bs_ptr bsptr;
	bitvector bv;

	// Allocate space for the bit_structure
	bsptr = (bs_ptr) malloc(sizeof(struct bit_structure));
    if (!bsptr)
        return NULL;

	/* Allocate space for the bitvector. In order to know how many 
	 * bytes we need, we need to allocate num_bits/8 + 1. We'll use 
	 * calloc instead of malloc in order to initialize all values to 0. 
	 */ 
	bv = (bitvector) calloc(1, (num_bits / 8) + 1 );
    if (!bv)
        return NULL;

	// Assign bvptr into the bit_structure
	bsptr->bv = bv;

	// Initialize size for bit_structure
	bsptr->size = num_bits;
	
	return bsptr;

}

/*
 * Function to free bit structure memory
 */
void bs_free(bs_ptr bsptr) {

    free(bsptr->bv);
    free(bsptr);

}

/*
 * Function to set a bit in a vector
 */
void set_bit(bs_ptr bsptr, int bit) {
	int block;
	int offset;
    bitvector bv = bsptr->bv;

    // Make sure bit is not greater than size of bitvector
    if (bit >= bsptr->size)
        return;

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

	
	// Now set the bit
	bv[block] |= (1 << offset);

}

/*
 * Function to get a bit value from a bit vector
 */
int get_bit(bs_ptr bsptr, int bit) {
	int block;
	int offset;
    bitvector bv = bsptr->bv;

    // Make sure bit is not greater than size of bitvector
    if (bit >= bsptr->size)
        return 0;

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

	
	// Now set the bit
	return (bv[block] & (1 << offset)) ? 1 : 0;

}


/////*
//// * Function that accepts a bitvector and a 
//// * file pointer as an argument. The file should 
//// * already be opened. The function will take the
//// * bitvector and print it out in a textual
//// * representation to the screen.
//// */
////void printBV(FILE *fptr, bitvector bv) 
////{

////    vardef * def;
////    int i,k;
////    int position;
////    int blocksize = (sizeof(int)*8);
////    int blocks = bvSize / blocksize + 1;
////    int first = 1;

////    /* Print out a left brace */
////    fprintf(fptr,"{ ");

////    /* Iterate over the blocks */
////    for (i = 0; i < blocks; i++) {

////        /* Iterate over each bit in the block */
////        for (k = 0; k < blocksize; k++) {

////            /* Check the bit to see if it is set */
////            if (bv[i] & (1 << k)) {

////                position = k+blocksize*i;	

////                if (first) 
////                    first = 0;
////                else 
////                    fprintf(fptr,", ");


////                /* 
////                 * Print the correct output based on what 
////                 * type the bit is related to.
////                 */
////                switch (unmapBVkey(position)) {

////                    case KEY_REG: 
////                        
////                        fprintf(fptr,"R%d", position);
////                        break;

////                    case KEY_VAR:

////                        /*
////                         * Find the related variable definition
////                         * so we can extract a bit more information.
////                         */
////                        def = findDef(position - (bvSize - 1 - varDefNum - 1));
////                        fprintf(fptr,"MEM[R%d + %d]", def->reg, def->offset);
////                        break;

////                    case KEY_PSUEDO:

////                        /* Print psuedo register */
////                        fprintf(fptr,"$");
////                        break;
////                }

////            }

////        }

////    }


////    /* Print out a left brace */
////    fprintf(fptr," }");

////}

