

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
    int firstwrite;
    int firstread;

    // Disable interrupts - needs to be atomic
    disable(ps);

    // Iterate over all of the specified resources and release
    // one instance of them.
    for (i=0; i<numlocks; i++) {

        lock = ldes1[i];
        lptr = &locks[lock];
        firstwrite = 0;
        firstread  = 0;

        if (isbadlock(lock) || lptr->lstate==LFREE) {
            restore(ps);
            return(SYSERR);
        }

        // Update the count for this resource
        if (lptr->nw == 1)
            lptr->nw--;
        else
            lptr->nr--;

        // What is the current state of the resource? Possibilities are:
        //
        //  1 - There are still readers with access. Let all readers
        //      with priority higher than highest priority writer in.
        //  2 - No one has access. Select highest priority process in
        //      wait queue.

        // State 1 - 
        if (lptr->nr != 0 && lptr->nw == 0) {

            // Since priority goes from head (lower priority) to
            // tail (higher priority) we will iterate backwards until
            // we find a waiting WRITE. All READs we encounter before
            // then are READs with higher priority than the highest
            // priority WRITE. 
            item = q[lptr->lqtail].qprev;
            while (item != lptr->lqhead) { 
                if (q[item].qtype == WRITE)
                    break; 
                
                // This a read with higher priority than highest
                // priority write. dequeue from lock queue and make it
                // ready to be scheduled.
                lptr->nr++;
                dequeue(item);
                ready(item, RESCHYES);

                item = q[item].qprev;
            }
            restore(ps);
            return(OK);
        }

        // State 2 - Select highest priority process. If waiting
        // READs/WRITEs have same priority then select READ. 
        if (lptr->nr == 0 && lptr->nw == 0) {

            // If there are no waiting procs then nothing to do
            if (isempty(lptr->lqhead)) {
                restore(ps);
                return(OK);
            }

            // Choose highest priority waiting proc.
            item = q[lptr->lqtail].qprev;

            // Are there multiple with the same priority? If not, no
            // more to do.
            if (q[item].qkey != q[q[item].qprev].qkey)
                break;

            // Is one of the highest priority waiting procs a write?
            // If not, no more to do.
            while (q[item].qkey == q[q[item].qprev].qkey) { 
                if (q[item].qtype == WRITE)
                    firstwrite = firstwrite ? firstwrite : item;
                else
                    firstread = firstread ? firstread : item;
                item = q[item].qprev;
            }
            if (!firstwrite)
                break;

            // Ok, we have a write with equal priority to highest
            // priority read. If this write has been sidelined 3 times
            // then choose the write. Else choose the read.
            if (something)
                item = firstwrite;
            else {
                item = firstread;
                increment something.
            }
                


        }











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
    
