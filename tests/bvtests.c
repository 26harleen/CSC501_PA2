
// To Test cp bvtests.c bitvector.c bitvector.h to a directory and run
// gcc -g -O0 bitvector.c bvtests.c
// ./a.out
#include <stdio.h>
#include "bitvector.h"


void main() {

    bs_ptr bsptr;
   
    if (!(bsptr = bs_alloc(75))) {
       printf("Failed to create bitvector\n");
    }

    set_bit(bsptr, 0);
    set_bit(bsptr, 10);
    set_bit(bsptr, 21);
    set_bit(bsptr, 33);
    set_bit(bsptr, 50);
    set_bit(bsptr, 74);

    printf("Value of bit 0: %d\n", get_bit(bsptr, 0));
    printf("Value of bit 10: %d\n", get_bit(bsptr, 10));
    printf("Value of bit 21: %d\n", get_bit(bsptr, 21));
    printf("Value of bit 33: %d\n", get_bit(bsptr, 33));
    printf("Value of bit 50: %d\n", get_bit(bsptr, 50));
    printf("Value of bit 74: %d\n", get_bit(bsptr, 74));

    printf("Value of bit 1: %d\n", get_bit(bsptr, 1));
    printf("Value of bit 11: %d\n", get_bit(bsptr, 11));
    printf("Value of bit 22: %d\n", get_bit(bsptr, 22));
    printf("Value of bit 34: %d\n", get_bit(bsptr, 34));
    printf("Value of bit 51: %d\n", get_bit(bsptr, 51));
    printf("Value of bit 73: %d\n", get_bit(bsptr, 73));

    bs_free(bsptr);

}
