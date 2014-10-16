#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
int ldelete (int lockdescriptor)
{
	STATWORD ps;
	int pid;
	disable(ps);
	if(isbadlock(lockdescriptor) || locks[lockdescriptor].lockState == 0)
	{
		restore(ps);
		return SYSERR;
	}
	locks[lockdescriptor].lockState = 0;
	if(locks[lockdescriptor].timesUsed != proctab[currpid].locksHeld[lockdescriptor][1])
	{
		restore(ps);
		return SYSERR;
	}
	if(nonempty(locks[lockdescriptor].lqhead))
	{
		while((pid = getfirst(locks[lockdescriptor].lqhead)) != EMPTY)
		{
			proctab[pid].plockret = DELETED;
			ready(pid, RESCHNO);
		}
		resched();
	}
	restore(ps);
	return OK;
}