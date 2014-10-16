#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
int lcreate(void)
{
	STATWORD ps;
	disable(ps);
	int lockDescriptor;
	lockDescriptor = getLock();
	if(lockDescriptor == -1)
	{
		restore(ps);
		return SYSERR;
	}
	locks[lockDescriptor].lockState = 1;
	locks[lockDescriptor].readers = 0;
	locks[lockDescriptor].writers = 0;
	locks[lockDescriptor].timesUsed++;
	restore(ps);
	return lockDescriptor;
}
int getLock()
{
	int i;
	for(i=0; i<NLOCKS; i++)
	{
		if(locks[i].lockState == 0)
			return i;
	}
	return SYSERR;
}