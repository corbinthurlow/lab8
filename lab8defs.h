//defines for moving
#define LEFT 0
#define RIGHT 1
//defines for rotating
#define RTRIGHT 1 //Clock wise
#define RTLEFT 0	//counter clock wise

#define SLIDE 1	//slide 1
#define ROTATE 0 //rotate 0


//Seed #1247 - straight piece on left wall
//Seed #1251 - straight piece on right wall
//Seed #1258 - corner piece 0 on left wall
//Seed #1511 - corner piece 1 on right wall
//Seed #1503 - corner piece 2 on right wall
//Seed #1506 - corner piece 3 on left wall

#define SEED 87532
//defines for orientations
#define FLAT 0
#define TOWER 1
#define BLCORNER 0
#define BRCORNER 1
#define TRCORNER 2
#define TLCORNER 3


//defines for board dimensions
#define COLUMNS 6
#define ROWS 16

//defines for pieces
#define CORNERPIECE 0
#define STRAIGHTPIECE 1

//size for Piece Q size
#define PIECEQ_SIZE 40
//size of for command Q size
#define CMDQ_SIZE 40





//struct for pieces
typedef struct newPiece {
	int id;
	int orientation;
	int column;
	int type;
} piece;

//struct for commands to be sent to simptris
typedef struct cmdsimp {
	int id;
	int cmd;
	int param;
} simpcmd;
