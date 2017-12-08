
#include "lab8defs.h"
#include "simptris.h"
#include "clib.h"
#include "yakk.h"                     // contains kernel definitions 
#define TASK_STACK_SIZE   512         // stack size in words 
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
void placeStraightPiece(piece* Piece);
void placeLeftCorner(piece* Piece,int col);
void placeRightCorner(piece* Piece,int col);
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
YKQ* PieceQPtr;

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

void sendCommand(int id,int command,int parameter){
	simpcmd* newCmd;
	newCmd = &CmdQ[simpCMDIdx];
	incCmdIdx();
	newCmd->id = id;
	newCmd->cmd = command;
	newCmd->param = parameter;
	YKQPost(CmdQPtr,(void*)newCmd);
	//make tmp cmd
	//cmd->id = id;
	//cmd->cmd = command;
	//cmd->param = parameter;
}



//function to see if the left or right bin is flat at the moment
int binIsFlat(int bin){
	int i;
	int mask;
	if(bin == LEFT){
		for(i = 0; i < ROWS - 1; i++){
			if((ScreenBitMap0 & mask) == (ScreenBitMap1 & mask) && (ScreenBitMap1 & mask) == (ScreenBitMap2 & mask)){
				//if the this is a one and we entered here we know all the left bin is a one 				
				if(ScreenBitMap0 & mask){				
				return TRUE;
				}
				mask = mask << 1;
			}else{
				return FALSE;

			}
		}
	}else if(bin == RIGHT){
		for(i = 0; i < ROWS - 1; i++){
			if((ScreenBitMap3 & mask) == (ScreenBitMap4 & mask) && (ScreenBitMap4 & mask) == (ScreenBitMap5 & mask)){
				//if the this is a one and we entered here we know all the right bin is a one 				
				if(ScreenBitMap3 & mask){				
				return TRUE;
				}
				mask = mask << 1;
			}else{
				return FALSE;
			}
		}
	}
	return TRUE;
}




//function to determine the depth of the highest occupied bin in the game
int getBinMinDepth(int bin){
	int i;
	int mask = 1;
	int count = 0;
	if(bin == LEFT){
		for(i = 0; i < ROWS - 1; i++){
			if((ScreenBitMap0 & mask) || (ScreenBitMap1 & mask) || (ScreenBitMap2 & mask)){
				return count;
			}else{
				mask = mask << 1;
				count++;
			}
		
		}
	}
	else if(bin == RIGHT){
		for(i = 0; i < ROWS - 1; i++){
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






void placeLeftCorner(piece* Piece, int col) {
	simpcmd* cmpPtr;
	int id = Piece->pieceID;
		
	if (col == 0) {
		if (Piece->orientation == BLCORNER) {
			//piece is already in place. we're good
		} else if (Piece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			
		} else {
			
		}
	} else if (col == 1) {
		if (Piece->orientation == BRCORNER) {
			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);
	
		} else if (Piece->orientation == BLCORNER) {
	
			sendCommand(id, SLIDE, LEFT);
	
		} else if (Piece->orientation == TRCORNER) {
	
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
				
			sendCommand(id, SLIDE, LEFT);
	
		} else if (Piece->orientation == TLCORNER) {
	
			sendCommand(id, ROTATE, RTLEFT);
	
			sendCommand(id, SLIDE, LEFT);
					
		} else {
		}
	} else if (col == 2) {
		if (Piece->orientation == BLCORNER) {
			
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);

		} else if (Piece->orientation == BRCORNER) {
	
			sendCommand(id, ROTATE, RTRIGHT);
	
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);
	
		} else if (Piece->orientation == TRCORNER) {
	
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);
	
		} else if (Piece->orientation == TLCORNER) {
	
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, SLIDE, LEFT);
			
		}
	} else {
	}
}


void placeRightCorner(piece* Piece, int col) {
	
	int id = Piece->pieceID;
	
	if (col == 5) {
		if (Piece->orientation == BRCORNER) {
			//piece is already in place. we're good
		} else if (Piece->orientation == TRCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);
						
		} else {
		}
	} else if (col == 4) {
		if (Piece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (Piece->orientation == BLCORNER) {

			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (Piece->orientation == TRCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (Piece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
							
		} else {
			
		}
	} else if (col == 3) {
		if (Piece->orientation == BLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);

		} else if (Piece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);
						
		} else if (Piece->orientation == TRCORNER) {
			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);
						
		} else if (Piece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, SLIDE, RIGHT);
			
		}
	} else {
	}
}

void placeLeftCornerNotFlat(piece* Piece, int col) {
	
	int id = Piece->pieceID;
	if (col == 0) {
		if (Piece->orientation == BLCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);	
					
		} else if (Piece->orientation == TLCORNER) {

			sendCommand(id, SLIDE, RIGHT);			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
			
		} else if (Piece->orientation == TRCORNER || Piece->orientation == BRCORNER)
			placeLeftCornerNotFlat(Piece, col + 1);
		else {
			
		}
	} else if (col == 1) {
		if (Piece->orientation == BRCORNER) {

			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
					
		} else if (Piece->orientation == BLCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, RIGHT);
		
		} else if (Piece->orientation == TRCORNER) {
			
			sendCommand(id, SLIDE, RIGHT);
		
		} else if (Piece->orientation == TLCORNER) {
			
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, RIGHT);
						
		} else {
		}
	} else if (col == 2) {
		
		if (Piece->orientation == BLCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, ROTATE, RTLEFT);
				
		} else if (Piece->orientation == BRCORNER) {			

			sendCommand(id, ROTATE, RTLEFT);

		} else if (Piece->orientation == TRCORNER) {
			//you're good already. don't do anything
		} else if (Piece->orientation == TLCORNER) {	
			sendCommand(id, ROTATE,RTRIGHT);
		}
	} else {
		
	}
}



void placeRightCornerNotFlat(piece* Piece, int col) {
	
	int id = Piece->pieceID;
	
	if (col == 5) {
		if (Piece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, LEFT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);
		
		} else if (Piece->orientation == TRCORNER) {

			sendCommand(id, SLIDE, LEFT);		
			sendCommand(id, ROTATE, RTLEFT);
			sendCommand(id, SLIDE, LEFT);
		
		} else {
			
		}
	} else if (col == 4) {
		if (Piece->orientation == BRCORNER) {
			
			sendCommand(id, SLIDE, LEFT);		
			sendCommand(id, ROTATE, RTRIGHT);			
			sendCommand(id, ROTATE, RTRIGHT);
		
		} else if (Piece->orientation == BLCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, SLIDE, LEFT);		
		
		} else if (Piece->orientation == TRCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
			
			sendCommand(id, SLIDE, LEFT);
		
		} else if (Piece->orientation == TLCORNER) {
		
			sendCommand(id, SLIDE, LEFT);	
						
		} else {
			
		}
	} else if (col == 3) {
		if (Piece->orientation == BLCORNER) {

			sendCommand(id, ROTATE, RTRIGHT);

		} else if (Piece->orientation == BRCORNER) {
		
			sendCommand(id, ROTATE, RTRIGHT);
			sendCommand(id, ROTATE, RTRIGHT);
		
		} else if (Piece->orientation == TRCORNER) {
		
			sendCommand(id, ROTATE, RTLEFT);
		
		} else if (Piece->orientation == TLCORNER) {
			//do nothing. we're already in the right spot.
		}
	} else {
		printInt(col);
	}
}





void placeStraightPiece(piece* Piece){
	simpcmd* newCmd;
	int max;
	int i;
	int lBinDepth = getBinMinDepth(LEFT);
	int rBinDepth = getBinMinDepth(RIGHT);
	static int maxDepth = 3; 
	int ID = Piece->pieceID;
	//new piece is in the left bin
	if(Piece->column - 3 < 0){
		
		//determine if we place it in the left bin
		if(((lBinDepth - rBinDepth) >= maxDepth) || (((rBinDepth - lBinDepth) < maxDepth) && (lFlat || !rFlat))){
				//correct the piece placement			
				if(Piece->column == 0){
				sendCommand(ID,SLIDE,RIGHT);
				if(Piece->orientation == TOWER){
					sendCommand(ID,ROTATE,RTRIGHT);
				}
			}else if(Piece->column == 2){
				sendCommand(ID,SLIDE,LEFT);
				if(Piece->orientation == TOWER){
					sendCommand(ID,ROTATE,RTRIGHT);
				}
			}else if(Piece->orientation == TOWER){
				sendCommand(ID,ROTATE,RTRIGHT);			
			}
		}else{	//else we should place in the right bin
			
			//decide how many right shifts we need
			if(Piece->column == 0){
				max = 4;
			}else if(Piece->column == 1){
				max =3;
			}else {
				max = 2;
			}
			for(i = 0; i < max; i++){
				sendCommand(ID,SLIDE,RIGHT);
				if(Piece->orientation == TOWER && i == 0){
					sendCommand(ID,ROTATE,RTRIGHT);
				}
			}
		}
	}else{
		//determine if we place in the right bin
		if(((rBinDepth - lBinDepth) >= maxDepth) || (((lBinDepth - rBinDepth) < maxDepth) && (rFlat || !lFlat))){
				//correct the piece placement			
				if(Piece->column == 5){
				sendCommand(ID,SLIDE,LEFT);
				if(Piece->orientation == TOWER){
					sendCommand(ID,ROTATE,RTRIGHT);
				}
			}else if(Piece->column == 3){
				sendCommand(ID,SLIDE,RIGHT);
				if(Piece->orientation == TOWER){
					sendCommand(ID,ROTATE,RTRIGHT);
				}		
			}else if(Piece->orientation == TOWER){
			sendCommand(ID,ROTATE,RTRIGHT);
		}
	}
	else{	//we should place in the left bin
		
		if(Piece->column == 5){
			max = 4;
		}else if(Piece->column == 4){
			max = 3;
		}else{
			max = 2;
		}

		for(i = 0; i < max; i++){
			sendCommand(ID,SLIDE,LEFT);
			if(Piece->orientation == TOWER && i == 0){
				sendCommand(ID,ROTATE,RTRIGHT);
			}		
		}
	}
}

}

void placeCornerPiece(piece* Piece){

	int binUse;
	int tmpCol;
	int id = Piece->pieceID;
	if(Piece->column < 3){
		binUse = LEFT;
	}else{
		binUse = RIGHT;
	}
	
	if(lFlat && rFlat){
		if((getBinMinDepth(LEFT) > getBinMinDepth(RIGHT))){
			if(binUse == LEFT){
				lFlat = 0;
				placeLeftCorner(Piece,Piece->column);
			}else{
				if(Piece->column == 3){
					sendCommand(id,SLIDE,LEFT);
					tmpCol = (Piece->column - 1);
				}else if(Piece->column == 4){
					sendCommand(id,SLIDE,LEFT);
					sendCommand(id,SLIDE,LEFT);
					tmpCol = (Piece->column - 2);
				}else{
					sendCommand(id,SLIDE,LEFT);
					sendCommand(id,SLIDE,LEFT);
					sendCommand(id,SLIDE,LEFT);
					tmpCol = (Piece->column - 3);
				}			
				lFlat = 0;
				if(tmpCol != 2)
					
				placeLeftCorner(Piece,tmpCol);
				}
			}else{
				if(binUse == RIGHT){
					rFlat = 0;
					placeRightCorner(Piece,Piece->column);
				}else{
					if(Piece->column == 2){
						sendCommand(id,SLIDE,RIGHT);
						tmpCol = (Piece->column + 1);
					}else if (Piece->column == 1){
						sendCommand(id,SLIDE,RIGHT);
						sendCommand(id,SLIDE,RIGHT);
						tmpCol = (Piece->column + 2);				
					}else{
						sendCommand(id,SLIDE,RIGHT);	
						sendCommand(id,SLIDE,RIGHT);
						sendCommand(id,SLIDE,RIGHT);
						tmpCol = (Piece->column + 3);						
					}
					rFlat = 0;
					if(tmpCol != 3)
						
					placeRightCorner(Piece,tmpCol);
					}
				}
			}else if(lFlat && !rFlat){

			if(binUse == LEFT){
				if(Piece->column == 2){
					sendCommand(id,SLIDE,RIGHT);
					tmpCol = (Piece->column + 1);
				}else if(Piece->column == 1){
					sendCommand(id,SLIDE,RIGHT);
					sendCommand(id,SLIDE,RIGHT);
					tmpCol = (Piece->column + 2);	
				}else{
					sendCommand(id,SLIDE,RIGHT);
					sendCommand(id,SLIDE,RIGHT);
					sendCommand(id,SLIDE,RIGHT);
					tmpCol = (Piece->column + 3);
				}
				rFlat = 1;
				if(tmpCol != 3)

				placeRightCornerNotFlat(Piece,tmpCol);
				}else{
					rFlat = 1;
					placeRightCornerNotFlat(Piece,Piece->column);
				}
			}else{
				if(binUse == LEFT){
					lFlat = 1;
					placeLeftCornerNotFlat(Piece,Piece->column);
				}else{
					if(Piece->column == 3){
						sendCommand(id,SLIDE,LEFT);
						tmpCol =(Piece->column - 1);
					}else if(Piece->column == 4){
						sendCommand(id,SLIDE,LEFT);
						sendCommand(id,SLIDE,LEFT);
						tmpCol = (Piece->column - 2);
					}else{
						sendCommand(id,SLIDE,LEFT);
						sendCommand(id,SLIDE,LEFT);
						sendCommand(id,SLIDE,LEFT);
						tmpCol = (Piece->column - 3);
					}
					lFlat = 1;
					placeLeftCornerNotFlat(Piece,tmpCol);
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
		printString("starting commTask loop\n");
		newCmd = (simpcmd*) YKQPend(CmdQPtr);
		//if(CmdQPtr == NULL){
			//printString("EMPTIED THE CMD Q\n");		
		//}else{
		//	printString("ERROR!!!!!!! QUEUE NOT EMPTIED\n");
		//}		
		YKSemPend(cmdReceiveSem);
printString("inside and moving forawrd with commTask\n");
		if (newCmd->cmd == SLIDE) {
			SlidePiece(newCmd->id, newCmd->param);
		}
		else {
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
	//simpcmd* newCmd;
	//need to create to task to deal with arrival of pieces
	printString("In arrival task\n");
	while(1){
		printString("Pending on new piece.......\n");
		newPiece =(piece *) YKQPend(PieceQPtr);
		printString("We got : ");
		printInt(newPiece->pieceID);
		printNewLine();

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
	YKNewTask(ArrivalTask,(void *) &ArrivalTaskStk[TASK_STACK_SIZE],3);
	YKNewTask(CommTask,(void *) &CommTaskStk[TASK_STACK_SIZE],2);	

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
    YKNewTask(STask, (void *) &STaskStk[TASK_STACK_SIZE], 0);
	//before calling YKRUN set up the queue and semaphores like in other labs
	//create the semaphore for command received
	cmdReceiveSem = YKSemCreate(0);
	//create the semaphore for when a block touches down
	touchdownSem = YKSemCreate(1);
	//initialize the queue for commands
	CmdQPtr = YKQCreate(CmdQ,CMDQ_SIZE);
	//initialize the queue for the pieces
	PieceQPtr = YKQCreate(NewPieceQ,PIECEQ_SIZE);
	//then call YKRun()    
	YKRun();
}				
