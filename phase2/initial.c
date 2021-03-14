#include "asl.h"
#include "pcb.h"
#include "initial.h"
#include <umps3/umps/libumps.h>

unsigned int processCount;
unsigned int softBlockCount;
pcb_t *readyQueue;
pcb_t *currentProcess;

// TODO: Dispositivi

// TODO: allocPcb non inizializza p_s

// TODO: Trovare dove va messo
extern void test;
extern void uTLB_RefillHandler;

int main(void) {
    // Initialize the Pass Up Vector
    passupvector_t *passUpVector = (passupvector_t*) PASSVEC_LOCATION;
    passUpVector->tlb_refill_handler = (memaddr) &uTLB_RefillHandler;
    passUpVector->tlb_refill_stackPtr = (memaddr) TLBSP_START;
    passUpVector->exception_handler = (memaddr)  fooBar;
    passUpVector->tlb_refill_stackPtr = (memaddr) EXCSP_START;

    // Initialize the Level 2 structures
    initPcbs();
    initASL();

    // Initialize global variables
    processCount = 0;
    softBlockCount = 0;
    readyQueue = mkEmptyProcQ();
    currentProcess = NULL;

    // TODO: Load timer

    // Instantiate first process
    // allocPcb sets all the process fields to their default value
    pcb_t *process = allocPcb();
    processCount++;
    process->p_s.status = IEPON | TEBITON;
    // TODO: missing RAMBASESIZE declaration
    RAMTOP(process->p_s.reg_sp);
    
    process->p_s.pc_epc = (memaddr) &test;
    process->p_s.reg_t9 = (memaddr) &test;

    // TODO: call the scheduler
}