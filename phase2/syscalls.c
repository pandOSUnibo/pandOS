#include <umps3/umps/libumps.h>

#include "pandos_const.h"

#include "asl.h"
#include "pcb.h"

#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "syscalls.h"


HIDDEN void termProcessRecursive(pcb_t *p);

void createProcess(state_t * statep, support_t * supportp) {
	pcb_t *newProc = allocPcb();
	unsigned int retValue = -1;

	if (newProc != NULL) {
		processCount++;
		newProc->p_supportStruct = supportp;
		newProc->p_s = *statep;
		insertChild(currentProcess, newProc);
		insertProcQ(&readyQueue, newProc);
		retValue = 0;
	}
	EXCSTATE->reg_v0 = retValue;
}

void termProcess() {
	outChild(currentProcess);
	termProcessRecursive(currentProcess);
	// currentProcess will be overwritten by the scheduler
	// but for good practice we remove the dangling reference
	currentProcess = NULL;

	// Pass control to the scheduler
	schedule();
}


/**
 * @brief Recursively terminates a process and
 * its children.
 * 
 * @param p Process to be terminated.
 */
HIDDEN void termProcessRecursive(pcb_t *p) {
	pcb_t *child;

	// Handle all children
	while ((child = removeChild(p)) != NULL) {
		outProcQ(&readyQueue, child);
		termProcessRecursive(child);
	}

	// Handle the process itself

	// A process is blocked on a device if the semaphore is
	// semIntTimer or an element of semDevices
	bool blockedOnDevice =
		(p->p_semAdd >= (int *) semDevices &&
		 p->p_semAdd <
		 ((int *) semDevices +
		  (sizeof(semaphore) * DEVICE_TYPES * DEVICE_INSTANCES)))
		|| (p->p_semAdd == (int *) &semIntTimer);

	// If the process is blocked on a user semaphore, remove it
	pcb_t *removedPcb = outBlocked(p);

	// For device processes, 
	if (!blockedOnDevice && removedPcb != NULL) {
		(*(p->p_semAdd))++;
	}

    // Deallocate the pcb and update the processes counter
	freePcb(p);
	processCount--;
}

void passeren(semaphore *semAdd) {
	(*semAdd)--;

	if (*semAdd < 0) {
		currentProcess->p_s = *EXCSTATE;
		currentProcess->p_time += elapsedTime();
		insertBlocked((int *) semAdd, currentProcess);
		currentProcess = NULL;
		schedule();
	}
}

pcb_t *verhogen(semaphore *semAdd) {
	(*semAdd)++;

	pcb_t *unblockedProcess = NULL;

	if (*semAdd <= 0) {
		// Process to be waked up
		// If there are no longer any processes in the queue
		// (because they were terminated), removeBlocked will
		// return NULL
		unblockedProcess = removeBlocked(semAdd);
		if (unblockedProcess != NULL) {
			insertProcQ(&readyQueue, unblockedProcess);
		}
	}
	return unblockedProcess;
}

void ioWait(int intlNo, int dNum, bool waitForTermRead) {
	// Save the process state
	volatile state_t exceptionState = *EXCSTATE;
	currentProcess->p_s = exceptionState;
	softBlockCount++;
	// Select the correct semaphore
	switch (intlNo) {
	case DISKINT:
	case FLASHINT:
	case NETWINT:
	case PRNTINT:
		passeren(&semDevices[intlNo - 3][dNum]);
		break;
	case TERMINT:
		if (waitForTermRead)
			passeren(&semDevices[4][dNum]);
		else
			passeren(&semDevices[5][dNum]);
		break;
	default:
		// Invalid interrupt line: kill
		termProcess();
		break;
	}
}

void getTime(cpu_t *resultAddress) {
	*resultAddress = currentProcess->p_time + elapsedTime();
}

void clockWait() {
	softBlockCount++;
	passeren(&semIntTimer);
}

void getSupportPtr(support_t **resultAddress) {
	*resultAddress = currentProcess->p_supportStruct;
}
