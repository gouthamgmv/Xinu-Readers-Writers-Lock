#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
	static unsigned long *j;
void findNextRW(int ldes1);
void runNextRW(int ldes1, int procElem);
int releaseall (numlocks, ldes1)
	int numlocks;
	int ldes1;
{
	asm("movl %ebp, j");
	//kprintf("\n %d", *(j+2));
	//numlocks = *(j+2);
	int i, sysError = 0;
	STATWORD ps;
	disable(ps);
	//kprintf("\n what about here?");
	for(i=0; i<numlocks; i++)
	{
		//kprintf("\n but not here?");
		ldes1 = *(j+3+i);
		//kprintf("\n%d", ldes1);
		int procElem, maxPrio = 0, longestReader = -1, longestWriter = -1, maxReadTime = 0, maxWriteTime = 0, nextProc = 0;
		if(isbadlock(ldes1) || locks[ldes1].lockState == 0) 
		{
			//kprintf("\n coming here?");
			sysError = 1;
			continue;
		}
		//kprintf("\n yabbadabb %d %d", locks[ldes1].timesUsed, proctab[currpid].locksHeld[ldes1][1]);
		if(locks[ldes1].timesUsed != proctab[currpid].locksHeld[ldes1][1])
		{
			//kprintf("\n or here?");
			sysError = 1;
			continue;
		}
		//kprintf("\ntest %d %d", locks[ldes1].readers, locks[ldes1].writers);
		if(locks[ldes1].readers > 0)
			locks[ldes1].readers--;
		else if(locks[ldes1].writers > 0)
			locks[ldes1].writers--;
		locks[ldes1].procLock[currpid] = 0;
		proctab[currpid].locksHeld[ldes1][0] = 0;
		proctab[currpid].locksHeld[ldes1][1] = 0;
		if(isempty(locks[ldes1].lqhead))
			continue;
		procElem = q[locks[ldes1].lqtail].qprev;
		if(q[procElem].qkey != q[q[procElem].qprev].qkey)
		{
			//kprintf("\n %d %d %d", q[procElem].qtype, locks[ldes1].writers, locks[ldes1].readers);
			if(q[procElem].qtype == READ && locks[ldes1].writers == 0)
			{
				findNextRW(ldes1);
			}
			else if(q[procElem].qtype == WRITE && locks[ldes1].writers == 0 && locks[ldes1].readers == 0)
			{
				runNextRW(ldes1, procElem);
			}
		//kprintf("here6");
			continue;
		}
		else
		{
			maxPrio = q[procElem].qkey;
			
			while(q[procElem].qkey == maxPrio)
			{
			
				if(q[procElem].qtype == READ && q[procElem].qtime > maxReadTime)
					longestReader = procElem;
				else if(q[procElem].qtype == WRITE && q[procElem].qtime > maxWriteTime)
					longestWriter = procElem;
				if(longestWriter >= 0 && longestReader >= 0)
				{
					if( (maxReadTime-maxWriteTime) < 1000 || (maxWriteTime-maxReadTime) < 1000)
						nextProc = longestWriter;
					else if(maxReadTime > maxWriteTime)
						nextProc = longestReader;
					else if(maxReadTime < maxWriteTime)
						nextProc = longestWriter;
				}
				else if(longestReader >= 0 && longestWriter == 0)
					nextProc = longestReader;
				else if(longestWriter >= 0 && longestReader == 0)
					nextProc = longestWriter;
				else	
					nextProc = -1;
				procElem = q[procElem].qprev;
			}
			
			if(q[nextProc].qtype == READ && locks[ldes1].writers == 0)
				findNextRW(ldes1);
			else if(q[nextProc].qtype == WRITE && locks[ldes1].writers == 0 && locks[ldes1].readers == 0)
				runNextRW(ldes1, nextProc);
				
		}
	}
	if(sysError == 1)
	{
		restore(ps);
		return SYSERR;
	}
	
	restore(ps);
	resched();
	return OK;	
}
void findNextRW(int ldes1)
{
	int procElem, tmp, max = -999;
	procElem = q[locks[ldes1].lqtail].qprev;
	//	kprintf("here12");
	while(procElem != locks[ldes1].lqhead)
	{
	
		//kprintf("here13");
		if(q[procElem].qtype == WRITE && q[procElem].qkey > max)
			max = q[procElem].qkey;
		procElem = q[procElem].qprev;
	}
	procElem = q[locks[ldes1].lqtail].qprev;
		//kprintf("here14");
	while(procElem != locks[ldes1].lqhead)
	{
		//kprintf("here15");
		if(q[procElem].qtype == READ && q[procElem].qkey >= max)
		{
			tmp = q[procElem].qprev;
			runNextRW(ldes1, procElem);
			procElem = tmp;
		//kprintf("here16");
		}		
	}
}
void runNextRW(int ldes1, int procElem)
{
	if(q[procElem].qtype == READ)
		locks[ldes1].readers++;
	else if(q[procElem].qtype == WRITE)
		locks[ldes1].writers++;
	locks[ldes1].procLock[procElem] = 1;
	proctab[currpid].locksHeld[ldes1][0] = 1;
	proctab[currpid].locksHeld[ldes1][1] = locks[ldes1].timesUsed;
	dequeue(procElem);
	//kprintf("\n procElem is %d", procElem);
	ready(procElem, RESCHNO);
		//kprintf("here17");
}