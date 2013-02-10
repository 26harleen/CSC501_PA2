/* q.h - firstid, firstkey, isempty, lastkey, nonempty */

#ifndef _QUEUE_H_
#define _QUEUE_H_

/* q structure declarations, constants, and inline procedures       */

#ifndef NQENT
// Need:
//      1 entry for each process
//      2 for each lock      (1 head, 1 tail)
//      2 for each semaphore (1 head, 1 tail)
//      2 for ready queue    (1 head, 1 tail)
//      2 for sleep queue    (1 head, 1 tail) 
#define NQENT NPROC + 2*NLOCK + 2*NSEM + 2 + 2
#endif

struct  qent    {       /* one for each process plus two for    */
                /* each list                */
    int qkey;       /* key on which the queue is ordered    */
    int qnext;      /* pointer to next process or tail  */
    int qprev;      /* pointer to previous process or head  */
    int qtype;      // Type: READ/WRITE - used only for locks
};

extern  struct  qent q[];
extern  int nextqueue;

/* inline list manipulation procedures */

#define isempty(list)   (q[(list)].qnext >= NPROC)
#define nonempty(list)  (q[(list)].qnext < NPROC)
#define firstkey(list)  (q[q[(list)].qnext].qkey)
#define lastkey(tail)   (q[q[(tail)].qprev].qkey)
#define firstid(list)   (q[(list)].qnext)

/* gpq constants */

#define QF_WAIT     0   /* use semaphores to mutex      */
#define QF_NOWAIT   1   /* use disable/restore to mutex     */

/* ANSI compliant function prototypes */

int enqueue(int item, int tail);
int dequeue(int item);
int printq(int head);
int newqueue();
int insertd(int pid, int head, int key);
int insert(int proc, int head, int key);
int getfirst(int head);
int getlast(int tail);

#endif
