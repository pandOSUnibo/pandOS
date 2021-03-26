
#include "asl.h"
#include "pcb.h"
#include "initial.h"
#include "exceptions.h"
#include "scheduler.h"
#include <umps3/umps/libumps.h>

#include "debug.h"

unsigned int processCount;
unsigned int softBlockCount;
pcb_t *readyQueue;
pcb_t *currentProcess;

// TODO: softBlockedCount

semaphore semDevices[DEVICE_TYPES][DEVICE_INSTANCES];
semaphore semIntTimer;

// TODO: Mettere quanto più possibile HIDDEN

// TODO: Trovare dove va messo
extern void test();
extern void uTLB_RefillHandler();

int main(void) {
    // Initialize the Pass Up Vector
    passupvector_t *passUpVector = (passupvector_t*) PASSVEC_LOCATION;
    passUpVector->tlb_refill_handler = (memaddr) &uTLB_RefillHandler;
    passUpVector->tlb_refill_stackPtr = (memaddr) TLBSP_START;
    passUpVector->exception_handler = (memaddr) exceptionHandler;
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
    LDIT(100000UL);


    // Instantiate first process
    // allocPcb sets all the process fields to their default value
    pcb_t *process = allocPcb();
    processCount++;
    // TODO: Check IMON
    process->p_s.status = IEPON | TEBITON | IMON;
    RAMTOP(process->p_s.reg_sp);
    
    process->p_s.pc_epc = (memaddr) &test;
    process->p_s.reg_t9 = (memaddr) &test;

    insertProcQ(&readyQueue, process);

    // Call the scheduler
    schedule();
    return 1;
}