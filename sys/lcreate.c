
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
    int lock;
    int i;

    for (i=0 ; i < NLOCKS ; i++) {
        lock=nextlock--;
        if (nextlock < 0)
            nextlock = NLOCKS-1;
        if (locks[lock].lstate == LFREE) {
            locks[lock].lstate = LUSED;
            locks[lock].lnr    = 0;
            locks[lock].lnw    = 0;
            return(lock);
        }
    }
    return(SYSERR);
}
