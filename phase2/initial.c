#include <umps3/umps/libumps.h>

#include "asl.h"
#include "pcb.h"

#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"

unsigned int processCount;
unsigned int softBlockCount;
pcb_t *readyQueue;
pcb_t *currentProcess;

semaphore semDevices[DEVICE_TYPES][DEVICE_INSTANCES];
semaphore semIntTimer;

extern void test();
extern void uTLB_RefillHandler();

int main(void) {
	// Initialize the Pass Up Vector
	passupvector_t *passUpVector = (passupvector_t *) PASSVEC_LOCATION;

	passUpVector->tlb_refill_handler = (memaddr) &uTLB_RefillHandler;
	passUpVector->tlb_refill_stackPtr = (memaddr) TLBSP_START;
	passUpVector->exception_handler = (memaddr) &exceptionHandler;
	passUpVector->exception_stackPtr = (memaddr) EXCSP_START;

	// Initialize the Level 2 structures
	initPcbs();
	initASL();

	// Initialize global variables
	readyQueue = mkEmptyProcQ();
	currentProcess = NULL;

	// processCount, softBlockCount and device semaphores are
	// automatically initialized at compile time

	// Load Interval Timer
	LDIT(INTIMER);


	// Instantiate first process
	// allocPcb sets all the process fields to their default value
	pcb_t *process = allocPcb();

	processCount++;
	process->p_s.status = IEPON | TEBITON | IMON;
	RAMTOP(process->p_s.reg_sp);

	process->p_s.pc_epc = (memaddr) &test;
	process->p_s.reg_t9 = (memaddr) &test;

	insertProcQ(&readyQueue, process);

	// Call the scheduler
	schedule();
	return 1;
}
