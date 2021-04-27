#include <umps3/umps/libumps.h>

#include "initial.h"
#include "exceptions.h"
#include "vm_support.h"
#include "sys_support.h"

// TODO: Rimuovere define inutili

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)
#define PFNMASK 0x3FFFF000
#define PFNSHIFT 12
#define ASIDMASK 0x00000F60
#define GETASID(T) (T & ASIDMASK) >> ASIDSHIFT
#define GETVPN(T) (T & VPNMASK) >> VPNSHIFT
#define SETASID(TO, FROM) TO = (TO & ASIDMASK) | (FROM << ASIDSHIFT)
#define SETVPN(TO, FROM) TO = (TO & VPNMASK) | (FROM << VPNSHIFT)
#define SETPFN(TO, FROM) TO = (TO & PFNMASK) | (FROM << PFNSHIFT)

// TODO: Usarlo anche per le fasi precedenti?
#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON)

swap_t swapTable[FRAMENUMBER];
semaphore semSwapPool;


pteEntry_t* findEntry(int pageNumber) {
    return &(currentProcess->p_supportStruct->sup_privatePgTbl[pageNumber]);
}

// TODO: Il puntatore alla pteEntry può essere nullo?
int findReplacement() {
    static int currentReplacementIndex = 0;

    int i = 0;
    while((swapTable[(currentReplacementIndex + i) % FRAMENUMBER].sw_pte->pte_entryHI & VALIDON) && i < FRAMENUMBER)
        ;

    i = (i == FRAMENUMBER) ? 1 : i;

    return currentReplacementIndex = (currentReplacementIndex + i) % FRAMENUMBER;
}

// TODO: Qui deve prenderlo come puntatore o normale?
void updateTLB(pteEntry_t *updatedEntry){

    // Check if the updated TLB entry is cached in the TLB
    setENTRYHI(updatedEntry->pte_entryHI);
    TLBP();
    
    if ((getINDEX() & PRESENTFLAG) == CACHED) {
        // Update the TLB
        setENTRYLO(updatedEntry->pte_entryLO);
        TLBWI();
    }
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

    // Determine the ASID and the missing page number
    //int currentASID = GETASID(currentSupport->sup_exceptState[PGFAULTEXCEPT].entry_hi);
    int currentASID = currentSupport->sup_asid;
    int missingPageNumber = GETVPN(currentSupport->sup_exceptState[PGFAULTEXCEPT].entry_hi);
    
    // Pick a frame replacement by calling page replacement algorithm
    // TODO: è int?
    int selectedFrame = findReplacement();

    // Determine if frame i is occupied
    if (swapTable[selectedFrame].sw_pte->pte_entryHI & VALIDON) {
        // Occupied

        // Get the occupied ASID and page number
        int occupiedASID = swapTable[selectedFrame].sw_asid;
        int occupiedPageNumber = swapTable[selectedFrame].sw_pageNo;
        
        // Disable interrupts
        DISABLEINTERRUPTS;

        // Mark the occupied entry as not valid

        pteEntry_t *occupiedPageTable = swapTable[selectedFrame].sw_pte;
        // TODO: occupiedPageTable deve essere dereferenziato?
        occupiedPageTable[occupiedPageNumber].pte_entryLO &= ~VALIDON;

        // Update the TLB, if needed
        updateTLB(occupiedPageTable);

        // TODO: Update process x's backing store   

        // Re-enable interrupts
        ENABLEINTERRUPTS;
    }

    // TODO: Read the contents from the flash device

    // Update the swap pool table by setting the new ASID and page number
    swapTable[selectedFrame].sw_asid = currentASID;
    swapTable[selectedFrame].sw_pageNo = missingPageNumber;

    // TODO: Aggiornare anche il puntatore alla pteEntry

    DISABLEINTERRUPTS;
    
    // Update the process' page table
    currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO |= VALIDON;
    SETPFN(currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO, selectedFrame);

    // Update the TLB
    updateTLB(&(currentSupport->sup_privatePgTbl[missingPageNumber]));

    ENABLEINTERRUPTS;

    SYSCALL(VERHOGEN, &semSwapPool, 0, 0);

    // TODO: Controllare
    resume();
}

void uTLB_RefillHandler() {
    // Trova il page number
    // TLBR
    int pageNumber = (EXCSTATE->entry_hi & VPNMASK) >> VPNSHIFT;

    pteEntry_t *entry = findEntry(pageNumber);

    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);
    TLBWR();

    // Return control to the process
    resume();
}