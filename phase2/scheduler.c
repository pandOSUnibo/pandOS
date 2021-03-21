// preemptive round-robin with slice value of 5ms
// interrupt to generate clock

#include "initial.h"
#include "pcb.h"
#include <umps3/umps/libumps.h>

#define MUSEC_TO_TICKS(T)         (T) * (*((cpu_t *) TIMESCALEADDR))

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern pcb_t *readyQueue;
extern pcb_t *currentProcess;

void schedule() {
    if(emptyProcQ(readyQueue)) {

        // Job's done
        if(processCount == 0) {
            HALT();
        }
        // Wait state
        if(processCount > 0 && softBlockCount > 0) {

            // Enable interrupts and disable PLT
            unsigned int prevStatus = getSTATUS();
            setSTATUS(prevStatus & ~TEBITON | IECON);

            // Wait for a device interrupt
            WAIT();

            // Reset to previous status
            setSTATUS(prevStatus);
        }

        // Deadlock state
        if(processCount > 0 && softBlockCount == 0) {
            PANIC();
        }
    }

    // Get first ready process
    currentProcess = removeProcQ(&readyQueue);

    // Load 5ms on the PLT
    setTIMER(MUSEC_TO_TICKS(5000UL));

    // Load active processor state
    LDST(&(currentProcess->p_s));
}
