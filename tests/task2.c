#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

static int count = 0;
static int complete = 0;

void producer(char *name, int lck_count, int lck_complete) {
    int brk = 0;
    while (1) {

        // Get a WRITE lock for "count"
        lock(lck_count, WRITE, DEFAULT_LOCK_PRIO);

        if (count < 10) {
            sleep(rand()%2);
            count++;
            kprintf("%s produces -> %d\n", name, count);
        }


        // Get a WRITE lock for "complete"
        // Note: I would release count WRITE lock and acquire a READ 
        //       lock instead but we don't want two procs printing out
        //       "produces -> 10" so we just hold on to the WRITE lock
        lock(lck_complete, WRITE, DEFAULT_LOCK_PRIO);

        if (complete) {
            brk = 1; // Break out of loop after releasing locks
        } else {
            if (count >= 10) {
                complete = 1;
                kprintf("* %s finishes -> %d\n", name, count);
                brk = 1; // Break out of loop after releasing locks
            }
        }
        releaseall(2, lck_complete, lck_count);

        if (brk)
            break;
    }
    kprintf("%s completes\n", name);
}

void consumer(char *name, int lck_count, int lck_complete) {
    int brk = 0;


    while (1) {
        
        lock(lck_count, WRITE, DEFAULT_LOCK_PRIO);
        lock(lck_complete, READ, DEFAULT_LOCK_PRIO);
        

        if (complete) {
            brk = 1; // Break out of loop after releasing locks
        } else {
            if (count > 0 && !complete) {
                count--;
                kprintf("%s consumes -> %d\n", name, count);
            }
        }
        
        // Release locks
        releaseall(2, lck_complete, lck_count);
        
        // Break out of loop if necessary.. otherwise sleep. 
        if (brk)
            break;
        else
            sleep(rand()%3);
    }
    kprintf("%s completes\n", name);
}

void test_sync() {
    int lck_count = lcreate();
    int lck_complete = lcreate();
    int p1 = create(producer,  2000, 15, "producer", 3, "p1", lck_count, lck_complete);
    int p2 = create(producer,  2000, 15, "producer", 3, "p2", lck_count, lck_complete);
    int p3 = create(producer,  2000, 15, "producer", 3, "p3", lck_count, lck_complete);

    int c1 = create(consumer, 2000, 15, "consumer", 3, "c1", lck_count, lck_complete);
    int c2 = create(consumer, 2000, 15, "consumer", 3, "c2", lck_count, lck_complete);


    resume(p1);
    resume(p2);
    resume(p3);
    resume(c1);
    resume(c2);
}

int main( )
{
        srand(12345);
        test_sync();
}
