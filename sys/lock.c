
#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include <lock.h>
#include <pinh.h>

/*
 * Acquisition of lock for read/write
 *
 * int ldes1    - 
 * int type     -
 * int priority - any positive priority value. larger -> higher priority 
 *
 *
 * General statements about algorithm from PA description:
 *     1) The lock is free. No process owns lock. 
 *          - The requesting process gets the lock and sets the type 
 *            of locking to READ or WRITE.
 *    
 *     2) Lock is not free.
 *          - lock type=WRITE, requesting process must wait 
 *          - lock type=READ, requesting process is granted access if
 *             priority higher than highest priority waiting write.
 *
 *     If a process has to wait then insert it in the lock's wait list
 *     according the the wait priority. Control is returned only when
 *     the calling process is able to acquire the lock. 
 *    
 *     Note: wait priority is different than process priority
 *
 *
 *     No readers should be kept waiting unless:
 *      - a writer has already obtained the lock
 *      - there is a higher priority writer already waiting for the lock
 *
 *     If all waiting processes have equal priority, then readers
 *     should be given preference. In order to prevent starvation of
 *     the writers, allow  the read lock to be acquired a maximum of 3
 *     times by reader processes of the same priority and then let the
 *     writer acquire the lock
 *
 *     If a reader is chosen to have a lock, then all other readers
 *     with priority less than highest-priority waiting writer should
 *     be admitted. 
 */
int lock(int ldes1, int type, int priority) {

    STATWORD ps;    
    struct  lentry  *lptr;
    struct  pentry  *pptr;
    int lock = LOCK_INDEX(ldes1);
    int wait = 0;
    int item;


    // Disable interrupts as this operation needs to be atomic
    disable(ps);

    // Make sure the lock is valid.
    if (isbadlock(lock) || (lptr= &locks[lock])->lstate==LFREE) {
        restore(ps);
        return(SYSERR);
    }

    // Verify the lock requested matches the current version of the
    // lock. Otherwise it is from a previous version of the lock and 
    // SYSERR should be returned
    if (lptr->lversion != LOCK_VERSION(ldes1)) {
        restore(ps);
        return(SYSERR);
    }

    // What is the current state of the resource?
    //
    //      Available                   Request Type
    //      -----------------          ----------
    //  1 -  Yes  (nr == 0, nw == 0)     READ/WRITE
    //  2 -  No   (nr == 0, nw == 1)     READ/WRITE
    //  3 -  No   (nr != 0, nw == 0)       WRITE
    //  4 -  Maybe(nr != 0, nw == 0)       READ

    // State 1 - Lock is available. No need to wait.
    if (lptr->lnr == 0 && lptr->lnw == 0)
        wait = 0;

    // State 2 - Write already has lock. Add to queue and wait. 
    if (lptr->lnr == 0 && lptr->lnw == 1)
        wait = 1;

    // State 3 - Read already has lock and writer requesting lock. Must wait
    if (lptr->lnr != 0 && lptr->lnw == 0 && type == WRITE)
        wait = 1;


    // State 4 - Read already has lock. We may be able to grant the
    //           new process access if:
    //
    //              - There is not a higher priority writer waiting
    //              - A reader has not been given access over an
    //                equal priority writer 3 times in a row (starvation).
    //
    // Go through all items in the lock q. 
    if (lptr->lnr != 0 && lptr->lnw == 0 && type == READ) {
        // Since priority goes from head (lower priority) to
        // tail (higher priority) we will iterate backwards until
        // we hit our priority. If we encounter a waiting WRITE, then
        // there is a higher priority waiting write and we must wait.
        item = q[lptr->lqtail].qprev;
        while (priority < q[item].qkey) { 
            if (q[item].qtype == WRITE)
                wait = 1;         // higher priority write => must wait
            item = q[item].qprev; // Move to prev item;
        }
    }


    // Did we determine we needed to wait?
    if (wait) {
        pptr = &proctab[currpid];
        pptr->pstate = PRLOCK;
        pptr->plock = ldes1;
        insert(currpid, lptr->lqhead, priority);
        q[currpid].qtype   = type;
        q[currpid].qpassed = 0;
        pptr->plockret = OK; // Will change to DELETED if lock gets deleted

        // Update lppriomax for this lock (max priority of all waiting procs)
        update_lppriomax(lock);
        // Update pinh for all procs that hold this lock.
        update_pinh(lock);
        
        resched();           // Context switch happens here
        restore(ps);
        return pptr->plockret;
    }


    // Update the lock's bitvector so that it can know what procs hold it
    set_bit(lptr->lprocs_bsptr, currpid);
    // Update the proc's bitvector so that it can know what locks it holds
    set_bit(proctab[currpid].locks_bsptr, lock);
    // Update pinh for this process 
    update_priority(currpid);


    // Increment the reader/writer count and move on.
    if (type == READ)
        lptr->lnr++;
    else
        lptr->lnw++;
    
#if DEBUG
    kprintf("lock: READERS %d,\tWRITERS %d\n", lptr->lnr, lptr->lnw);
#endif

    // Enable interrupts
    restore(ps);
    return(OK);
}

