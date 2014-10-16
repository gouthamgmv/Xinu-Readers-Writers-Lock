#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
int lock (int ldes1, int type, int priority)
{
	int wait, procElem;
	STATWORD ps;
	disable(ps);
	if(isbadlock(ldes1) || locks[ldes1].lockState == 0) {
        restore(ps);
        return SYSERR;
    }
	if(locks[ldes1].timesUsed != proctab[currpid].locksHeld[ldes1][1] && proctab[currpid].locksHeld[ldes1][0] != 0)
	{
		//kprintf("%d %d", locks[ldes1].timesUsed, proctab[currpid].locksHeld[ldes1][1]);
		restore(ps);
		return SYSERR;
	}
	if(locks[ldes1].readers == 0 && locks[ldes1].writers == 0)
		wait = 0;
	else if(locks[ldes1].readers != 0 && locks[ldes1].writers == 0 && type == READ)
	{
		procElem = q[locks[ldes1].lqtail].qprev;
		while (priority < q[procElem].qkey)
		{
			if(q[procElem].qtype == WRITE)
				wait = 1;
			procElem = q[procElem].qprev;
		}
	}
	else if(locks[ldes1].readers != 0 && locks[ldes1].writers == 0 && type == WRITE)
		wait = 1;
	else if(locks[ldes1].writers != 0 && locks[ldes1].readers == 0)
		wait = 1;
	if(wait == 1)
	{
		proctab[currpid].pstate = PRLOCK;
		proctab[currpid].plock = ldes1;
		insert(currpid, locks[ldes1].lqhead, priority);
		q[currpid].qtype = type;
		q[currpid].qtime = ctr1000;
		locks[ldes1].procLock[currpid] = 1;
	proctab[currpid].locksHeld[ldes1][0] = 1;
	proctab[currpid].locksHeld[ldes1][1] = locks[ldes1].timesUsed;

		proctab[currpid].plockret = OK;
		resched();
		restore(ps);
		return proctab[currpid].plockret;
	}
	locks[ldes1].procLock[currpid] = 1;
	proctab[currpid].locksHeld[ldes1][0] = 1;
	proctab[currpid].locksHeld[ldes1][1] = locks[ldes1].timesUsed;
	//kprintf("\n setyabbadabb %d %d", locks[ldes1].timesUsed, proctab[currpid].locksHeld[ldes1][1]);
	if(type == READ)
		locks[ldes1].readers++;
	else
		locks[ldes1].writers++;
	restore(ps);
	return OK;
}
