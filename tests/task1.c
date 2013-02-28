#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>
#include <sem.h>

#define DEFAULT_LOCK_PRIO 20


// Process to run at pprio 15 that is always
// ready to run. 
void looper() {
    while(1)
        ;
}

////////////////////////////
// LOCK ACCESS
////////////////////////////
void reader(char *msg, int lck, int lprio, int seconds) {
    int rc;
    kprintf ("  %s: to acquire lock\n", msg);
    rc = lock(lck, READ, lprio);
    if (rc == SYSERR) {
        kprintf ("  %s: lock returned SYSERR\n", msg);
        return;
    }
    if (rc == DELETED) {
        kprintf ("  %s: lock was DELETED\n", msg);
        return;
    }
    kprintf ("  %s: acquired lock, sleep %ds\n", msg, seconds);
    sleep(seconds);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

void writer(char *msg, int lck, int lprio, int seconds) {
    int rc;
    kprintf ("  %s: to acquire lock\n", msg);
    rc = lock(lck, WRITE, lprio);
    if (rc == SYSERR) {
        kprintf ("  %s: lock returned SYSERR\n", msg);
        return;
    }
    if (rc == DELETED) {
        kprintf ("  %s: lock was DELETED\n", msg);
        return;
    }
    kprintf ("  %s: acquired lock, sleep %ds\n", msg, seconds);
    sleep(seconds);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

////////////////////////////
// SEMAPHORE ACCESS
////////////////////////////
void sreader(char *msg, int sem, int seconds) {
    int rc;
    kprintf ("  %s: to acquire sem\n", msg);
    rc = wait(sem);
    if (rc == SYSERR) {
        kprintf ("  %s: sem returned SYSERR\n", msg);
        return;
    }
    kprintf ("  %s: acquired sem, sleep %ds\n", msg, seconds);
    sleep(seconds);
    kprintf ("  %s: to release sem\n", msg);
    signal(sem);
}

void swriter(char *msg, int sem, int seconds) {
    sreader(msg, sem, seconds);
}


// Priority Inheritance chprio test
void pinh_chprio_test() {
    int lck;
    int rd1;
    int wr1, wr2;
    int lp;

    kprintf("\nBasic Priority chprio Test\n"
    "Once looper process is started readers/writers will be\n"
    "starved until chprio() is used to change prio of reader C\n"
    "(currently waiting on lock) to 30\n"
    " lock acquisition is:\n" 
    "writer A\n"
    "writer B\n"
    "reader C\n");
    lck = lcreate();

    // Create a process that is always ready to run at priority 15
    lp = create(looper, 2000, 15, "looper", 0, NULL); 

    wr1 = create(writer, 2000, 10, "writer", 4, "writer A p10 l20", lck, 20, 5);
    wr2 = create(writer, 2000, 11, "writer", 4, "writer B p11 l30", lck, 30, 2);
    rd1 = create(reader, 2000, 12, "reader", 4, "reader C p12 l20", lck, 20, 1);

    kprintf("-start writer A (pprio 10, lprio 20), then sleep 1s.\n");
    resume(wr1);
    sleep(1);

    kprintf("-start writer B (pprio 11, lprio 30), then sleep 1s.\n");
    resume(wr2);
    sleep(1);

    kprintf("-start reader C (pprio 12, lprio 20), then sleep 1s.\n");
    resume(rd1);
    sleep(1);

    // This will guarantee that writer B will never get chosen unless
    // we boost its priority. 
    kprintf("-start looper process (pprio 15), then sleep 8s.\n");
    resume(lp);
    sleep(8);

    //chprio 
    kprintf("-chprio of reader C to 30 --> All readers/writers will have effective priority 30.\n");
    chprio(rd1, 30);
    
    sleep (10);
    kprintf ("Test finished, verify readers/writers were hung until chprio!\n");
}

// Priority Inheritance kill test
void pinh_kill_test() {
    int lck;
    int rd1;
    int wr1, wr2;
    int lp;

    kprintf("\nPriority Inheritance kill Test\n"
    " lock acquisition is:\n" 
    "writer A\n"
    "writer B\n"
    "reader C\n");
    lck = lcreate();

    // Create a process that is always ready to run at priority 15
    lp = create(looper, 2000, 15, "looper", 0, NULL); 

    wr1 = create(writer, 2000, 5, "writer", 4, "writer A p5 l20", lck, 20, 10);
    wr2 = create(writer, 2000, 10, "writer", 4, "writer B p10 l30", lck, 30, 2);
    rd1 = create(reader, 2000, 30, "reader", 4, "reader C p30 l20", lck, 20, 1);

    kprintf("-start writer A (pprio 10, lprio 20), then sleep 1s.\n");
    resume(wr1);
    sleep(1);

    kprintf("-start writer B (pprio 11, lprio 30), then sleep 1s.\n");
    resume(wr2);
    sleep(1);

    kprintf("-start reader C (pprio 12, lprio 20), then sleep 1s.\n");
    resume(rd1);
    sleep(1);

    kprintf("-start looper process (pprio 15), then sleep 1s.\n");
    resume(lp);
    sleep(1);

    kprintf("-kill reader C (pprio 30) --> All other readers/writers will starve.\n");
    kill(rd1);
    
    sleep (10);
    kprintf ("Test finished, check order of acquisition!\n");
}

// Basic Priority Inheritance Test (locks)
void basic_pinh_locks_test() {
    int lck;
    int rd1;
    int wr1, wr2;
    int lp;

    kprintf("\nBasic Priority Inheritance Test (locks)\n"
    " lock acquisition is:\n" 
    "writer A\n"
    "writer B\n"
    "reader C\n");
    lck = lcreate();

    // Create a process that is always ready to run at priority 15
    lp = create(looper, 2000, 15, "looper", 0, NULL); 

    wr1 = create(writer, 2000, 20, "writer", 4, "writer A p20 l20", lck, 20, 5);
    wr2 = create(writer, 2000, 10, "writer", 4, "writer B p10 l30", lck, 30, 2);
    rd1 = create(reader, 2000, 40, "reader", 4, "reader C p40 l20", lck, 20, 1);

    kprintf("-start writer A (pprio 20, lprio 20), then sleep 1s.\n");
    resume(wr1);
    sleep(1);

    kprintf("-start writer B (pprio 10, lprio 30), then sleep 1s.\n");
    resume(wr2);
    sleep(1);

    // This will guarantee that writer B will never get chosen unless
    // we boost its priority. 
    kprintf("-start looper process (pprio 15), then sleep 1s.\n");
    resume(lp);
    sleep(1);
    
    kprintf("-start reader C (pprio 40, lprio 20), then sleep 1s.\n");
    resume(rd1);
    sleep(1);
    
  //kprintf("-kill reader B, then sleep 1s\n");
  //kill (rd2);
  //sleep (1);
  //

  //kprintf("-kill reader A, then sleep 1s\n");
  //kill (rd1);
  //sleep(1);
    
    sleep (10);
    kprintf ("Test finished, check order of acquisition!\n");
}

// Basic Priority Inheritance Test (XINU semaphores)
void basic_pinh_sem_test() {
    int sem;
    int rd1;
    int wr1, wr2;
    int lp;

    kprintf("\nBasic Priority Inheritance Test (XINU sem)\n"
    "Verify Writer B and Reader C never receives semaphore\n"
    "because Writer B (pprio 10) has priority less than\n"
    "looper process (pprio 15)\n"
    " lock acquisition is:\n" 
    "writer A\n");

    // Create a semaphore that only allows one accessor at a time.
    sem = screate(1);

    // Create a process that is always ready to run at priority 15
    lp = create(looper, 2000, 15, "looper", 0, NULL); 


    wr1 = create(swriter, 2000, 20, "writer", 3, "writer A p20", sem, 5);

    wr1 = create(swriter, 2000, 20, "writer", 3, "writer A p20", sem, 5);
    wr2 = create(swriter, 2000, 10, "writer", 3, "writer B p10", sem, 2);
    rd1 = create(sreader, 2000, 40, "reader", 3, "reader C p40", sem, 1);


    kprintf("-start writer A (pprio 20), then sleep 1s.\n");
    resume(wr1);
    sleep(1);

    kprintf("-start writer B (pprio 10), then sleep 1s.\n");
    resume(wr2);
    sleep(1);

    // This will guarantee that writer B will never get chosen unless
    // we boost its priority. We don't do that for sems => lock 
    kprintf("-start looper process (pprio 15), then sleep 1s.\n");
    resume(lp);
    sleep(1);
    
    kprintf("-start reader C (pprio 40), then sleep 1s.\n");
    resume(rd1);
    sleep(1);
    
    sleep (10);
    kprintf ("Test finished, verify writer B and reader C never recieved sem!\n");
}

int main() {
    int i, s;
    int count = 0;
    char buf[8];


    kprintf("Options are:\n");
    kprintf("\t1 - Priority Inheritance chprio test\n");
    kprintf("\t2 - Priority Inheritance kill test\n");
    kprintf("\t3 - Basic Priority Inheritance Test (locks)\n");
    kprintf("\t4 - Basic Priority Inheritance Test (XINU semaphores)\n");
    kprintf("\nPlease Input:\n");
    while ((i = read(CONSOLE, buf, sizeof(buf))) <1);
    buf[i] = 0;
    s = atoi(buf);
    switch (s)
    {
    case 1:
        // Priority Inheritance chprio test
        pinh_chprio_test();
        break;
    
    case 2:
        // Priority Inheritance kill test
        pinh_kill_test();
        break;
        
    case 3:
        // Basic Priority Inheritance Test (locks)
        basic_pinh_locks_test();
        break;

    case 4:
        // Basic Priority Inheritance Test (XINU semaphores)
        basic_pinh_sem_test();
        break;

    }
}




