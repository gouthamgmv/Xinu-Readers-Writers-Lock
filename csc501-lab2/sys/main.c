#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>

int reader(char, int, int);
int multipleReader(char, int, int, int, int);
int writer(char, int, int);
int deletingWriter (char, int, int);
int recreatingWriter (char, int, int);
int retryingWriter (char, int, int);

int reader1, reader2, reader3;
int writer1, writer2, writer3;
int lck1, lck2, lck3;

int main()
{
        kprintf("\n\nTEST 1:\n Multiple readers\n ");
		lck1 = lcreate();
		lck2 = lcreate();
		lck3 = lcreate();
        resume(reader1 = create(reader,2000,20,"reader1",3,'A', lck1, 30));
        resume(reader2 = create(reader,2000,20,"reader2",3,'B', lck1, 40));
		resume(reader3 = create(reader,2000,20,"reader3",3,'C', lck1, 30));
		sleep(5);
		kprintf("\n\nTEST 2:\n Multiple writers\n");
        resume(writer1 = create(writer,2000,20,"writer1",3,'A', lck1, 30));
        resume(writer2 = create(writer,2000,20,"writer2",3,'B', lck1, 40));
		resume(writer3 = create(writer,2000,20,"writer3",3,'C', lck1, 30));
		sleep(10);
		kprintf("\n\nTEST 3:\n Lock deletion\n A acquires the lock while B and C wait for the lock. A deletes the lock when it's done. B and C return DELETED!\n");
        resume(writer1 = create(deletingWriter,2000,20,"writer1",3,'A', lck1, 30));
        resume(writer2 = create(writer,2000,20,"writer2",3,'B', lck1, 40));
		resume(writer3 = create(writer,2000,20,"writer3",3,'C', lck1, 30));
		sleep(5);
		kprintf("\n\nTEST 4:\n Lock deletion\n A acquires the lock while B waits for it. A deletes the lock when it's done. C uses the same lock descriptor. B returns DELETED! If B tries querying for the lock again, SYSERR will be returned.\n");
		lck1 = lcreate();
        resume(writer1 = create(recreatingWriter,2000,20,"writer1",3,'A', lck1, 30));
        resume(writer2 = create(retryingWriter,2000,20,"writer2",3,'B', lck1, 40));
		sleep(5);
		resume(writer3 = create(writer,2000,20,"writer3",3,'C', lck1, 30));
		sleep(5);
		kprintf("\n\nTEST 5:\n Locking policy: Writer A already executing. Reader B will wait. \n");
        resume(writer1 = create(writer,2000,20,"writer1",3,'A', lck1, 30));
		resume(reader1 = create(reader,2000,20,"reader1",3,'B', lck1, 30));
		sleep(5);
		kprintf("\n\nTEST 6:\n Locking policy: Reader A already executing. Writer B with higher priority waiting. Writer D and Reader E with priority between B and C but D has more waiting time. Reader C will wait. \n");
		resume(reader1 = create(reader,2000,20,"reader1",3,'A', lck1, 30));
        resume(writer1 = create(writer,2000,20,"writer1",3,'B', lck1, 50));
		resume(reader2 = create(reader,2000,20,"reader2",3,'C', lck1, 30));
        resume(writer2 = create(writer,2000,20,"writer2",3,'D', lck1, 40));
		resume(reader3 = create(reader,2000,20,"reader3",3,'E', lck1, 40));
		sleep(10);
		kprintf("\n\nTEST 7:\n Multiple locks acquired and released! \n");
		resume(reader1 = create(multipleReader,2000,20,"reader1",5,'A', lck1, lck2, lck3, 30));
		return 0;
}
int reader(char c, int ldes1, int prio)
{
	int flag;
	flag = lock(ldes1, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	sleep(2);
	kprintf("\n%c is releasing lock %d!", c, ldes1);
	releaseall(1, ldes1);
	//kprintf("done?");
}	

int writer(char c, int ldes1, int prio)
{
	int flag;
	flag = lock(ldes1, WRITE, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	sleep(2);
	kprintf("\n%c is releasing lock %d!", c, ldes1);
	releaseall(1, ldes1);
}	

int deletingWriter(char c, int ldes1, int prio)
{
	int flag;
	flag = lock(ldes1, WRITE, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	sleep(2);
	kprintf("\n%c is releasing lock %d!", c, ldes1);
	ldelete(ldes1);
}	

int recreatingWriter(char c, int ldes1, int prio)
{
	int flag;
	flag = lock(ldes1, WRITE, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	sleep(2);
	kprintf("\n%c is releasing lock %d!", c, ldes1);
	ldelete(ldes1);
	lck1 = lcreate();
}	

int retryingWriter(char c, int ldes1, int prio)
{
	int flag;
	flag = lock(ldes1, WRITE, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		kprintf("\n%c trying to acquire lock %d again!", c, ldes1);
		flag = lock(ldes1, WRITE, prio);
		if(flag == SYSERR)
		{	
			kprintf("\n%c's lock returned SYSERR!", c);
			return;
		}
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	sleep(2);
	kprintf("\n%c is releasing lock %d!", c, ldes1);
	releaseall(1, ldes1);
}	
int multipleReader(char c, int ldes1, int ldes2, int ldes3, int prio)
{
	int flag;
	flag = lock(ldes1, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	flag = lock(ldes2, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes2);
	flag = lock(ldes1, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes3);
	flag = releaseall(3, ldes1, ldes2, ldes3);
	kprintf("\n Releasing all locks!");
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	int ldes4 = 40;
	flag = lock(ldes1, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes1);
	flag = lock(ldes2, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes2);
	flag = lock(ldes1, READ, prio);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	if(flag == DELETED)
	{
		kprintf("\n%c's lock was deleted!", c);
		return;
	}
	kprintf("\n%c has obtained lock %d!", c, ldes3);
	kprintf("\n Trying to release a lock that is not held by %c", c);
	flag = releaseall(3, ldes1, ldes2, ldes4);
	if(flag == SYSERR)
	{
		kprintf("\n%c's lock returned SYSERR!", c);
		return;
	}
	//kprintf("done?");
}	
