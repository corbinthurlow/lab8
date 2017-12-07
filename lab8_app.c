#include "lab8defs.h"
#include "simptris.h"
#include "clib.h"
#include "yakk.h"                     /* contains kernel definitions */


#define TASK_STACK_SIZE   512         /* stack size in words */

//task sizes for each task
int ArrivalTaskStk[TASK_STACK_SIZE];		//task for incoming pieces
int CommTaskStk[TASK_STACK_SIZE];			//task for communication with simptris
int STaskStk[TASK_STACK_SIZE];				//task for stats

//taken from clib.s
extern int NewPieceID;
extern int NewPieceType;
extern int NewPieceOrientation;
extern int NewPieceColumn;
extern int ScreenBitMap0;
extern int ScreenBitMap1;
extern int ScreenBitMap2;
extern int ScreenBitMap3;
extern int ScreenBitMap4;
extern int ScreenBitMap5;




//the idea im kinda thinking is we have a function for placing different pieces ... we can change this if we want these are simply the base ideas of functions I think we may need... feel free to change them
void placeStraghtPiece(piece* Piece);
void placeLeftCorner(piece* Piece);
void placeRightCorner(piece* Piece);
void placeRightCornerNotFlat(piece* Piece, int pieceColumn);
void placeLeftCornerNotFlat(piece* Piece, int pieceColumn);
void placeCornerPiece(piece* Piece);


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


//index to keep track of simpcmd array
int simpCMDIdx;

void *NewPieceQ[PIECEQ_SIZE]; //space for new piece queue
YKQ *PieceQPtr;

//index for piece array
int PieceArrayIdx;

//I think we will need sometime of function to send a command back to the simptris like this... not sure
void sendCommand(int id,int command,int parameter);

//encapuslating functions to increment indexs for piece array and command array
void incCmdIdx(){
	//if there is more space for commands
	if((simpCMDIdx + 1) < CMDQ_SIZE){
			//increment the index
			simpCMDIdx++;
	}else{
		//else reset it
		simpCMDIdx = 0;
	}
}

void incPieceArrayIdx(){
	//if there is more space for pieces
	if((PieceArrayIdx + 1) < PIECEQ_SIZE){
		//increment the index
		PieceArrayIdx++;
	}else{
		PieceArrayIdx = 0;
	}
}



/**********************************************TASKS ARE HERE************************************/





void CommTask(void){
	//need to create task to communicate with simptris
	//I am just following the basic task structure to create these..... I really doubt they work but just kinda getting started and I am sure you can add something to make them work or change them to make them better
	
}

void ArrivalTask(void){
	//need to create to task to deal with arrival of pieces
}


//I feel that this task will be similar to code that he has given to us in the past for CPU usage and such... i Kinda copied and pasted some different code into here for now
void STask(void)           /* tracks statistics */
{	

	//copied all of this from previous task code as well for now
	unsigned max, switchCount, idleCount;
    int tmp;
	
	//again copied this stuff from previous task code
    YKDelayTask(1);
    printString("Welcome to the YAK kernel\r\n");
    printString("Determining CPU capacity\r\n");
    YKDelayTask(1);
    YKIdleCount = 0;
    YKDelayTask(5);
    max = YKIdleCount / 25;
    YKIdleCount = 0;
	//not really sure where these calls should take place...we had them here so I am going to leave them here
	SeedSimptris(SEED);
 	StartSimptris();
	//I am unsure where we want to create the other two tasks but my initial thought it here for now
	//To me I think it makes sense to have the arrival task lowest priority....but feel free to change it
	
	//For some reason when calling these issues happen......not sure why 
	//YKNewTask(ArrivalTask,(void *) &ArrivalTaskStk[TASK_STACK_SIZE],3);
	//YKNewTask(CommTask,(void *) &CommTaskStk[TASK_STACK_SIZE],2);	

	//just literally copied this from other task code   
    while (1)
    {
        YKDelayTask(20);
        
        YKEnterMutex();
        switchCount = YKCtxSwCount;
        idleCount = YKIdleCount;
        YKExitMutex();
        printString("<CS: ");
        printInt((int)switchCount);
        printString(", CPU: ");
        tmp = (int) (idleCount/max);
        printInt(100-tmp);
        printString(">\r\n");
        
        YKEnterMutex();
        YKCtxSwCount = 0;
        YKIdleCount = 0;
        YKExitMutex();
    }
}







/**********************************************************************************************************/







void main(void)
{
	//I want to initialize my gloabl indices here
	PieceArrayIdx = 0;
	simpCMDIdx = 0;	
	YKInitialize();
	
	//What priority should this task get????? 
    YKNewTask(STask, (void *) &STaskStk[TASK_STACK_SIZE], 0);
	//before calling YKRUN set up the queue and semaphores like in other labs
	//create the semaphore for command received
	cmdReceiveSem = YKSemCreate(0);
	//create the semaphore for when a block touches down
	touchdownSem = YKSemCreate(0);
	//initialize the queue for commands
	CmdQPtr = YKQCreate(CmdQ,CMDQ_SIZE);
	//initialize the queue for the pieces
	PieceQPtr = YKQCreate(NewPieceQ,PIECEQ_SIZE);
	//then call YKRun()    
	YKRun();
}


