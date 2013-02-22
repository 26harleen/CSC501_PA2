
#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include <lock.h>

/*
 * Deletes the lock identified by the descriptor lockdescriptor
 */
int ldelete(int lockdescriptor) {


    // You must implement your lock system such that waiting on a lock 
    // will return a new constant DELETED instead of OK when returning 
    // due to a deleted lock. This will indicate to the user that the 
    // lock was deleted and not unlocked. As before, any calls to lock() 
    // after the lock is deleted should return SYSERR.



    // XXX 
    // Must find a way to make locks unique such that newly created
    // locks that have the same ID as old locks that were deleted
    // don't affect old processes that may have been waiting on the
    // old lock that was deleted. 

    STATWORD ps;    
    int pid;
    int lock = lockdescriptor;
    struct lentry *lptr;

    disable(ps);
    if (isbadlock(lock) || locks[lock].lstate==LFREE) {
        restore(ps);
        return(SYSERR);
    }
    lptr = &locks[lock];
    lptr->lstate = LFREE;
    if (nonempty(lptr->lqhead)) {
        while((pid=getfirst(lptr->lqhead)) != EMPTY) {
            proctab[pid].plockret = DELETED;
            ready(pid,RESCHNO);             
        }
        resched();
    }
    restore(ps);
    return(OK);
}
