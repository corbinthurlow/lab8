

#include "clib.h"
#include "yakk.h"
//#include "lab6defs.h"
//#include "lab7defs.h"
#include "lab8defs.h"


extern int KeyBuffer;
void InterruptTicker();
void InterruptReseter();
void InterruptKeyboarder();

//extern YKQ *MsgQPtr;
//extern struct msg MsgArray[];
extern int GlobalFlag;
extern unsigned int YKTickNum;
/*
void InterruptTicker(){
	
	/*static int next = 0;
	static int data = 0;
     create a message with tick (sequence #) and pseudo-random data 
    MsgArray[next].tick = YKTickNum;
    data = (data + 89) % 100;
    MsgArray[next].data = data;
    if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
	printString("  TickISR: queue overflow! \n");
    else if (++next >= MSGARRAYSIZE)
	next = 0;

}

void InterruptReseter(){
	printNewLine();
	printString("Reset Pressed");
	exit(0);
}

void InterruptKeyboarder(){
	//GlobalFlag = 1;
	/*	
	char c;
    c = KeyBuffer;

    if(c == 'a') YKEventSet(charEvent, EVENT_A_KEY);
    else if(c == 'b') YKEventSet(charEvent, EVENT_B_KEY);
    else if(c == 'c') YKEventSet(charEvent, EVENT_C_KEY);
    else if(c == 'd') YKEventSet(charEvent, EVENT_A_KEY | EVENT_B_KEY | EVENT_C_KEY);
    else if(c == '1') YKEventSet(numEvent, EVENT_1_KEY);
    else if(c == '2') YKEventSet(numEvent, EVENT_2_KEY);
    else if(c == '3') YKEventSet(numEvent, EVENT_3_KEY);
    else {
        print("\nKEYPRESS (", 11);
        printChar(c);
        print(") IGNORED\n", 10);
    }

}
*/

// pre lab6 hander code

//#include "clib.h"
//#include "yakk.h"

/*************************************************myinth.c for lap 8**************************************/
int counter = 0;
extern int KeyBuffer;
//externing to the piece array made in the task code
extern piece PieceArray[];
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
//externing these as well if we need them here....not sure
//semaphores for receiving command and touchdown
extern YKSEM *cmdReceiveSem;
extern YKSEM *touchdownSem;
void InterruptTicker();
void InterruptReseter();
void InterruptKeyboarder();
//index for piece array
int NewPieceArrayIdx;
extern YKQ *PieceQPtr;

//to increment index for new pieces
void incNewPieceArrayIdx(){
	//if there is more space for pieces
	if((NewPieceArrayIdx + 1) < PIECEQ_SIZE){
		//increment the index
		NewPieceArrayIdx++;
	}else{
		NewPieceArrayIdx = 0;
	}
}

void InterruptTicker(){

/*
printNewLine();
counter++; 
printString("TICK ");
printInt(counter);
printNewLine();
*/
}

void InterruptReseter(){
	printNewLine();
	printString("Reset Pressed");
	exit(0);
}

void InterruptKeyboarder(){
	/*int j = 0;
	if(KeyBuffer == 'd' || KeyBuffer == 'D'){
		printNewLine();
		printString("DELAY KEY PRESSED");
		printNewLine();
		
		for(j = 0; j < 7000; j++){
		
		}
		printNewLine();
		printString("DELAY COMPLETE");
		printNewLine();
	} else if(KeyBuffer == 'p' || KeyBuffer == 'P') {
		YKSemPost(NSemPtr);
	}else{
		printNewLine();
		printString("KEYPRESS (");
		printChar(KeyBuffer);
		printString(") IGNORED");
		printNewLine();
	}
*/
}

void newPiece_handler(){
	//simply setting up the needed members of our piece struct for the index in the array of pieces
	PieceArray[NewPieceArrayIdx].pieceID = NewPieceID;
	PieceArray[NewPieceArrayIdx].type = NewPieceType;
	PieceArray[NewPieceArrayIdx].orientation = NewPieceOrientation;
	PieceArray[NewPieceArrayIdx].column = NewPieceColumn;
	//Now we will check to see if the new piece q is full or not
	if(YKQPost(PieceQPtr,(void *) &(PieceArray[NewPieceArrayIdx])) == 0){
		printString("Piece queue is full\n");
	}else{
		//else increment the index
		incNewPieceArrayIdx();
	}
}


void gameOver_handler(){
	//simply end the game
	exit(0);
}

//I think we would need to just pose to the cmd
void cmdReceived_handler(){
	YKSemPost(cmdReceiveSem);
}

void touchDown_handler(){



}

void lineClr_handler(){



}

