

#include <lock.h>


/*
 * Simultaneously release one or more locks. Takes a variable number
 * of arguments. The number of locks to be released is passed in as
 * the first argument. 
 */
int releaseall(int numlocks, int ldes1) {
    STATWORD ps;    
    register struct lentry *lptr;
    int i;
    int lock;

    // Disable interrupts - needs to be atomic
    disable(ps);

    // Iterate over all of the specified resources and release
    // one instance of them.
    for (i=0; i<numlocks; i++) {

        lock = ldes1[i];
        lptr = &locks[lock];

        if (isbadlock(lock) || lptr->lstate==LFREE) {
            restore(ps);
            return(SYSERR);
        }

        // Increment lock counter and decrement type counter
        lptr->lcnt++;
        if (lptr->ltype == WRITE)
            lptr->nw--;
        if (lptr->ltype == READ)
            lptr->nr--;


        // What is the current state of the resource? Possibilities are:
        //
        //      Available               Type
        //      -----------------     ----------
        //  1 -  (lcnt == 1) - No more processes waiting on resource
        //  2 -  (lcnt <  1) - Processes waiting       Write
        //  1 -  Yes  (lcnt == 1)        N/A
        //  2 -  No   (lcnt <  1)       Write
        //  3 -  Maybe(lcnt <  1)       Read

        // State 1 - Resource is available and no more processes waiting.
        if (lptr->lcnt == 1) {
            // Nothing to do
        }
        // make sure next thing done here is return

        // State 2 - Write already has lock. Add to queue and wait. 
        } else if (lptr->lcnt < 1 && lptr->ltype == WRITE) {

            wait = 1;


        // State 3 - Read already has lock. We may be able to grant the
        //           new process access if:
        //
        //              - There is not a higher priority writer waiting
        //              - A reader has not been given access over an
        //                equal priority writer 3 times in a row (starvation).
        //
        // Go through all items in the lock q. 

        if ((sptr->semcnt++) < 0)
            ready(getfirst(sptr->sqhead), RESCHYES);
    }
    restore(ps);
    return(OK);
}
    
