#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

void reader(char *msg, int lck, int lprio) {
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
    kprintf ("  %s: acquired lock, sleep 3s\n", msg);
    sleep (3);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

void writer(char *msg, int lck, int lprio) {
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
    kprintf ("  %s: acquired lock, sleep 3s\n", msg);
    sleep (3);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

void combo(char *msg, int lck1, int lck2, int lprio1, int lprio2) {
    int rc;

    // Get first lock
    kprintf ("  %s: to acquire read lock %d\n", msg, lck1);
    rc = lock(lck1, READ, lprio1);
    if (rc == SYSERR) {
        kprintf ("  %s: lock returned SYSERR\n", msg);
        return;
    }
    if (rc == DELETED) {
        kprintf ("  %s: lock was DELETED\n", msg);
        return;
    }
    kprintf ("  %s: acquired lock %d, sleep 3s\n", msg, lck1);


    // Get second lock
    kprintf ("  %s: to acquire write lock %d\n", msg, lck2);
    rc = lock(lck2, WRITE, lprio2);
    if (rc == SYSERR) {
        kprintf ("  %s: lock returned SYSERR\n", msg);
        releaseall(1, lck1);
        return;
    }
    if (rc == DELETED) {
        kprintf ("  %s: lock was DELETED\n", msg);
        releaseall(1, lck1);
        return;
    }
    kprintf ("  %s: acquired lock %d, sleep 3s\n", msg, lck2);
    sleep (3);
    kprintf ("  %s: to release locks\n", msg);
    releaseall(2, lck1, lck2);
}

// Starvation Test
void starve_test() {
    int lck;
    int rd1, rd2, rd3, rd4, rd5, rd6;
    int wr1, wr2, wr3, wr4;

    kprintf("\nStarve Test: Make sure writer C20 gets priority before reader H20.\n"
    "Expected order of lock acquisition is:\n" 
    "writer A20,\n"
    "reader B20,\n"
    "writer D30,\n"
    "reader E20,\n"
    "writer F30,\n"
    "reader G20,\n"
    "writer C20,\n"
    "reader H20,\n");
    lck  = lcreate ();
  

    rd1 = create(reader, 2000, 20, "reader", 3, "reader B20", lck, 20);
    rd2 = create(reader, 2000, 20, "reader", 3, "reader E20", lck, 20);
    rd3 = create(reader, 2000, 20, "reader", 3, "reader G20", lck, 20);
    rd4 = create(reader, 2000, 20, "reader", 3, "reader H20", lck, 20);

    wr1 = create(writer, 2000, 20, "writer", 3, "writer A20", lck, 20);
    wr2 = create(writer, 2000, 20, "writer", 3, "writer D30", lck, 30);
    wr3 = create(writer, 2000, 20, "writer", 3, "writer F30", lck, 30);
    wr4 = create(writer, 2000, 20, "writer", 3, "writer C20", lck, 20);

    kprintf("-start writer A20. Sleep 2s\n");
    resume(wr1);
    sleep(2);


    kprintf("-start reader B20 and writer C20. Sleep 3s\n");
    resume(rd1);
    resume(wr4);
    sleep(3);

    kprintf("-start writer D30. Sleep 2s\n");
    resume(wr2);
    sleep(2);

    kprintf("-start reader E20. Sleep 3s\n");
    resume(rd2);
    sleep(3);

    kprintf("-start writer F30. Sleep 3s\n");
    resume(wr3);
    sleep(3);

    kprintf("-start reader G20. Sleep 3s\n");
    resume(rd3);
    sleep(3);

    kprintf("-start reader H20. Sleep 10s\n");
    resume(rd4);



    sleep (10);
    kprintf ("Starve Test finished, check order of acquisition!\n");
}

// Priority Test - Reads wait if higher priority writer
void write_prio_test() {
    int     lck;
    int     rd1, rd2, rd3, rd4;
    int     wr1;

    kprintf("\nwrite_prio_test: Reads wait if higher priority write exists."
    " lock acquisition is:\n"
    "reader A,\n"
    "writer C,\n"
    "reader B,\n"
    "reader D,\n"
    "reader E,\n");
    lck  = lcreate ();
      

    rd1 = create(reader, 2000, 20, "reader", 3, "reader A", lck, 20);
    rd2 = create(reader, 2000, 20, "reader", 3, "reader B", lck, 20);
    rd3 = create(reader, 2000, 20, "reader", 3, "reader D", lck, 20);
    rd4 = create(reader, 2000, 20, "reader", 3, "reader E", lck, 20);
    wr1 = create(writer, 2000, 20, "writer", 3, "writer C", lck, 30);

    kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
    resume(rd1);
    sleep (1);

    kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
    resume(wr1);


    kprintf("-start reader B, D, E.\n");
    resume (rd2);
    resume (rd3);
    resume (rd4);


    sleep (10);
    kprintf ("write_prio_test finished, check order of acquisition!\n");
}

// Lock Deleted Test - Verify process gets DELETED rc if lock was deleted
void lock_deleted_test()
{
    int     lck;
    int     rd1, rd2, rd3, rd4;
    int     wr1;

    kprintf("\nlock_deleted_test: Verify procs waiting on deleted locks"
    " get proper return.\n"
    "reader A: gets lock\n"
    "writer C: gets lock\n"
    "reader B: gets DELETED\n"
    "reader D: gets DELETED\n"
    "reader E: gets DELETED\n");
    lck  = lcreate ();
      

    rd1 = create(reader, 2000, 20, "reader", 3, "reader A", lck, 20);
    rd2 = create(reader, 2000, 20, "reader", 3, "reader B", lck, 20);
    rd3 = create(reader, 2000, 20, "reader", 3, "reader D", lck, 20);
    rd4 = create(reader, 2000, 20, "reader", 3, "reader E", lck, 20);
    wr1 = create(writer, 2000, 20, "writer", 3, "writer C", lck, 30);

    kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
    resume(rd1);
    sleep(2);

    kprintf("-start writer C, readers B, D, E\n");
    resume(wr1);
    resume(rd2);
    resume(rd3);
    resume(rd4);
    sleep(2);

    ldelete(lck);

    sleep (10);
    kprintf ("write_prio_test finished, check order of acquisition!\n");
}

// Lock Re-use Test - 
void lock_reuse_test()
{
    int i;
    int lck;
    int rd1, rd2, rd3, rd4;
    int wr1;

    kprintf("\nlock_reuse_test: Verify the following:\n"
    "reader A: acquires lock\n"
    "writer C: gets DELETED\n"
    "reader B: gets SYSERR\n"
    "reader D: acquires lock\n");


    // Create original lock
    lck = lcreate();
    //kprintf("Original Lock: %d\t Lock Index: %d\n", lck, LOCK_INDEX(lck));

    // Create processes that use original lock
    rd1 = create(reader, 2000, 20, "reader", 3, "reader A", lck, 20);
    rd2 = create(reader, 2000, 20, "reader", 3, "reader B", lck, 20);
    wr1 = create(writer, 2000, 20, "writer", 3, "writer C", lck, 30);

    // Start reader A
    resume(rd1);
    sleep(1);

    // Start writer C. Will get returned DELETED after lock gets
    // deleted.
    resume(wr1);
    sleep(1);

    // Delete lock
    ldelete(lck);

    // Cycle all the way back around to get back to the original
    // lock slot in the array of locks. 
    for (i=1; i<NLOCKS; i++) {
       lck = lcreate();
       ldelete(lck);
    }

    // Create a new lock (this lock will use same slot as original)
    lck = lcreate();

    // Create a new process to use the new lock spot. 
    rd3 = create(reader, 2000, 20, "reader", 3, "reader D", lck, 20);
    //kprintf("Final Lock: %d\t Lock Index: %d\n", lck, LOCK_INDEX(lck));

    // Start reader B: Should get returned SYSERR since the lock it
    // uses was already deleted and a new lock is using that index
    resume(rd2);

    // Start reader D that uses new lock
    resume(rd3);
    sleep(4);

    kprintf ("lock_reuse_test finished, check order of acquisition!\n");
}

// Multi Lock Test
void multi_lock_test() {
    int lck1, lck2;
    int rd1, rd2, rd3, rd4, rd5, rd6;
    int wr1, wr2;
    int rw1;

    kprintf("\nMultiple Lock Test: tests releasing multiple locks at a time"
    " lock acquisition is:\n" 
    "reader A, Lock 1\n"
    "reader B, Lock 2\n"
    "writer C, Lock 1\n"
    "writer F, Lock 2\n"
    " combo H, Lock 1\n"
    "reader D, Lock 1\n"
    " combo H, Lock 2\n"
    "reader E, Lock 2\n");

    lck1  = lcreate();
    lck2  = lcreate();
  

    rd1 = create(reader, 2000, 20, "reader", 3, "reader A", lck1, 20);
    rd2 = create(reader, 2000, 20, "reader", 3, "reader B", lck2, 20);
    rd3 = create(reader, 2000, 20, "reader", 3, "reader D", lck1, 20);
    rd4 = create(reader, 2000, 20, "reader", 3, "reader E", lck2, 20);
    wr1 = create(writer, 2000, 20, "writer", 3, "writer C", lck1, 30);
    wr2 = create(writer, 2000, 20, "writer", 3, "writer F", lck2, 35);
    rw1 = create( combo, 2000, 20, "writer", 5, " combo H", lck1, lck2, 20, 30);

    resume(rd1);
    sleep(1);
    resume(wr1);
    resume(rd2);
    sleep(1);
    resume(wr2);
    resume(rw1);
    resume(rd3);
    resume(rd4);


    sleep (12);
    kprintf ("Multi Lock Test finished, check order of acquisition!\n");
}

int main() {
    int i, s;
    int count = 0;
    char buf[8];


    kprintf("Please Input:\n");
    while ((i = read(CONSOLE, buf, sizeof(buf))) <1);
    buf[i] = 0;
    s = atoi(buf);
    switch (s)
    {
    case 1:
        starve_test();
        break;
    
    case 2:
        write_prio_test();
        break;
        
    case 3:
        lock_deleted_test();
        break;

    case 4:
        lock_reuse_test();
        break;

    case 5:
        multi_lock_test();
        break;
    }
}




