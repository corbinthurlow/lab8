#include "clib.h"
#include "yaku.h"
#include "yakk.h"


int IdleTaskStk[IDLE_TASK_STACK_SIZE];	// Setting the size of the stack for the idle task
TCBptr YKCurrTask;						// TCB pointer to the current task

// Function Prototypes
void YKIdleTask(void); 			// Idle task, always ready .... currently written in yaks.s
void insertReady(TCBptr tmp);	// Insert a TCB to the ready list
void removeReady(void);			// Remove the current ready TCB
void removeBlocked(TCBptr tmp);	//this will remove a given blocked task from the block list
void YKDispatcherASM();			// Dispatch function written in assembly called from here
TCBptr getSemBlockTask(YKSEM *semaphore); // Get highest priority task waiting on semaphore
void **moveQPtr(YKQ *queue, void** QPtr); // moves either head or tail pointer to next index
void* ssTemp;					// Temp global to store ss to be able to push and pop it
void* bxTemp;
// Needed global variables
unsigned int YKCtxSwCount;    	// Variable for tracking context switches
unsigned int YKTickNum;       	// Variable incremented by tick handler
unsigned int YKNestingLevel;  	// Variable tracking nesting level
unsigned int YKKernelStarted; 	// Variable indicating kernel has started
unsigned int YKIdleCount;		// Idle count variable
// TCB and linkedlist declarations
TCBptr YKRdyList;  				// a list of TCBs of all ready tasks in order of decreasing priority
TCBptr YKBlockList; 			//tasks delayed or blocked or suspended
TCBptr YKFreeTCBList;			//a list of available TCBs
TCB YKTCBArray[MAX_TASKS+1]; 	//array to allocate all needed TCBs including idle task
int saveContext = 0;			//value used to determine whether or not context needs saving
TCBptr oldTask;					//pointer used for keeping track of the old task
// Semaphore declarations
//YKSEM* YKFreeSemList;
YKSEM YKSemArray[MAX_SEM];
YKQ YKQArray[MAX_MESSAGE_QUEUES];
int YKQIndex = 0;
TCBptr queueWaitListInsert(TCBptr front, TCBptr target);
TCBptr queueWaitListRemove(TCBptr front, TCBptr target); //removes a tcb when no longer waiting for a queue...will at least alwasy return idle task to be run
YKEVENT YKEventArray[MAX_EVENTS];
int idxNextAvailEvent;





//initializes the global variables needed to run the RTOS
//and adds IdleTask to queue
void YKInitialize(void){

	int i;

	YKEnterMutex();		//disable interrupts 
	
	YKFreeTCBList = &(YKTCBArray[0]);	//set the free tcb list to first address in allocated space
	for (i = 0; i < MAX_TASKS; i++)		
		YKTCBArray[i].next = &(YKTCBArray[i+1]);
	YKTCBArray[MAX_TASKS].next = NULL; 		//last task is idle task

	// initialize YKSemArray[] array with free semaphores
	for (i = 0; i < MAX_SEM; i++) {
		YKSEM initSem;
		initSem.value = 0;
		initSem.isTaken = 0;
		YKSemArray[i] = initSem;
	}
	

	YKCtxSwCount = 0;		//init switch count to zero
	YKIdleCount = 0;		//init idlecount to zero
	YKTickNum = 0;			//init tick number to zero
	YKNestingLevel = 0;		//init nesting level to 0
	YKKernelStarted = 0;	//variable used to tell of kernel has started
	idxNextAvailEvent = 0;
	
	YKRdyList = NULL; //ready list inited to null
	YKBlockList = NULL;	//block list inited to null
	YKCurrTask = NULL;	//currtask inited to null

	YKNewTask(YKIdleTask, (void*)&IdleTaskStk[IDLE_TASK_STACK_SIZE], LOWEST_PRIORITY);	//created the idle task

	YKExitMutex();	//re-enables interrupts


}

//creates a new task. 
//init task data
void YKNewTask(void (*task)(void), void *taskStack, unsigned int priority){
	TCBptr newTaskPtr = YKFreeTCBList;
	
	
	YKEnterMutex(); //disable interrupts when creating a new task

	//set initial register values
	*((int*)taskStack-1) = 0x0200; //set flags
	*((int*)taskStack-2) = 0;		 		// set space for CS
	*((int*)taskStack-3) = (int*)task;		//set space for IP
	*((int*)taskStack-4) = &taskStack;		//set space for bp
	*((int*)taskStack-5) = 0;				//set space for DX
	*((int*)taskStack-6) = 0;				//set space for CX
	*((int*)taskStack-7) = 0;				//set space for BX
	*((int*)taskStack-8) = 0;				//set space for AX
	*((int*)taskStack-9) = 0;				//set space for ES
	*((int*)taskStack-10) = 0;				//set space for DI
	*((int*)taskStack-11) = 0;				//set space for SI
	*((int*)taskStack-12) = 0;				//set space for DS
	*((int*)taskStack-13) = 0;				//set space for SS

	//allocate a spot in the task array

	YKFreeTCBList->stackptr = ((int*)taskStack-13);
	YKFreeTCBList->state = NEW;
	YKFreeTCBList->priority = priority;
	YKFreeTCBList->delay = 0;
	YKFreeTCBList->sem = 0;
	YKFreeTCBList->queue = 0;
	//YKFreeTCBList->occup = 0;

	//remove this TCB form AvailTCBList
	newTaskPtr = YKFreeTCBList; // redundant? - shawn
   	YKFreeTCBList = newTaskPtr->next;

	//
	insertReady(newTaskPtr);

	if (YKKernelStarted == 1) {
		YKScheduler();
	}

	YKExitMutex();

}

//this function simply increments nestlevel
void YKEnterISR(){
	YKNestingLevel++;
}


//call this at the end of ISR while interrupts still are disabled
void YKExitISR(){
	//decrement the nesting count
	YKNestingLevel--;
	//call the scheduler if the count is 0 to get the highest ready task
	if(YKNestingLevel == 0 && YKKernelStarted == 1){
		YKScheduler();
	}


}


//delay task
void YKDelayTask(unsigned int count){ 
	//if count is zero don't delay and simply return
	if(count == 0){
		return;	
	}
	//Possible disable interrupts?????
	YKEnterMutex();
	//set the current task delay count to the count passed in
	YKCurrTask->delay = count;
	//remove the task from the ready task
	removeReady();
	//call the scheduler to see the next ready task
	if(YKKernelStarted == 1){

	YKScheduler();
	
	}
	YKExitMutex();

}




//YAK's idle task. this task does not do much, but spins in a loop and counts.
// The while loop is exactly 4 lines of of instructions. The YKIdleCount = YKIdleCount
// takes two instructions. This is fine because YKIdleTask is the only function to 
// modify YKIdleCount (unless CPU resets it to 0, then there will be a problem

void YKRun(void) {
	YKKernelStarted = 1;
	YKScheduler();
}

void YKScheduler(void) {	
	if (YKCurrTask != NULL && YKRdyList->priority == YKCurrTask->priority) {
		//do nothing, because we're executing highest priority already
		return;
	}

	//check interrupt nesting level, if greater than 0, don't save context
	if(YKNestingLevel == 0 && YKCurrTask != NULL){ // we are not in interrupts - shawn
		saveContext = 1;
	} else {
		saveContext = 0;
	}

	YKCtxSwCount++;
	//assign the first task in the list to YKCurrTask
	oldTask = YKCurrTask;
	YKCurrTask = YKRdyList;
	
	//assign new status to task
	if(oldTask != NULL) 
		oldTask->state = READY; // need to consider if old task blocked itself. We don't want to make it ready here - shawn
	
	YKCurrTask->state = RUNNING;
	
	YKDispatcherASM();
}



//
void YKTickHandler(){
	//if the number of clock ticks have occured make the delayed task read
	//we will need to check each task that is blocked/suspended
	TCBptr tmp;

	TCBptr tmp2;
	//this will store the delay count for each task we look at
	int delay_count;
	//assign tmp to the blocklist
	tmp = YKBlockList;
	//increment YKTickNum
	YKTickNum++;
	while(tmp != NULL){ //){
		//i think we will need to disable interrupts here since this could be called from an tick....
		//YKEnterMutex(); // still disable even if highest priority? -shawn
		//set delay_count to the current task we are looking at delay count
		delay_count = tmp->delay;
		//if the delay count is more than zero
		//set tmp2 to tmp next
		tmp2 = tmp->next;
		if(delay_count > 0){
			//decrement the tasks delay count now
			 tmp->delay = delay_count - 1;
			 if(tmp->delay == 0){
			 	//set the task state to ready
			 	tmp->state = READY;
			 	//remove it from the blocked list
			 	removeBlocked(tmp);

			 }
		}
		tmp = tmp2;	//go to next blocked task
		//YKExitMutex();
	}

}


/*
 * code to insert an entry in doubly linked ready list sorted by
 * priority numbers (lowest number first).  tmp points to TCB
 * to be inserted 
 */ 
void insertReady(TCBptr tmp) {
	TCBptr tmp2;
	
	tmp->state = READY;
    if (YKRdyList == NULL) { /* is this first insertion? YKIdleTask - shawn */
		YKRdyList = tmp;
		YKRdyList->next = NULL;
		YKRdyList->prev = NULL;
    } else { /* not first insertion */
		tmp2 = YKRdyList;	/* insert in sorted ready list */
		while (tmp2->priority < tmp->priority) /* While current priority is higher than inserting prio - shawn*/
			tmp2 = tmp2->next;	/* assumes idle task is at end */
		if (tmp2->prev == NULL)	/* insert in list before tmp2 */ //This TCB is highest priority - shawn
			YKRdyList = tmp;
		else
			tmp2->prev->next = tmp;
		tmp->prev = tmp2->prev;
		tmp->next = tmp2;
		tmp2->prev = tmp;
    }
}

/*
 * code to remove an entry from the ready list and put in
 * suspended list, which is not sorted.  (This only works for the
 * current task, so the TCB of the task to be suspended is assumed
 * to be the first entry in the ready list.)
 */
void removeReady(void){
	TCBptr tmp2,tmp; //set up tmp TCBptrs
	tmp = YKRdyList;	//set tmp tp the ready TCB
	tmp->state = BLOCKED; //set the tmp state to Blocked
	YKRdyList = tmp->next;	//then set tmp next to be ready 
	tmp->next->prev = NULL;	//set tmp's next prev to null
	tmp->next = YKBlockList;
	YKBlockList = tmp;
	tmp->prev = NULL;
	if(tmp->next != NULL){
		tmp->next->prev = tmp; // pointing to self - Shawn
	}


}

//this will remove a given blocked task from the block list
void removeBlocked(TCBptr tmp){
	TCBptr tmp_local;
	//set the passed in tcb to ready
	tmp->state = READY;
	//now we will fix the blocked list
	if(tmp->prev == NULL){	//meaning there was only one blocked and it was tmp
		//tmp->next = YKBlockList;	//set tmp next to the block list now
		YKBlockList = tmp->next;
	}else{
		tmp->prev->next = tmp->next;
	}
	//if tmp next is not null
	if(tmp->next != NULL){
		//set tmps next prev to tmp prev 
		tmp->next->prev = tmp->prev;
	}
	//assign tmp_local to the ready list
	tmp_local = YKRdyList;
	//insertReady(tmp);	//will be ahead of idle task
	//while the ready list has a lower priority than the tmp passed in move tmp_local up in the list
	while(tmp_local->priority < tmp->priority){	
		tmp_local = tmp_local->next;
	}
	if(tmp_local->prev == NULL){	//we will insert the tcb pointed to by temp_local
		YKRdyList = tmp;
	}else{
		tmp_local->prev->next = tmp;	//rearranging the list
	}

	//now we will finish the tcb pointer clean up
	tmp->prev = tmp_local->prev;
	tmp->next = tmp_local;
	tmp_local->prev = tmp;

}


YKSEM* YKSemCreate(int value){
	int i;
	YKSEM* YKFreeSem;

	for (i = 0; i < MAX_SEM; i++) {	
		YKEnterMutex();
		if (YKSemArray[i].isTaken == 0) { // The semaphore is free to use
			YKFreeSem = &(YKSemArray[i]);
			YKFreeSem->isTaken = 1;
			YKExitMutex();
			YKFreeSem->value = value;
			return YKFreeSem;
		}
		YKExitMutex();
	}
	return YKFreeSem; // should never reach this if there are still free semaphores
}

void YKSemPend(YKSEM *semaphore) {
	TCBptr tmpTCB; //set up tmp TCBptrs
	YKEnterMutex();
	// check to see if semaphore is free
	if (semaphore->value-- > 0) { // first check if greater than 0, then dec
		YKExitMutex();
		return;
	}

	tmpTCB = YKRdyList;
	tmpTCB->sem = semaphore;
	removeReady();
	YKScheduler();
	YKExitMutex();
}

void YKSemPost(YKSEM *semaphore) {
	TCBptr tmpTCB; //set up tmp TCBptrs
	YKEnterMutex();
	// check to see if a task is waiting on this semaphore
	if (semaphore->value++ >= 0) { // first check if >= 0, then inc
		YKExitMutex();
		return;
	}

	tmpTCB = getSemBlockTask(semaphore);
	tmpTCB->sem = 0; // set address of sem to 0 meaning no longer waiting for sem
	removeBlocked(tmpTCB); // place tcb back into ready list

	if (YKNestingLevel == 0) { // if not inside an ISR
		YKScheduler();		
	}
	YKExitMutex();
}

// returns the highest priority blocked semaphore that is waiting
// on the passed in semaphore
TCBptr getSemBlockTask(YKSEM *semaphore) {
	int highestPriority = LOWEST_PRIORITY;
	TCBptr TCBFromList = YKBlockList;
	TCBptr TCBHighestPriority = YKBlockList;

	while(TCBFromList != NULL) {
		if (/*(TCBFromList->priority < highestPriority) &&*/ // if better priority
				TCBFromList->sem == semaphore) {	// and if waiting on sem
			highestPriority = TCBFromList->priority; // save the highest pri
			TCBHighestPriority = TCBFromList;	// save that TCB*
		}
		TCBFromList = TCBFromList->next;
	}
	return TCBHighestPriority;

}


YKQ* YKQCreate(void **start, unsigned int size){
	YKQ* NewQ = &YKQArray[YKQIndex++];		//the new MsgQ to be created
	NewQ->length = size;			//the length of the queue
	NewQ->MsgQ = start;				//this sets the MsgQ to the address of the starting **
	NewQ->full = 0;					//set full to zero to begin with
	NewQ->empty = 1;				//set empty to 1 to begin with
	NewQ->head = 0;					//set head to index zero
	NewQ->tail = 0;					//set tail to index zero 
	return NewQ;					//This will return the NewQ					
}

void *YKQPend(YKQ *queue) {
	void *message;		//message to be pended to
	YKEnterMutex();		//disable interrupts
	// check if queue is empty
	if (queue->empty) {
		YKRdyList = queueWaitListRemove(YKRdyList,YKCurrTask); //rearrange ready list by blocking currTask waiting for queue
		queue->waitList = queueWaitListInsert(queue->waitList, YKCurrTask);	//set the waitlist for the queue
		YKScheduler();		//call the scheduler
		// I think don't exitMutex yet
	}
	
	// remove the oldest element in the queue
	message = queue->MsgQ[queue->head];
	if(queue->head == (queue->length - 1)){	//if the head is at length minus one we are going to wrap the head around
		queue->head = 0;					//set head back to zero
	}else{
		queue->head++;						//else increment head
	}
	
	queue->full = 0;						//set full to zero meaning not full yet
	if(queue->tail == queue->head){			//if tail is equal to head
		queue->empty =1;					//the queue is empty
	}
	YKExitMutex();							//re-enable the interrupts
	return message;							//return the message queue
}

//posting to a message queue
int YKQPost(YKQ *queue, void *msg) {
	TCBptr old;		//tcb ptr used to point to old waitList for queue
	YKEnterMutex();	//disable interrupts
	// check if queue is full	
	if (queue->full) {
		YKExitMutex();	//re-enable interrupts
		return 0;		//return 0
	}

	//insert message into queue at tail index
	(queue->MsgQ[queue->tail]) = msg;
	if(queue->tail == (queue->length - 1)){		//if the tail is length minus 1
		queue->tail = 0;						//then wrap tail index back around
	}else{			
		queue->tail++;							//else increment tail index
	}
	queue->empty = 0;							//set empty to zero
	if(queue->tail == queue->head){				//if the tail is equal to head
		queue->full = 1;						//then set full to 1
	}	
	if(queue->waitList != NULL){				//if the waitList for the queue is not empty
		old = queue->waitList;					//set old to the current waitList
		queue->waitList = queueWaitListRemove(queue->waitList,old);	//rearrange waitList for the queue
		old->state = READY;		//set the old Tcb state to READY
		YKRdyList = queueWaitListInsert(YKRdyList,old);		//rearrange the ready list based on the old TCB that just became ready
		
	}
	if (YKNestingLevel == 0) { // if not inside an ISR
		YKScheduler();		//then call scheduler
	}
	YKExitMutex();		//re-enable interrupts
	return 1;			//return 1
}


//function to insert tcb into queue waitlist when block for waiting on a queue
TCBptr queueWaitListInsert(TCBptr front, TCBptr target){
	
	TCBptr tmp2;		//tmp TCB
	
	if(target == NULL){	//if target is null
		return front;	//return the front TCB
	}

	// put in list (idle task always at end) 
	tmp2 = front;	

	if(tmp2==NULL){	//if tmp is null
		target->next = NULL;	//rearrange needed next and prev pointers
		target->prev = NULL;
		front = target;
		return front;
	}
    while (tmp2->priority < target->priority){	//checking priority to rearrange list
    	if(tmp2->next != NULL)
			tmp2 = tmp2->next;
		else{
			tmp2->next = target;
			target->prev = tmp2;
			target->next = NULL;
			return front;
		}
    }
    if (tmp2->prev == NULL)		// insert before TCB pointed to by tmp2
		front = target;
    else
		tmp2->prev->next = target;
    
    target->prev = tmp2->prev;
    target->next = tmp2;
    tmp2->prev = target;

	return front;
}


//removes a tcb when no longer waiting for queue...will at least alwasy return idle task to be run
TCBptr queueWaitListRemove(TCBptr front, TCBptr target){
	if(target == NULL){
		return front;
	}

	if (target->prev == NULL)	// fix up suspended list 
		front = target->next;
    else
		target->prev->next = target->next;
    if (target->next != NULL)
		target->next->prev = target->prev;

	target->prev = NULL;
	target->next = NULL;

	return front;
}



YKEVENT *YKEventCreate(unsigned initVal){
	YKEVENT* event = &YKEventArray[idxNextAvailEvent++];
	event->waitList = NULL;
	event->value = initVal;
	return event;
}





unsigned YKEventPend(YKEVENT *event,unsigned eventMask, int waitMode){
	//disable interrupts
		YKEnterMutex();
	
		if(waitMode == EVENT_WAIT_ANY){
			//check to see if any of the same bits are set
			if(eventMask & event->value){
				return event->value;
			}
		}
		else{
			if(eventMask == event->value){
				return event->value;
			}
		}
	
		//if we get here, the conditions have not been met
	
		//update the ready list
		YKRdyList = queueWaitListRemove(YKRdyList, YKCurrTask);
	
		//update task state
		YKCurrTask->state = BLOCKED;
		YKCurrTask->evWtMode = waitMode;
		YKCurrTask->evWtValue = eventMask;
	
		event->waitList = queueWaitListInsert(event->waitList, YKCurrTask);
	
	
		//since we know that nesting level should be 0, we can
		//call scheduler without worrying too much about it
		YKScheduler();
	
		YKExitMutex();
	
		return event->value;

}



void YKEventSet(YKEVENT *event,unsigned eventMask){
		TCBptr currTCB;
		TCBptr nextTCB;
	
		YKEnterMutex();
	
		event->value = eventMask | event->value;
	
		currTCB = event->waitList;
		while(currTCB != NULL){
			//preserve the next TCB in the order
			nextTCB = currTCB->next;
			if(currTCB->evWtMode == EVENT_WAIT_ANY){
				if(currTCB->evWtValue & event->value){
					//remove from waitlist
					event->waitList = queueWaitListRemove(event->waitList, currTCB);
	
					currTCB->state = READY;
	
					//put on ready list
					YKRdyList = queueWaitListInsert(YKRdyList, currTCB);
				}
			}
			else{
				if(currTCB->evWtValue == event->value){
					event->waitList = queueWaitListRemove(event->waitList, currTCB);
	
					currTCB->state = READY;
	
					//put on ready list
					YKRdyList = queueWaitListInsert(YKRdyList, currTCB);
				}
			}
	
			//advance to the next TCB, if there are any more TCB's
			currTCB = nextTCB;
		}
	
	
		//call scheduler if in task code
		if(YKNestingLevel == 0){
			YKScheduler();
		}
	
		YKExitMutex();

}



void YKEventReset(YKEVENT *event,unsigned eventMask){
	eventMask = ~eventMask;
	event->value &= eventMask;
}









