#include <umps3/umps/libumps.h>

#include "initial.h"
#include "exceptions.h"
#include "vm_support.h"
#include "sys_support.h"

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)
#define ASIDMASK 

pteEntry_t swapTable[FRAMENUMBER];
semaphore semSwapPool;


//TODO: * a destra?
pteEntry_t* findEntry(int pageNumber) {
    return &(currentProcess->p_supportStruct->sup_privatePgTbl[pageNumber]);
}

void uTLB_PageFaultHandler() {
    // Get Current Process's Support Structure
    support_t *currentSupport = SYSCALL(GETSUPPORTPTR, 0, 0, 0);

    // TODO: dobbiamo controllare che currentSupport non sia NULL oppure no?

    // If it's a TLB-Modification exception, treat it as a program trap
    if((currentSupport->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE) >> CAUSESHIFT == 1){
        generalExceptionHandler();
    }

    // Gain mutual exclusion over the Swap Pool table
    SYSCALL(PASSEREN, &semSwapPool, 0, 0);

    // Determine the missing page number
    int missingPageNumber = (currentSupport->sup_exceptState[PGFAULTEXCEPT].entry_hi & GETPAGENO) >> VPNSHIFT;
    
    // Pick a frame replacement by calling page replacement algorithm
    // TODO: è int?
    int selectedFrame = ...;

    // Determine if frame i is occupied
    if (swapTable[selectedFrame].pte_entryHI & VALIDON) {
        // Occupied

        // Get the current page number and ASID
        int occupiedPageNumber = (swapTable[selectedFrame].pte_entryHI & GETPAGENO) >> VPNSHIFT;
        int occupiedASID = (swapTable[selectedFrame].pte_entryHI & ASIDMASK) >> ASIDSHIFT;
        
        // Disable interrupts
        setSTATUS(getSTATUS() & (~IECON));
        
        // Mark the current entry as not valid
        currentSupport->sup_privatePgTbl[occupiedPageNumber].pte_entryLO &= ~VALIDON;

        // TODO: Update the TLB
        TLBP()

        // Re-enable interrupts
        setSTATUS(getSTATUS() | IECON);
    }

    // 4

    SYSCALL(VERHOGEN, &semSwapPool, 0, 0);
}

void uTLB_RefillHandler() {
    // Trova il page number
    // TLBR
    int pageNumber = (EXCSTATE->entry_hi & GETPAGENO) >> VPNSHIFT;

    pteEntry_t *entry = findEntry(pageNumber);

    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);
    TLBWR();

    // Return control to the process
    resume();
}