#define NLOCKS 50
#define READ 0
#define WRITE 1
struct lockTable
{
	int lockState;
	int readers;
	int writers;
	int timesUsed;
	int lqhead;
	int lqtail;
	int procLock[NPROC];
};
#define isbadlock(l) (l<0 || l>=NLOCKS)
extern struct lockTable locks[];
extern unsigned long ctr1000;
extern void linit();
extern int lcreate(void);
extern int ldelete(int);
extern int lock(int, int, int);
extern int releaseall(int, int, ...);