
#include <kernel.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

/*
 * Creates a lock and returns a lock descriptor that can be used in
 * further calls to refer to this lock. This call should return SYSERR
 * if there are no available entries in the lock table. The number of
 * locks allowed is NLOCKS.
 */
int lcreate() {

    STATWORD ps;    
    int lock;

    disable(ps);
    if ((lock=newlock())==SYSERR ) {
        restore(ps);
        return(SYSERR);
    }
    // lqhead and lqtail were initialized at system startup
    restore(ps);
    return(lock);
}

/*
 * allocate an unused lock and return its index
 */
LOCAL int newlock() {
    int i, ld, lock;

    for (i=0 ; i < NLOCKS ; i++) {
        // Get the next lock and update counter
        lock=nextlock--;

        // Wrap if necessary
        if (nextlock < 0)
            nextlock = NLOCKS-1;

        // Is the current lock free? 
        if (locks[lock].lstate == LFREE) {

            // Initialize the locks state information
            locks[lock].lstate = LUSED;
            locks[lock].lnr    = 0;
            locks[lock].lnw    = 0;
            locks[lock].lversion++;

            // Generate the unique lock descriptor. This is
            // composed of lock index bits plus lock version
            // bits. 
            ld = (locks[lock].lversion << LOCK_INDEX_BITS) | lock;

            return(ld);
        }
    }
    return(SYSERR);
}
