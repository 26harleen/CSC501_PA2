
#ifndef _LOCK_H_
#define _LOCK_H_


#define DELETED XXX
#define WRITE   XXX
#define READ    XXX
#define NLOCKS  50

#define isbadlock(l) (l<0 || l>=NLOCKS)

// This is a lock table entry
struct lentry {
    char lstate;    // The state LFREE or LUSED
    int  lcnt;      // Count for this lock
    int  lnr;       // Count of readers
    int  lnw;       // Count of writers
    int  ltype;     // The current access type READ/WRITE given to processes
    int  lqhead;    // q index of head of list
    int  lqtail;    // q index of tail of list
};

extern struct lentry locks[];
extern int nextlock;

void linit();
int  lcreate();
int  ldelete(int lockdescriptor);
int  lock(int ldes1, int type, int priority);
int  releaseall(int numlocks, int ldes1, ...);

#endif
