#include <umps3/umps/libumps.h>

#include "initial.h"
#include "exceptions.h"
#include "vm_support.h"
#include "sys_support.h"

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)
#define PFNMASK 0x3FFFF000
#define PFNSHIFT 12
// Get PFN from an entry_low
#define GETPFN(T) (T & PFNMASK) >> PFNSHIFT
// Get VPN from an entry_hi
#define GETVPN(T) (T & VPNMASK) >> VPNSHIFT
#define SETPFN(TO, FROM) TO = (TO & PFNMASK) | (FROM << PFNSHIFT)

// TODO: Usarlo anche per le fasi precedenti?
#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON)

#define FLASHADDRSHIFT 8

swap_t swapTable[FRAMENUMBER];
semaphore semSwapPool;

typedef unsigned int flashaddr;

// TODO: Guardare ottimizzazioni

// TODO: Fare un typedef per il tipo register (=unsigned int)?

pteEntry_t* findEntry(int pageNumber) {
    return &(currentProcess->p_supportStruct->sup_privatePgTbl[pageNumber]);
}

int findReplacement() {
    static int currentReplacementIndex = 0;

    int i = 0;
    while((swapTable[(currentReplacementIndex + i) % FRAMENUMBER].sw_pte->pte_entryHI & VALIDON) && i < FRAMENUMBER)
        ++i;

    i = (i == FRAMENUMBER) ? 1 : i;

    return currentReplacementIndex = (currentReplacementIndex + i) % FRAMENUMBER;
}

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

void executeFlashAction(int deviceNumber, memaddr frameLocation, unsigned int command, support_t *currentSupport) {
    // Obtain the mutex on the device
    SYSCALL(PASSEREN, semMutexDevices[FLASHINT][deviceNumber], 0, 0);
    *((flashaddr *)DEVREG(FLASHINT, deviceNumber, DATA0)) = frameLocation;

    // Disabling interrupt doesn't interfere with SYS5, since SYSCALLS aren't
    // interrupts
    DISABLEINTERRUPTS;

    *((unsigned int *)DEVREG(FLASHINT, deviceNumber, COMMAND)) = command;
    // Wait for the device
    // The device ACK is handled by SYS5
    unsigned int deviceStatus = SYSCALL(IOWAIT, FLASHINT, deviceNumber, FALSE);

    ENABLEINTERRUPTS;

    // Release the mutex
    SYSCALL(VERHOGEN, semMutexDevices[FLASHINT][deviceNumber], 0, 0);

    if (deviceStatus != READY) {
        // Release the mutex on the swap pool semaphore
        SYSCALL(VERHOGEN, &semSwapPool, 0, 0);
        // Raise a trap
        trapExceptionHandler(currentSupport);
    }
}

void readFrameFromFlash(int deviceNumber, flashaddr flashLocation, memaddr frameLocation, support_t *currentSupport) {
    unsigned int command = FLASHREAD | (flashLocation << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, frameLocation, command, currentSupport);
}

void writeFrameToFlash(int deviceNumber, flashaddr flashLocation , memaddr frameLocation, support_t *currentSupport) {
    unsigned int command = FLASHWRITE | (flashLocation << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, frameLocation, command, currentSupport);
}

void uTLB_PageFaultHandler() {
    // Get Current Process's Support Structure
    support_t *currentSupport = SYSCALL(GETSUPPORTPTR, 0, 0, 0);

    // If it's a TLB-Modification exception, treat it as a program trap
    if((currentSupport->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE) >> CAUSESHIFT == 1){
        generalExceptionHandler();
    }

    // Gain mutual exclusion over the Swap Pool table
    SYSCALL(PASSEREN, &semSwapPool, 0, 0);

    // Determine the ASID and the missing page number
    int currentASID = currentSupport->sup_asid;
    int missingPageNumber = GETVPN(currentSupport->sup_exceptState[PGFAULTEXCEPT].entry_hi);
    
    // Pick a frame replacement by calling page replacement algorithm
    int selectedFrame = findReplacement();

    // Determine if frame i is occupied
    if (swapTable[selectedFrame].sw_pte->pte_entryLO & VALIDON) {
        // Occupied

        // Get the occupied ASID and page number
        int occupiedASID = swapTable[selectedFrame].sw_asid;
        int occupiedPageNumber = swapTable[selectedFrame].sw_pageNo;
        
        // Disable interrupts
        DISABLEINTERRUPTS;

        // Mark the occupied entry as not valid
        pteEntry_t *occupiedPageTable = swapTable[selectedFrame].sw_pte;
        occupiedPageTable->pte_entryLO &= ~VALIDON;

        // Update the TLB, if needed
        updateTLB(occupiedPageTable);

        // Update process x's backing store
        // TODO: quale device?? Ora metto occupiedASID-1 temporaneamente
        writeFrameToFlash(occupiedASID-1, occupiedPageNumber,  GETPFN(occupiedPageTable->pte_entryLO), currentSupport);

        // Re-enable interrupts
        ENABLEINTERRUPTS;
    }

    // Read the contents from the flash device
    readFrameFromFlash(currentASID - 1, missingPageNumber, GETPFN(swapTable[selectedFrame].sw_pte->pte_entryLO), currentSupport);

    // Update the swap pool table by setting the new ASID, page number and pointer to the process's page table entry
    swapTable[selectedFrame].sw_asid = currentASID;
    swapTable[selectedFrame].sw_pageNo = missingPageNumber;
    swapTable[selectedFrame].sw_pte = &(currentSupport->sup_privatePgTbl[missingPageNumber]);

    DISABLEINTERRUPTS;
    
    // Update the process' page table
    currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO |= VALIDON;
    SETPFN(currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO, selectedFrame);

    // Update the TLB
    updateTLB(&(currentSupport->sup_privatePgTbl[missingPageNumber]));

    ENABLEINTERRUPTS;

    SYSCALL(VERHOGEN, &semSwapPool, 0, 0);

    // Return control to the process by loading the processor state
    resume();
}

void uTLB_RefillHandler() {
    // Get the page number
    int pageNumber = GETVPN(EXCSTATE->entry_hi);

    pteEntry_t *entry = findEntry(pageNumber);

    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);
    TLBWR();

    // Return control to the process by loading the processor state
    resume();
}