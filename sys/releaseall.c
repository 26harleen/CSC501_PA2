

#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>

LOCAL void firstchoice(int ldes, int item);
LOCAL void allowreaders(int ldes);
LOCAL void unblock(int ldes, int item);

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

        kprintf("checking lock %d\n", lock);

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
 ///////if (lptr->lnr != 0 && lptr->lnw == 0) {

 ///////    
 ///////    allowreaders(lock);

 ///////  //// Since priority goes from head (lower priority) to
 ///////  //// tail (higher priority) we will iterate backwards until
 ///////  //// we find a waiting WRITE. All READs we encounter before
 ///////  //// then are READs with higher priority than the highest
 ///////  //// priority WRITE. 
 ///////  //item = q[lptr->lqtail].qprev;
 ///////  //while (item != lptr->lqhead) { 
 ///////  //    if (q[item].qtype == WRITE)
 ///////  //        break; 

 ///////  //    // Save off prev item (needed because unblock() will 
 ///////  //    // dequeue item from the list.
 ///////  //    prev = q[item].qprev;
 ///////  //    
 ///////  //    // This a read with higher priority than highest
 ///////  //    // priority write. dequeue from lock queue and make it
 ///////  //    // ready to be scheduled.
 ///////  //    unblock(lock, item);

 ///////  //    // Move to prev item;
 ///////  //    item = prev;
 ///////  //}
 ///////}

 ///////// State 2 - Select highest priority process. If waiting
 ///////// READs/WRITEs have same priority then select READ. 
 ///////if (lptr->lnr == 0 && lptr->lnw == 0) {

// XXX must make this select more than 1 read if more than 1 is available
//
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
            while (q[item].qkey == q[q[item].qprev].qkey) { 

                // If there is a write that has been passed by
                // too many times then keep track of it.
                if (q[item].qtype == WRITE && q[item].qpassed == 3)
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

        ////// Are all of the equal priority items reads? If so,
        ////// choose the first one and continue.
        ////if (!firstwrite) {
        ////    firstchoice(lock, firstread);
        ////    continue;
        ////}

        ////// Are all of the equal priority items writes? If so,
        ////// choose the first one and continue.
        ////if (!firstread) {
        ////    firstchoice(lock, firstwrite);
        ////    continue;
        ////}

        ////// Ok, we have at least one read and one write at the
        ////// highest priority. If this write has been sidelined 3 times
        ////// then choose the write. Else choose the read.
        ////// XXX may need to check all writes
        ////if (q[firstwrite].qpassed >= 3) {
        ////    firstchoice(lock, firstwrite);
        ////} else {
        ////    firstchoice(lock, firstread);
        ////}

////////////// If there are no waiting procs then nothing to do
////////////if (isempty(lptr->lqhead))
////////////    continue;

////////////// Choose highest priority waiting proc.
////////////item = q[lptr->lqtail].qprev;

////////////// Are there multiple with the same priority?
////////////if (q[item].qkey == q[q[item].qprev].qkey) {

////////////    // Ok, We have multiple items in the queue at the highest
////////////    // priority. Find the first read (if there is one), and
////////////    // the first write (if there is one).
////////////    while (q[item].qkey == q[q[item].qprev].qkey) { 
////////////        if (q[item].qtype == WRITE)
////////////            firstwrite = firstwrite ? firstwrite : item;
////////////        else
////////////            firstread = firstread ? firstread : item;
////////////        item = q[item].qprev;
////////////    }

////////////    // Are all of the equal priority items writes? If so,
////////////    // choose the first one and continue.
////////////    if (!firstread) {
////////////        unblock(lock, firstwrite);
////////////        continue;
////////////    }


////////////    // If we have a write then analyze to see if it has
////////////    // waited 3 times. 
////////////    if (firstwrite) {

////////////        // We have at least one read and one write at the 
////////////        // highest priority. If this write has been sidelined 
////////////        // 3 times then choose the write. Else bump the count
////////////        // and we will let the reads go through
////////////        // XXX may need to check all writes
////////////        if (q[firstwrite].qpassed == 3) {
////////////            unblock(lock, firstwrite);
////////////            continue;
////////////        } else {
////////////            q[firstwrite].qpassed++;
////////////        }

////////////    }
////////////}


////////////allowreaders(lock);


            // Are all of the equal priority items reads? If so,
            // choose the first one and continue.
          //if (!firstwrite) {
          //    unblock(lock, firstread);
          //    continue;
          //}

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
    int item, prev, lock;

    // Get the pointer to the lock entry
    lock = LOCK_INDEX(ldes);
    lptr = &locks[lock];

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

    // Update the reader/writer counters and possibly the
    // qpassed var for any waiting writers.
    update_counters(lock, q[item].qtype, q[item].qkey);

    kprintf("unblock: READERS %d,\tWRITERS %d\n", lptr->lnr, lptr->lnw);

    // Remove the item from the lock queue and make it 
    // ready to be scheduled.
    dequeue(item);
    ready(item, RESCHNO);
}

    
