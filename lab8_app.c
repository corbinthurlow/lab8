#include "clib.h"
#include "yakk.h"
#include "simptris.h"
#include "lab8defs.h"

                     // contains kernel definitions 
#define TASK_STACK_SIZE   256         // stack size in words 
#define TRUE 1
#define FALSE 0
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

int lFlat = 1;
int rFlat = 1;


//the idea im kinda thinking is we have a function for placing different pieces ... we can change this if we want these are simply the base ideas of functions I think we may need... feel free to change them
void placeStraightPiece(piece* newPiece);
void placeLeftCorner(piece* newPiece,int col);
void placeRightCorner(piece* Piece,int col);
void placeRightCornerNotFlat(piece* newPiece, int col);
void placeLeftCornerNotFlat(piece* newPiece, int col);
void placeCornerPiece(piece* newPiece);

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
YKQ *CmdQPtr;

//struct msg MsgArray[256];
//index to keep track of simpcmd array
int simpCMDIdx;

void *NewPieceQ[PIECEQ_SIZE]; //space for new piece queue
YKQ *PieceQPtr;

//index for piece array
int PieceArrayIdx;

//I think we will need sometime of function to send a command back to the simptris like this... not sure
void sendCommand(int id,int command,int parameter);

int getBinMinDepth(int bin);

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

void sendCommand(int id,int command,int parameter){
	simpcmd* newCmd;
	newCmd = &simpCMDarray[simpCMDIdx];
	incCmdIdx();
	newCmd->id = id;
	newCmd->cmd = command;
	newCmd->param = parameter;
	YKQPost(CmdQPtr,(void*)newCmd);
}

//function to determine the depth of the highest occupied bin in the game
int getBinMinDepth(int bin){
	int i;
	int mask = 1;
	int count = 0;
	if(bin == LEFT){
		for(i = 0; i < (ROWS - 1); i++){
			if((ScreenBitMap0 & mask) || (ScreenBitMap1 & mask) || (ScreenBitMap2 & mask)){
				return count;
			}else{
				mask = mask << 1;
				count++;
			}
		
		}
	}
	else if(bin == RIGHT){
		for(i = 0; i < (ROWS - 1); i++){
			if((ScreenBitMap3 & mask) || (ScreenBitMap4 & mask) || (ScreenBitMap5 & mask)){
				return count;
			}else{
				mask = mask << 1;
				count++;
			}
		}	
	}
	return count;
}






void placeLeftCorner(piece* newPiece, int col) {
	int id = newPiece->id;
		
	if (col == 0) {
		if (newPiece->orientation == BLCORNER) {
			//piece is already in place. we're good
		} else if (newPiece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			
		} else {
			//printString("Sanity check!!! lab8app.c line 188");
		}
	} else if (col == 1) {
		if (newPiece->orientation == BRCORNER) {
			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);
	
		} else if (newPiece->orientation == BLCORNER) {
	
			sendCommand(id, SLIDE, LEFT);
	
		} else if (newPiece->orientation == TRCORNER) {
	
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
				
			sendCommand(id, SLIDE, LEFT);
	
		} else if (newPiece->orientation == TLCORNER) {
	
			sendCommand(id, ROTATE, RTLEFT);
	
			sendCommand(id, SLIDE, LEFT);
					
		} else {
			//printString("Sanity check!!! lab8app.c line 214");
		}
	} else if (col == 2) {
		if (newPiece->orientation == BLCORNER) {
			
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);

		} else if (newPiece->orientation == BRCORNER) {
	
			sendCommand(id, ROTATE, RTRIGHT);
	
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);
	
		} else if (newPiece->orientation == TRCORNER) {
	
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);
	
		} else if (newPiece->orientation == TLCORNER) {
	
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);
			
		}
	} else {
		//printString("\n\nSanity Check!!! line 244 ----   column: ");
		//printInt(piece_column);
	}
}


void placeRightCorner(piece* newPiece, int col) {
	int id = newPiece->id;
	
	if (col == 5) {
		if (newPiece->orientation == BRCORNER) {
			//piece is already in place. we're good
		} else if (newPiece->orientation == TRCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);
						
		} else {
			//printString("Sanity check!!! lab8app.c line 393");
		}
	} else if (col == 4) {
		if (newPiece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (newPiece->orientation == BLCORNER) {

			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (newPiece->orientation == TRCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (newPiece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
							
		} else {
			//printString("Sanity check!!! lab8app.c line 562");
		}
	} else if (col == 3) {
		if (newPiece->orientation == BLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);

		} else if (newPiece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);
						
		} else if (newPiece->orientation == TRCORNER) {
			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);
						
		} else if (newPiece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);
			
		}
	} else {
		//printString("\n\nSanity Check!!! line 311 ----   column: ");
		//printInt(piece_column);
	}
	
}

void placeLeftCornerNotFlat(piece* newPiece, int col) {
	int id = newPiece->id;
	if (col == 0) {
		if (newPiece->orientation == BLCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);	
					
		} else if (newPiece->orientation == TLCORNER) {

			sendCommand(id, SLIDE, RIGHT);			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (newPiece->orientation == TRCORNER || newPiece->orientation == BRCORNER)
			placeLeftCornerNotFlat(newPiece, col + 1);
		else {
			//printString("Sanity check!!! lab8app.c line 719");
		}
	} else if (col == 1) {
		if (newPiece->orientation == BRCORNER) {

			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
					
		} else if (newPiece->orientation == BLCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
		
		} else if (newPiece->orientation == TRCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
		
		} else if (newPiece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
						
		} else {
			//printString("Sanity check!!! lab8app.c line 689");
		}
	} else if (col == 2) {
		
		if (newPiece->orientation == BLCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
				
		} else if (newPiece->orientation == BRCORNER) {			

			sendCommand(id, ROTATE, RTLEFT);

		} else if (newPiece->orientation == TRCORNER) {
			//you're good already. don't do anything
		} else if (newPiece->orientation == TLCORNER) {	
			sendCommand(id, ROTATE, RTRIGHT);
		}
	} else {
		//printString("\n\nSanity Check!!! line 376 ----   column: ");
		//printInt(piece_column);
	}
}



void placeRightCornerNotFlat(piece* newPiece, int col) {
	int id = newPiece->id;
	
	if (col == 5) {
		if (newPiece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);
		
		} else if (newPiece->orientation == TRCORNER) {

			sendCommand(id, SLIDE, LEFT);		
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, LEFT);
		
		} else {
			//printString("Sanity check!!! lab8app.c line 788");
		}
	} else if (col == 4) {
		if (newPiece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, LEFT);		
			sendCommand(id, ROTATE, RTRIGHT);			
			sendCommand(id, ROTATE, RTRIGHT);
		
		} else if (newPiece->orientation == BLCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);		
		
		} else if (newPiece->orientation == TRCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
			
			sendCommand(id, SLIDE, LEFT);
		
		} else if (newPiece->orientation == TLCORNER) {
		
			sendCommand(id, SLIDE, LEFT);	
						
		} else {
			//printString("Sanity check!!! lab8app.c line 856");
		}
	} else if (col == 3) {
		if (newPiece->orientation == BLCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);

		} else if (newPiece->orientation == BRCORNER) {
		
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
		
		} else if (newPiece->orientation == TRCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
		
		} else if (newPiece->orientation == TLCORNER) {
			//do nothing. we're already in the right spot.
		}
	} else {
		//printString("\n\nSanity Check!!! line 445 ----   column: ");
		//printInt(piece_column);
	}
}





void placeStraightPiece(piece* newPiece){
	int max;
	int i;
	int lBinDepth = getBinMinDepth(LEFT);
	int rBinDepth = getBinMinDepth(RIGHT);
	static int maxDepth = 3;

	int pieceID = newPiece->id;

	//new piece is in the left bin
	if (newPiece->column - 3 < 0) {

		//should we place this in the left bin?
		if (((lBinDepth - rBinDepth) >= maxDepth) || (((rBinDepth - lBinDepth) < maxDepth) && (lFlat || (!rFlat)))) {
			
			//make corrections to piece placement
			if(newPiece->column == 0){
				sendCommand(pieceID, SLIDE, RIGHT);

				if (newPiece->orientation == TOWER) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}	
			}
			else if(newPiece->column == 2){
				sendCommand(pieceID, SLIDE, LEFT);

				if (newPiece->orientation == TOWER) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}	
			}
			else if (newPiece->orientation == TOWER) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}	
		}

		//we should place this in the right bin
		else {
			
			
			//decide how many right shifts we need
			if(newPiece->column == 0) 
				max = 4;
			else if(newPiece->column == 1)
				max = 3;
			else
				max = 2;

			for(i = 0; i < max; i++){
				sendCommand(pieceID, SLIDE, RIGHT);

				if (newPiece->orientation == TOWER && i == 0) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}		
			}

		}
		
					
	}
	else {
		//should we place this in the right bin?
		if(	((rBinDepth - lBinDepth) >= maxDepth) || (((lBinDepth - rBinDepth) < maxDepth) && (rFlat || !lFlat))) {
			
			//make corrections to piece placement
			if(newPiece->column == 5){
				sendCommand(pieceID, SLIDE, LEFT);

				if (newPiece->orientation == TOWER) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}		
			}
			else if(newPiece->column == 3){
				sendCommand(pieceID, SLIDE, RIGHT);

				if (newPiece->orientation == TOWER) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}	
			}
			else if (newPiece->orientation == TOWER) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}	
		}

		//we should place this in the left bin
		else {
			
			//decide how many left shifts we need
			if(newPiece->column == 5) 
				max = 4;
			else if(newPiece->column == 4)
				max = 3;
			else
				max = 2;

			for(i = 0; i < max; i++){
				sendCommand(pieceID, SLIDE, LEFT);

				if (newPiece->orientation == TOWER && i == 0) {
					sendCommand(pieceID, ROTATE, RTRIGHT);
				}	
			}
		}
	}
}

void placeCornerPiece(piece* newPiece){
	int binUse;
	int tmpCol;
	int id = newPiece->id;
	
	if (newPiece->column < 3)
		binUse = LEFT;
	else
		binUse = RIGHT;
	
	if (lFlat && rFlat) {
		if ((getBinMinDepth(LEFT) > getBinMinDepth(RIGHT))) {
			if (binUse == LEFT) {
				lFlat = 0;
				placeLeftCorner(newPiece, newPiece->column);
			} else {
				if (newPiece->column == 3) {
			
					sendCommand(id, SLIDE, LEFT);

					tmpCol = (newPiece->column - 1);

				} else if (newPiece->column == 4) {
			
					sendCommand(id, SLIDE, LEFT);
					sendCommand(id, SLIDE, LEFT);
			
					tmpCol = (newPiece->column - 2);
				} else {//(newPiece->column == 5)

					sendCommand(id, SLIDE, LEFT);
					sendCommand(id, SLIDE, LEFT);
					sendCommand(id, SLIDE, LEFT);
					
					tmpCol = (newPiece->column - 3);
				}
				lFlat = 0;				
				placeLeftCorner(newPiece, tmpCol);
			}
		} else {
			if (binUse == RIGHT) {
				rFlat = 0;
				placeRightCorner(newPiece, newPiece->column);
			} else {
				
				if (newPiece->column == 2) {
				
					sendCommand(id, SLIDE, RIGHT);
				
					tmpCol = (newPiece->column + 1);

				} else if (newPiece->column == 1) {
			
					sendCommand(id, SLIDE, RIGHT);
					sendCommand(id, SLIDE, RIGHT);

					tmpCol = (newPiece->column + 2);
				
				} else {

					sendCommand(id, SLIDE, RIGHT);
					sendCommand(id, SLIDE, RIGHT);
					sendCommand(id, SLIDE, RIGHT);
					
					tmpCol = (newPiece->column + 3);
				}
				rFlat = 0;					
				placeRightCorner(newPiece, tmpCol);
			}
		}	
	} else if (lFlat && !rFlat) {
		
		if (binUse == LEFT) {
			
			if (newPiece->column == 2) {
				
				sendCommand(id, SLIDE, RIGHT);					

				tmpCol = (newPiece->column + 1);

			} else if (newPiece->column == 1) {
			
				sendCommand(id, SLIDE, RIGHT);
				sendCommand(id, SLIDE, RIGHT);

				tmpCol = (newPiece->column + 2);
			} else {
				
				sendCommand(id, SLIDE, RIGHT);
				sendCommand(id, SLIDE, RIGHT);
				sendCommand(id, SLIDE, RIGHT);
				
				tmpCol = (newPiece->column + 3);
			}
			rFlat = 1;			
			placeRightCornerNotFlat(newPiece, tmpCol);
			
		} else { //binToUse == RIGHT
			
			rFlat = 1;
			placeRightCornerNotFlat(newPiece, newPiece->column);
		}
		
	} else { //!lFlat && rFlat
		if (binUse == LEFT) {
			lFlat = 1;
			placeLeftCornerNotFlat(newPiece, newPiece->column);
		} else {	//binToUse == RIGHT
			
			if (newPiece->column == 3) {
		
				sendCommand(id, SLIDE, LEFT);

				tmpCol = (newPiece->column - 1);

			} else if (newPiece->column == 4) {
		
				sendCommand(id, SLIDE, LEFT);
				sendCommand(id, SLIDE, LEFT);
		
				tmpCol = (newPiece->column - 2);
			} else {
				
				sendCommand(id, SLIDE, LEFT);
				sendCommand(id, SLIDE, LEFT);
				sendCommand(id, SLIDE, LEFT);
				
				tmpCol = (newPiece->column - 3);
			}
			
			lFlat = 1;					
			placeLeftCornerNotFlat(newPiece, tmpCol);
		}
	}
}






/**********************************************TASKS ARE HERE*********************************/





void CommTask(void){
	simpcmd* newCmd;
	SeedSimptris((long) SEED);
 	StartSimptris();
	//need to create task to communicate with simptris
	//I am just following the basic task structure to create these..... I really doubt they work but just kinda getting started and I am sure you can add something to make them work or change them to make them better
	while(1){
		//printString("starting commTask loop\n");
		newCmd = (simpcmd *) YKQPend(CmdQPtr);
		//printString("got a command!\n");
				//if(CmdQPtr == NULL){
			//printString("EMPTIED THE CMD Q\n");		
		//}else{
		//	printString("ERROR!!!!!!! QUEUE NOT EMPTIED\n");
		//}		
		YKSemPend(cmdReceiveSem);
		//printString("inside and moving forawrd with commTask\n");
		if (newCmd->cmd == SLIDE) {
			//printString("PIECE SHOULD SLIDE\n");
			//printString("ID IS:");
			//printInt(newCmd->id);
			//printNewLine();
			SlidePiece(newCmd->id, newCmd->param);
		}
		else {
			//printString("PIECE SHOULD ROTATE\n");
			//printString("ID IS:");
			//printInt(newCmd->id);
			//printNewLine();
			RotatePiece(newCmd->id, newCmd->param);
		}
		
		
	}	
	
}


/*
//defines for moving
#define LEFT 0
#define RIGHT 1
//defines for rotating
#define RTRIGHT 0
#define RTLEFT 1
//define for the function to call
#define SLIDE 0
#define ROTATE 1
*/

void ArrivalTask(void){
	piece* newPiece;		
	simpcmd* newCmd;
	//need to create to task to deal with arrival of pieces
	//printString("In arrival task\n");
	while(1){
		//printString("Pending on new piece.......\n");
		newPiece =(piece *) YKQPend(PieceQPtr);
		//printString("We got : ");
		//printInt(newPiece->id);
		//printNewLine();

		if (newPiece->type == STRAIGHTPIECE){
			placeStraightPiece(newPiece);
		}else if(newPiece->type == CORNERPIECE){
			placeCornerPiece(newPiece);
		}
		//call placeStraightPiece(piece)
		//else call placeCornerPiece(piece)
		

		//newCmd =  &CmdQ[simpCMDIdx];
		//incCmdIdx();
		// do some checking and determining which moves to do

		//newCmd->id = newPiece->pieceID;

		//newCmd->cmd = LEFT;
		//newCmd->param = SLIDE;
		//printString("placing command into queue\n");
		//YKQPost(CmdQPtr, (void *) newCmd);

		//printString("placing 2nd command into queue\n");
		//newCmd->cmd = RTLEFT;
		//newCmd->param = ROTATE;
		//YKQPost(CmdQPtr, (void *) newCmd);	
	}
}


//I feel that this task will be similar to code that he has given to us in the past for CPU usage and such... i Kinda copied and pasted some different code into here for now
void STask(void)           // tracks statistics 
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
	
	//I am unsure where we want to create the other two tasks but my initial thought it here for now
	//To me I think it makes sense to have the arrival task lowest priority....but feel free to change it
	
	//For some reason when calling these issues happen......not sure why 
	YKNewTask(ArrivalTask,(void *) &ArrivalTaskStk[TASK_STACK_SIZE],25);
	YKNewTask(CommTask,(void *) &CommTaskStk[TASK_STACK_SIZE],20);	

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







/**************************************************************************************/







void main(void)
{
	//I want to initialize my gloabl indices here
	PieceArrayIdx = 0;
	simpCMDIdx = 0;	
	YKInitialize();
	
	//What priority should this task get????? 
    YKNewTask(STask, (void *) &STaskStk[TASK_STACK_SIZE], 10);
	//before calling YKRUN set up the queue and semaphores like in other labs
	//create the semaphore for command received
	cmdReceiveSem = YKSemCreate(1);
	//create the semaphore for when a block touches down
	touchdownSem = YKSemCreate(1);
	//initialize the queue for commands
	CmdQPtr = YKQCreate(CmdQ,CMDQ_SIZE);
	//initialize the queue for the pieces
	PieceQPtr = YKQCreate(NewPieceQ,PIECEQ_SIZE);
	//then call YKRun()    
	YKRun();
}				
