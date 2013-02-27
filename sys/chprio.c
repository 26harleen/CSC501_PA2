/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
#include <pinh.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
    STATWORD ps;    
    struct  pentry  *pptr;

    disable(ps);
    if (isbadpid(pid) || newprio<=0 ||
        (pptr = &proctab[pid])->pstate == PRFREE) {
        restore(ps);
        return(SYSERR);
    }
    pptr->pprio = newprio;
    if(pptr -> pstate == PRREADY) {
        dequeue(pid);
        insert(pid, rdyhead, pptr -> pprio);
    }

    // Update the processes priority inheritance
    update_priority(pid);

    // If the process is waiting for a lock then update the lock
    if (pptr->pstate == PRLOCK) {
        // Update lppriomax for the lock (max priority of all waiting procs)
        update_lppriomax(LOCK_INDEX(pptr->plock));
        // Update pinh for all procs that hold this lock.
        update_pinh(LOCK_INDEX(pptr->plock));
    }

    restore(ps);
    return(newprio);
}
