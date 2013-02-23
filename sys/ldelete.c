
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



    STATWORD ps;    
    int pid;
    int lock = LOCK_INDEX(lockdescriptor);
    struct lentry *lptr;

    disable(ps);
    lptr = &locks[lock];
    if (isbadlock(lock) || lptr->lstate==LFREE) {
        restore(ps);
        return(SYSERR);
    }
    if (lptr->lversion != LOCK_VERSION(lockdescriptor)) {
        restore(ps);
        return(SYSERR);
    }
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
