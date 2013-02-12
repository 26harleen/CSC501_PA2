

#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>

LOCAL void unblock(int lock, int item);

/*
 * Simultaneously release one or more locks. Takes a variable number
 * of arguments. The number of locks to be released is passed in as
 * the first argument. 
 */
int releaseall(numlocks, ldes1)
    int numlocks;
    int ldes1;
{
    STATWORD ps;    
    register struct lentry *lptr;
    int i, item, prev, lock;
    int firstwrite;
    int firstread;

    // Disable interrupts - needs to be atomic
    disable(ps);

    // Iterate over all of the specified resources and release
    // one instance of them.
    for (i=0; i<numlocks; i++) {

        lock = *((&ldes1) + i);
        kprintf("checking lock %d\n", lock);
        lptr = &locks[lock];
        firstwrite = 0;
        firstread  = 0;

        if (isbadlock(lock) || lptr->lstate==LFREE) {
            restore(ps);
            return(SYSERR);
        }

        // Update the count for this resource
        if (lptr->lnw == 1)
            lptr->lnw--;
        else
            lptr->lnr--;

        // What is the current state of the resource? Possibilities are:
        //
        //  1 - There are still readers with access. Let all readers
        //      with priority higher than highest priority writer in.
        //  2 - No one has access. Select highest priority process in
        //      wait queue.

        // State 1 - 
        if (lptr->lnr != 0 && lptr->lnw == 0) {

            // Since priority goes from head (lower priority) to
            // tail (higher priority) we will iterate backwards until
            // we find a waiting WRITE. All READs we encounter before
            // then are READs with higher priority than the highest
            // priority WRITE. 
            item = q[lptr->lqtail].qprev;
            while (item != lptr->lqhead) { 
                if (q[item].qtype == WRITE)
                    break; 

                // Save off prev item (needed because unblock() will 
                // dequeue item from the list.
                prev = q[item].qprev;
                
                // This a read with higher priority than highest
                // priority write. dequeue from lock queue and make it
                // ready to be scheduled.
                unblock(lock, item);

                // Move to prev item;
                item = prev;
            }
        }

        // State 2 - Select highest priority process. If waiting
        // READs/WRITEs have same priority then select READ. 
        if (lptr->lnr == 0 && lptr->lnw == 0) {

// XXX must make this select more than 1 read if more than 1 is available

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
    resched();
    return(OK);
}


LOCAL void unblock(int lock, int item) {

    register struct lentry *lptr;

    // Get the pointer to the lock entry
    lptr = &locks[lock];

    // Increment the counter for this type of lock
    if (q[item].qtype == WRITE)
        lptr->lnw++;
    else
        lptr->lnr++;

    kprintf("unblock: READERS %d,\tWRITERS %d\n", lptr->lnr, lptr->lnw);

    // Remove the item from the lock queue and make it 
    // ready to be scheduled.
    dequeue(item);
    ready(item, RESCHNO);
}
    
