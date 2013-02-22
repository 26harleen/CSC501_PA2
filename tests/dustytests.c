#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

/*--------------------------------Test 1--------------------------------*/
 
/*----------------------------------Test 2---------------------------*/
void reader(char *msg, int lck, int lprio)
{
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
    kprintf ("  %s: rc = %d\n", msg, rc);
    kprintf ("  %s: acquired lock, sleep 3s\n", msg);
    sleep (3);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

void writer(char *msg, int lck, int lprio)
{
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
    kprintf ("  %s: rc = %d\n", msg, rc);
    kprintf ("  %s: acquired lock, sleep 3s\n", msg);
    sleep (3);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

// Starvation Test
void starve_test()
{
    int     lck;
    int     rd1, rd2, rd3, rd4, rd5, rd6;
    int     wr1;

    kprintf("\nStarve Test: wait on locks with priority. Expected order of"
    " lock acquisition is:\n" 
    "reader A,\n"
    "reader B,\n"
    "reader D,\n"
    "reader E,\n"
    "writer C,\n"
    "reader F,\n"
    "reader G,\n");
    lck  = lcreate ();
  

    rd1 = create(reader, 2000, 20, "reader", 3, "reader A", lck, 20);
    rd2 = create(reader, 2000, 20, "reader", 3, "reader B", lck, 20);
    rd3 = create(reader, 2000, 20, "reader", 3, "reader D", lck, 20);
    rd4 = create(reader, 2000, 20, "reader", 3, "reader E", lck, 20);
    rd5 = create(reader, 2000, 20, "reader", 3, "reader F", lck, 20);
    rd6 = create(reader, 2000, 20, "reader", 3, "reader G", lck, 20);
    wr1 = create(writer, 2000, 20, "writer", 3, "writer C", lck, 20);

    kprintf("-start reader A, and Writer C, then sleep 1s. lock granted to reader A\n");
    resume(rd1);
    resume(wr1);
    sleep (1);


    kprintf("-start reader B. Sleep 1s\n");
    resume (rd2);
    sleep(1);
    kprintf("-start reader D. Sleep 1s\n");
    resume (rd3);
    sleep(1);
    kprintf("-start reader E. Sleep 1s\n");
    resume (rd4);
    sleep(1);
    kprintf("-start reader F. Sleep 1s\n");
    resume (rd5);
    sleep(1);
    kprintf("-start reader G. Sleep 1s\n");
    resume (rd6);
    sleep(1);


    sleep (10);
    kprintf ("Starve Test finished, check order of acquisition!\n");
}

// Priority Test - Reads wait if higher priority writer
void write_prio_test()
{
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


    kprintf("-start reader B, D, E. reader B is granted lock.\n");
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
    "get proper return.\n"
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
    sleep(1);

    kprintf("-start writer C, readers B, D, E\n");
    resume(wr1);
    resume(rd2);
    resume(rd3);
    resume(rd4);
    sleep(1);

    ldelete(lck);

    sleep (10);
    kprintf ("write_prio_test finished, check order of acquisition!\n");
}

int main( )
{
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
    }
}




