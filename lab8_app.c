#include "lab8defs.h"
#include "simptris.h"
#include "clib.h"
#include "yakk.h"                     /* contains kernel definitions */


#define TASK_STACK_SIZE   512         /* stack size in words */
int ArrivalTaskStk[TASK_STACK_SIZE];
int CommTaskStk[TASK_STACK_SIZE];
int STaskStk[TASK_STACK_SIZE];


extern int ScreenBitMap0;
extern int ScreenBitMap1;
extern int ScreenBitMap2;
extern int ScreenBitMap3;
extern int ScreenBitMap4;
extern int ScreenBitMap5;

//semaphores for receiving command and touchdown
YKSEM *cmdReceiveSem;
YKSEM *touchdownSem;

//array for static allocate pieces
piece PieceArray[PIECEQ_SIZE];
//allocate space for cmds
simpcmd simpCMDarray[CMDQ_SIZE];

//cmd queue
void *CmdQ[CMDQ_SIZE];
//ykq for commands
YKQ* CmdQPtr;









void STask(void)           /* tracks statistics */
{
while(1){
	unsigned int i;
	for(i = 0; i  < 65535; i++){
	printInt(i);
	printNewLine();
    SeedSimptris(i);
 	StartSimptris();
	YKDelayTask(50);
}
	}
}




void main(void)
{
    YKInitialize();


    YKNewTask(STask, (void *) &STaskStk[TASK_STACK_SIZE], 0);
	
    YKRun();
}


