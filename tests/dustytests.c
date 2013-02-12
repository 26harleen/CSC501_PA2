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
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void writer(char *msg, int lck, int lprio)
{
	kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
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

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		"lock acquisition is: reader A, reader B, reader D, writer C & E\n");
        lck  = lcreate ();
      

	rd1 = create(reader, 2000, 20, "reader", 3, "reader A", lck, 20);
	rd2 = create(reader, 2000, 20, "reader", 3, "reader B", lck, 20);
	rd3 = create(reader, 2000, 20, "reader", 3, "reader D", lck, 20);
	rd4 = create(reader, 2000, 20, "reader", 3, "reader E", lck, 20);
	rd5 = create(reader, 2000, 20, "reader", 3, "reader F", lck, 20);
	rd6 = create(reader, 2000, 20, "reader", 3, "reader G", lck, 20);
        wr1 = create(writer, 2000, 20, "writer", 3, "writer C", lck, 20);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);
	resume (rd5);
	resume (rd6);


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
		"lock acquisition is: reader A, writer C, reader B, reader D, reader E\n");
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
        //sleep (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);


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
		break;
	}
}




