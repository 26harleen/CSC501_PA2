
#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include <lock.h>

/*
 * Acquisition of lock for read/write
 *
 * int ldes1    - 
 * int type     -
 * int priority - any positive priority value. larger -> higher priority 
 */
int lock(int ldes1, int type, int priority) {

    STATWORD ps;    
    struct  lentry  *lptr;
    struct  pentry  *pptr;
    int lock = LOCK_INDEX(ldes1);
    int wait = 0;
    int item;

    // 1) The lock is free. No process owns lock. 
    //      - The requesting process gets the lock and sets the type 
    //        of locking to READ or WRITE.
    //
    // 2) Lock is not free.
    //      - lock type=WRITE, requesting process must wait 
    //      - lock type=READ, requesting process is granted access if
    //         priority higher than highest priority waiting write.

    // If a process has to wait then insert it in the lock's wait list
    // according the the wait priority. Control is returned only when
    // the calling process is able to acquire the lock. 
    //
    // Note: wait priority is different than process priority


    // No readers should be kept waiting unless:
    //  - a writer has already obtained the lock
    //  - there is a higher priority writer already waiting for the lock

    // If all waiting processes have equal priority, then readers
    // should be given preference. In order to prevent starvation of
    // the writers, allow  the read lock to be acquired a maximum of 3
    // times by reader processes of the same priority and then let the
    // writer acquire the lock

    // If a reader is chosen to have a lock, then all other readers
    // with priority less than highest-priority waiting writer should
    // be admitted. 

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

        // If there are any equal priority writes bump qpassed 
        // count and check to see if they have been passed up too
        // many times already. If so then wait
        while (priority == q[item].qkey && !wait) { 
            if (q[item].qtype == WRITE && q[item].qpassed == 3)
                wait = 1;
            item = q[item].qprev; // Move to prev item;
        }
        //XXX need to handle the starvation thing. maybe?
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
        resched();           // Context switch happens here
        restore(ps);
        return pptr->plockret;
    }


    // Update the reader/writer counters and possible the
    // qpassed var for any waiting writers.
    update_counters(ldes1, type, priority);
    
    kprintf("lock: READERS %d,\tWRITERS %d\n", lptr->lnr, lptr->lnw);

    // Enable interrupts
    restore(ps);
    return(OK);
}


















////// What is the current state of the lock?
//////
//////      Available               Type
//////      -----------------     ----------
//////  1 -  Yes  (lcnt == 1)        N/A
//////  2 -  No   (lcnt <  1)       Write
//////  3 -  Maybe(lcnt <  1)       Read

////// State 1 - Lock is available. Decrease lcnt and set the type. 
////if (lptr->lcnt == 1) {
////    lptr->lcnt--;
////    lptr->ltype = type;
////    wait = 0;
////// make sure next thing done here is return
//////

////// State 2 - Write already has lock. Add to queue and wait. 
////} else if (lptr->lcnt < 1 && lptr->ltype == WRITE) {

////    wait = 1;


////// State 3 - Read already has lock. We may be able to grant the
//////           new process access if:
//////
//////              - There is not a higher priority writer waiting
//////              - A reader has not been given access over an
//////                equal priority writer 3 times in a row (starvation).
//////
////// Go through all items in the lock q. 
////} else if (lptr->lcnt < 1 && lptr->ltype == READ) {
////    item = q[lptr->lqhead].qnext;
////    //while (item != lptr->lqtail) {
////    while (priority < q[item].qkey) {
////        if (q[item].qtype == WRITE) {
////            wait = 1; // There is a higher priority write waiting - must wait
////        }
////        item = q[item].qnext;
////    }

////} else {

////    // Should never get here
////    restore(ps);
////    return(SYSERR);
////}

////if (wait) {
////    lptr->lcnt--;
////    (pptr = &proctab[currpid])->pstate = PRLOCK;
////    pptr->plock = lock;
////    insert(currpid, lptr->lqhead, priority);
////    pptr->plockret = OK; // Will change to DELETED if lock gets deleted
////    resched();           // Context switch happens here
////    restore(ps);
////    return pptr->plockret;
////}

////// Enable interrupts
////restore(ps);
////return(OK);



////STATWORD ps;    
////struct  sentry  *sptr;
////struct  pentry  *pptr;

////disable(ps);
////if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
////    restore(ps);
////    return(SYSERR);
////}
////
////if (--(sptr->semcnt) < 0) {
////    (pptr = &proctab[currpid])->pstate = PRWAIT;
////    pptr->psem = sem;
////    enqueue(currpid,sptr->sqtail);
////    pptr->pwaitret = OK;
////    resched();
////    restore(ps);
////    return pptr->pwaitret;
////}
////restore(ps);
////return(OK);


void update_counters(int ldes1, int type, int priority) {
    int item;
    struct  lentry  *lptr;
    int lock = LOCK_INDEX(ldes1);
    lptr     = &locks[lock];


    // If this is a read find any equal priority writes 
    // and bump their passed count.
    if (type == READ) {

        // Find all equal priority writes and bump count 
        item = q[lptr->lqtail].qprev;
        while (item != lptr->lqhead) { 
            if (priority == q[item].qkey && q[item].qtype == WRITE)
                q[item].qpassed++;
            item = q[item].qprev; // Move to prev item;
        }
    }

    // Increment the reader/writer count and move on.
    if (type == READ)
        lptr->lnr++;
    else
        lptr->lnw++;

}
