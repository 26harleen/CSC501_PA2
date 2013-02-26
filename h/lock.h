#include <bitvector.h>


#ifndef _LOCK_H_
#define _LOCK_H_


#define WRITE   111
#define READ    222
#define NLOCKS  50
#define LOCK_INDEX_BITS 6

#define	LFREE	'\03' // This lock is free
#define	LUSED	'\04' // This lock is used

#define isbadlock(l) (l<0 || l>=NLOCKS)


// The lock descriptor will be composed of 6 bits indicating
// the index into the locks array and the rest of the bits will
// be version bits 
#define LOCK_INDEX(l) (l & ((1 << LOCK_INDEX_BITS) - 1))
#define LOCK_VERSION(l) (l >> LOCK_INDEX_BITS)

// This is a lock table entry
struct lentry {
    char lstate;    // The state LFREE or LUSED
    int  lnr;       // Count of readers
    int  lnw;       // Count of writers
    int  ltype;     // The current access type READ/WRITE given to processes
    int  lqhead;    // q index of head of list
    int  lqtail;    // q index of tail of list
    unsigned int lversion;  // the version of the lock (starts at 0)
    bs_ptr lbsptr;
};

extern struct lentry locks[];
extern int nextlock;

void linit();
int  lcreate();
int  ldelete(int lockdescriptor);
int  lock(int ldes1, int type, int priority);
int  releaseall(int numlocks, int ldes1, ...);
void update_counters(int ldes1, int type, int priority);

#endif
