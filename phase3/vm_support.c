#include <umps3/umps/libumps.h>

#include "initial.h"
#include "exceptions.h"
#include "vm_support.h"

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
    // TODO: Gestione page fault
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