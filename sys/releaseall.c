#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>
#include <pinh.h>

LOCAL void firstchoice(int ldes, int item);
LOCAL void allowreaders(int ldes);
LOCAL void unblock(int ldes, int item);
LOCAL void bump_qpassed(int ldes1, int priority);

/*
 * Simultaneously release one or more locks. Takes a variable number
 * of arguments. The number of locks to be released is passed in as
 * the first argument. 
 *
 * Note: the sytax of the first part of the function below allows
 *       the variable number of arguments.
 */
int releaseall(numlocks, ldes1)
    int numlocks;
    int ldes1;
{
    STATWORD ps;    
    register struct lentry *lptr;
    int i, item, ldes, lock;
    int firstwrite;
    int bestwrite;
    int firstread;
    int priority;

    // Disable interrupts - needs to be atomic
    disable(ps);

    // Iterate over all of the specified resources and release
    // one instance of them.
    for (i=0; i<numlocks; i++) {

        ldes = *((&ldes1) + i);
        lock = LOCK_INDEX(ldes);
        lptr = &locks[lock];
        firstwrite = 0;
        bestwrite  = 0;
        firstread  = 0;
        priority   = 0;

        // Verify lock index is valid and lock is not somehow
        // free 
        if (isbadlock(lock) || lptr->lstate==LFREE) {
            restore(ps);
            return(SYSERR);
        }

        // Verify the lock in question matches the current version of the
        // lock. Otherwise it is from a previous version of the lock and 
        // SYSERR should be returned
        if (lptr->lversion != LOCK_VERSION(ldes)) {
            restore(ps);
            return(SYSERR);
        }

#if DEBUG
        kprintf("checking lock %d\n", lock);
#endif

        // Update the count for this resource
        if (lptr->lnw == 1)
            lptr->lnw--;
        else
            lptr->lnr--;

        // Update the lock's bitvector so that it can know what procs hold it
        clr_bit(lptr->lprocs_bsptr, currpid);
        // Update the proc's bitvector so that it can know what locks it holds
        clr_bit(proctab[currpid].locks_bsptr, lock);
        // Update the priority of the process that is releasing the lock
        update_priority(currpid);




        // What we will do here is select either a read or a write
        // as our best candidate for the next lock access and then
        // call firstchoice() with our candidate. There is logic below
        // firstchoice() that makes further qualifications. 

        // If there are no waiting procs then nothing to do
        if (isempty(lptr->lqhead))
            continue;

        // Choose highest priority waiting proc.
        item = q[lptr->lqtail].qprev;

        // Are there multiple with the same priority? If not, no
        // more to do.
        if (q[item].qkey != q[q[item].qprev].qkey) {
            firstchoice(ldes, item);
            continue;
        }

        // Ok, We have multiple items in the queue at the highest
        // priority. Find the first read (if there is one), and
        // the first write (if there is one).
        priority = q[item].qkey;
        while (q[item].qkey == priority) { 

            // If there is a write that has been passed by
            // too many times then keep track of it.
            if (q[item].qtype == WRITE && q[item].qpassed >= 3)
                bestwrite = item;

            // Keep record of the first read and the first
            // write among these equal priority items
            if (q[item].qtype == WRITE)
                firstwrite = firstwrite ? firstwrite : item;
            else
                firstread = firstread ? firstread : item;

            item = q[item].qprev;
        }

        // Was there a write that has been passed by too
        // many times already? If so choose it
        if (bestwrite) {
            firstchoice(ldes, bestwrite);
            continue;
        }

        if (firstread)
            firstchoice(ldes, firstread);
        else
            firstchoice(ldes, firstwrite);
    }

    restore(ps);
    resched();
    return(OK);
}


LOCAL void firstchoice(int ldes, int item) {
    register struct lentry *lptr;
    int lock;

    // Get the pointer to the lock entry
    lock = LOCK_INDEX(ldes);
    lptr = &locks[lock];

    // If this is a read then call allowreaders() 
    // that will unblock any eligible reads
    if (q[item].qtype == READ &&
        lptr->lnw     == 0
    )
        allowreaders(ldes);

    if (q[item].qtype == WRITE && 
        lptr->lnw     == 0     &&
        lptr->lnr     == 0
    )
        unblock(ldes, item);
}

LOCAL void allowreaders(int ldes) {

    register struct lentry *lptr;
    int item, prev, lock, maxwrite, maxread;

    // Get the pointer to the lock entry
    lock = LOCK_INDEX(ldes);
    lptr = &locks[lock];

    maxwrite = 0;
    maxread  = 0;


    // Find the priority of the highest priority write and highest
    // priority read. Since priority goes from head (lower priority)
    // to tail (higher priority) we will iterate backwards.
    item = q[lptr->lqtail].qprev;
    while (item != lptr->lqhead) { 
        if (q[item].qtype == WRITE && !maxwrite)
            maxwrite = q[item].qkey;
        if (q[item].qtype == READ && !maxread)
            maxread = q[item].qkey;

        // Move to prev item;
        item = q[item].qprev;
    }

    // Bump the qpassed count for the first writer that matches
    // the maximum priority reader.
    bump_qpassed(ldes, maxread);


    // Now unblock any reads with priority higher or equal to the
    // highest priority write.
    item = q[lptr->lqtail].qprev;
    while ((q[item].qkey >= maxwrite) && (item != lptr->lqhead)) { 
        if (q[item].qtype == WRITE) {
            item = q[item].qprev;
            continue; 
        }

        // Save off prev item (needed because unblock() will 
        // dequeue item from the list.
        prev = q[item].qprev;
        
        // This a read with higher priority than highest
        // priority write. dequeue from lock queue and make it
        // ready to be scheduled.
        unblock(ldes, item);

        // Move to prev item;
        item = prev;
    }
}

LOCAL void unblock(int ldes, int item) {

    register struct lentry *lptr;
    int lock;

    // Get the pointer to the lock entry
    lock = LOCK_INDEX(ldes);
    lptr = &locks[lock];

    // Update the reader/writer counters
    if (q[item].qtype == READ)
        lptr->lnr++;
    else
        lptr->lnw++;


    // Update the lock's bitvector so that it can know what procs hold it
    set_bit(lptr->lprocs_bsptr, item);
    // Update the proc's bitvector so that it can know what locks it holds
    set_bit(proctab[item].locks_bsptr, lock);
    // Update lppriomax for this lock (max priority of all waiting procs)
    update_lppriomax(lock);
    // Update pinh for all procs that hold this lock.
    update_pinh(lock);


#if DEBUG
    kprintf("unblock: READERS %d,\tWRITERS %d\n", lptr->lnr, lptr->lnw);
#endif

    // Remove the item from the lock queue and make it 
    // ready to be scheduled.
    dequeue(item);
    ready(item, RESCHNO);
}


LOCAL void bump_qpassed(int ldes1, int priority) {
    int item;
    struct  lentry  *lptr;
    int lock = LOCK_INDEX(ldes1);
    lptr     = &locks[lock];


    // Find first equal priority write and bump count 
    item = q[lptr->lqhead].qnext;
    while (item != lptr->lqtail) { 
        if (priority == q[item].qkey && q[item].qtype == WRITE) {
            q[item].qpassed++;
            break;
        }
        item = q[item].qnext; // Move to next item;
    }
}
    
