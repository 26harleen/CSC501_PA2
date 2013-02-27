/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

unsigned long currSP;   /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:   Upon entry, currpid gives current process id.
 *      Proctab[currpid].pstate gives correct NEXT state for
 *          current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
    register struct pentry  *optr;  /* pointer to old process entry */
    register struct pentry  *nptr;  /* pointer to new process entry */
    int item, max, maxitem;
    max     = 0;
    maxitem = 0; // Null Process

    // 
    optr = &proctab[currpid];
    
    // If nothing in ready queue then move on.
    if ((optr->pstate == PRCURR) && isempty(rdyhead))
        return OK;

    // Find the process with the greatest priority
    item = q[rdytail].qprev;
    while (item != rdyhead) {
        if (EFFECTIVE_PRIO(&proctab[item]) > max) {
            max     = EFFECTIVE_PRIO(&proctab[item]);
            maxitem = item;
        }
        item = q[item].qprev;
    }

    /* no switch needed if current process priority higher than next*/
    if ((optr->pstate == PRCURR) && (max < EFFECTIVE_PRIO(optr))) {
        return(OK);
    }
    
    /* force context switch */

    if (optr->pstate == PRCURR) {
        optr->pstate = PRREADY;
        insert(currpid,rdyhead,optr->pprio);
    }

    //kprintf("Choosing pid %d with effective priority %d\n", maxitem, max);

    currpid = maxitem;           // Update the currpid global 
    dequeue(maxitem);            // Remove the process from the ready list  
    nptr = &proctab[currpid];    // Get a pointer to the PCB(pentry) for the proc
    nptr->pstate = PRCURR;       // mark it currently running
#ifdef  RTCLOCK
    preempt = QUANTUM;      /* reset preemption counter */
#endif
    
    ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
    
    /* The OLD process returns here when resumed. */
    return OK;
}
