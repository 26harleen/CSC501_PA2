

#include <lock.h>

LOCAL void unblock(int lock, int item);

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
                unlock(lock, item);

                // Move to next item;
                item = q[item].qprev;
            }
        }

        // State 2 - Select highest priority process. If waiting
        // READs/WRITEs have same priority then select READ. 
        if (lptr->nr == 0 && lptr->nw == 0) {

            // If there are no waiting procs then nothing to do
            if (isempty(lptr->lqhead))
                continue;

            // Choose highest priority waiting proc.
            item = q[lptr->lqtail].qprev;

            // Are there multiple with the same priority? If not, no
            // more to do.
            if (q[item].qkey != q[q[item].qprev].qkey) {
                unblock(lock, item);
                continue;
            }

            // Ok, We have multiple items in the queue at the highest
            // priority. Find the first read (if there is one), and
            // the first write (if there is one).
            while (q[item].qkey == q[q[item].qprev].qkey) { 
                if (q[item].qtype == WRITE)
                    firstwrite = firstwrite ? firstwrite : item;
                else
                    firstread = firstread ? firstread : item;
                item = q[item].qprev;
            }

            // Are all of the equal priority items reads? If so,
            // choose the first one and continue.
            if (!firstwrite) {
                unblock(lock, firstread);
                continue;
            }

            // Are all of the equal priority items writes? If so,
            // choose the first one and continue.
            if (!firstread) {
                unblock(lock, firstwrite);
                continue;
            }

            // Ok, we have at least one read and one write at the
            // highest priority. If this write has been sidelined 3 times
            // then choose the write. Else choose the read.
            // XXX may need to check all writes
            if (q[firstwrite].qpassed == 3) {
                unblock(lock, firstwrite);
            } else {
                unblock(lock, firstread);
                q[firstwrite].qpassed++;
            }
        }

    }
    restore(ps);
    return(OK);
}


LOCAL void unblock(int lock, int item) {

    register struct lentry *lptr;

    // Get the pointer to the lock entry
    lptr = &locks[lock];

    // Increment the counter for this type of lock
    if (q[item].qtype == WRITE)
        lptr->nw++;
    else
        lptr->nr++;

    // Remove the item from the lock queue and make it 
    // ready to be scheduled.
    dequeue(item);
    ready(item, RESCHYES);
}
    
