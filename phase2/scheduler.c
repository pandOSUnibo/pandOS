#include <umps3/umps/libumps.h>

#include "pcb.h"

#include "initial.h"
#include "scheduler.h"

volatile cpu_t sliceStart;

/**
 * @brief Preemptive round-robin scheduler with time slice value of 5ms.
 * 
 */
void schedule() {
	if (emptyProcQ(readyQueue)) {
		// Job's done
		if (processCount == 0) {
			HALT();
		}

		// Wait state
		if (processCount > 0 && softBlockCount > 0) {
			// Enable interrupts and disable PLT
			unsigned int prevStatus = getSTATUS();
            setTIMER(MUSEC_TO_TICKS(MAXPLT));
            setSTATUS((prevStatus) | IECON | IMON);

			// Wait for a device interrupt
			WAIT();

			// Reset to previous status
			setSTATUS(prevStatus);
		}

		// Deadlock state
		if (processCount > 0 && softBlockCount == 0) {
			PANIC();
		}
	}

	// Get first ready process
	currentProcess = removeProcQ(&readyQueue);

	// Load 5ms on the PLT
	setTIMER(MUSEC_TO_TICKS(TIMESLICE));

	// Save the time slice beginning time
	STCK(sliceStart);

	// Load active processor state
	LDST(&(currentProcess->p_s));
}
