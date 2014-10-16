#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
struct lockTable locks[NLOCKS];
void linit()
{
	struct lockTable *lptr;
	int i, j, k;
	for(i=0; i<NLOCKS; i++)
	{
		lptr = &locks[i];
		lptr->lockState = 0;
		lptr->timesUsed = -1;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
		for(j=0; j<NPROC; j++)
			lptr->procLock[j] = 0;
	}
}