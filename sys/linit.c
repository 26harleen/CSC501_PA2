
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

struct lentry locks[NLOCKS];
int nextlock;

/*
 * Initialize the locking structures.
 */
void linit() {

    struct lentry *lptr;
    int i;
    nextlock = NLOCKS - 1;

    for (i=0 ; i < NLOCKS ; i++) {
        lptr = &locks[i];
        lptr->lstate       = LFREE;
        lptr->lprocs_bsptr = bs_alloc(NPROC);
        lptr->lversion     = 0;
        lptr->lqtail       = 1 + (lptr->lqhead = newqueue());
    }
}
