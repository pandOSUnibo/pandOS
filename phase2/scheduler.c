// preemptive round-robin with slice value of 5ms
// interrupt to generate clock

#include "initial.h"
#include "pcb.h"
#include "scheduler.h"
#include <umps3/umps/libumps.h>
#include "debug.h"

cpu_t sliceStart;

void schedule() {
    addokbuf("Entering scheduler.\n");
    if(emptyProcQ(readyQueue)) {

        // Job's done
        if(processCount == 0) {
            HALT();
        }
        // Wait state
        if(processCount > 0 && softBlockCount > 0) {
            addokbuf("Waiting for interrupts...\n");
            // Enable interrupts and disable PLT
            unsigned int prevStatus = getSTATUS();
            // TODO: Devo overridare la maschera?
            // TODO: Togliere?
            setTIMER(MUSEC_TO_TICKS(500000UL)); //TODO: Temporaneo e ho tolto la disattivazione
            // TODO: Controllare IMON
            setSTATUS((prevStatus) | IECON | IMON);

            // Wait for a device interrupt
            WAIT();

            // Reset to previous status
            setSTATUS(prevStatus);
        }

        // Deadlock state
        if(processCount > 0 && softBlockCount == 0) {
            addokbuf("PANIC: scheduler.c | DEADLOCK\n");
            PANIC();
        }
    }

    // Get first ready process
    currentProcess = removeProcQ(&readyQueue);

    // Load 5ms on the PLT
    setTIMER(MUSEC_TO_TICKS(5000UL));

    // Save the time slice beginning time
    STCK(sliceStart);

    // Load active processor state
    addokbuf("Leaving scheduler.\n");
    LDST(&(currentProcess->p_s));
}
