#include "asl.h"
#include "pcb.h"
#include "initial.h"
#include "scheduler.h"
#include <umps3/umps/libumps.h>

unsigned int processCount;
unsigned int softBlockCount;
pcb_t *readyQueue;
pcb_t *currentProcess;

SEMAPHORE semDisk[DEVICE_ISTANCES];
SEMAPHORE semFlash[DEVICE_ISTANCES];
SEMAPHORE semNetwork[DEVICE_ISTANCES];
SEMAPHORE semPrinter[DEVICE_ISTANCES];
SEMAPHORE semTerminalTrans[DEVICE_ISTANCES];
SEMAPHORE semTerminalRecv[DEVICE_ISTANCES];
SEMAPHORE semIntTimer;

// TODO: Verificare se il modo di dichiarare le variabili globali è corretto (cursed)

// TODO: allocPcb non inizializza p_s

// TODO: Trovare dove va messo
extern void test();
extern void uTLB_RefillHandler();

int main(void) {
    // Initialize the Pass Up Vector
    passupvector_t *passUpVector = (passupvector_t*) PASSVEC_LOCATION;
    passUpVector->tlb_refill_handler = (memaddr) &uTLB_RefillHandler;
    passUpVector->tlb_refill_stackPtr = (memaddr) TLBSP_START;
    // TODO: Capire cosa assegnare
    // passUpVector->exception_handler = (memaddr)  fooBar;
    passUpVector->tlb_refill_stackPtr = (memaddr) EXCSP_START;

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
    process->p_s.status = IEPON | TEBITON;
    RAMTOP(process->p_s.reg_sp);
    
    process->p_s.pc_epc = (memaddr) &test;
    process->p_s.reg_t9 = (memaddr) &test;

    insertProcQ(&readyQueue, process);

    schedule();
    return 1;
}