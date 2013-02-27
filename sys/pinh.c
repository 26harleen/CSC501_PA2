#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

// Update lppriomax for this lock by iterating
// over all processes currently WAITING in the locks
// wait queue and setting it to the maximum;
//
// Note: Don't need to consider pinh here because waiting
//       processes won't have inherited priority
void update_lppriomax(int lindex) {
    int item, max;
    register struct lentry *lptr;
    lptr = &locks[lindex];
    max  = 0;

    item = q[lptr->lqhead].qnext;
    while (item != lptr->lqtail) { 
        if (proctab[item].pprio > max)
            max = proctab[item].pprio;
        item = q[item].qnext; // Move to next item;
    }
    lptr->lppriomax = max;
}


// Update the inherited priority(pinh) for all processes 
// that currently HOLD the specified lock. 
void update_pinh(int lindex) {
    int i;
    register struct lentry *lptr;
    lptr = &locks[lindex];

    for(i=0; i<NPROC; i++) {
        if (get_bit(lptr->lprocs_bsptr, i))
            if (lptr->lppriomax > proctab[i].pprio)
                proctab[i].pinh = lptr->lppriomax;
    }
}


// On obtaining or releasing a lock, the priority of the process 
// has to be reset to the maximum priority of all the processes 
// in the wait queues of all the locks held by the process.
void update_priority(int pid) {
    struct pentry *pptr;
    int i, max;
    pptr = &proctab[pid];

    max = 0;

    for(i=0; i<NLOCKS; i++) {
        if (get_bit(pptr->locks_bsptr, i))
            if (locks[i].lppriomax > max)
                max = locks[i].lppriomax;
    }

    // If the process has higher priority than any of the 
    // procs waiting for lock then just use original priority
    // and leave pinh = 0, else set pinh.
    if (pptr->pprio > max)
        pptr->pinh = 0;
    else
        pptr->pinh = max;

}
