#include <umps3/umps/libumps.h>

#include "initial.h"
#include "exceptions.h"
#include "vm_support.h"
#include "sys_support.h"

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)
#define PFNMASK 0x3FFFF000
#define PFNSHIFT 12
#define GETVPN(T) (T & VPNMASK) >> VPNSHIFT
#define SETPFN(TO, FROM) TO = (TO & PFNMASK) | (FROM << PFNSHIFT)

// TODO: Usarlo anche per le fasi precedenti?
#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON)

#define FLASHADDRSHIFT 8

swap_t swapTable[FRAMENUMBER];
semaphore semSwapPool;

typedef unsigned int flashaddr;

// TODO: Fare un typedef per il tipo register (=unsigned int)?

pteEntry_t* findEntry(int pageNumber) {
    return &(currentProcess->p_supportStruct->sup_privatePgTbl[pageNumber]);
}

int findReplacement() {
    static int currentReplacementIndex = 0;

    int i = 0;
    while((swapTable[(currentReplacementIndex + i) % FRAMENUMBER].sw_pte->pte_entryHI & VALIDON) && i < FRAMENUMBER)
        ;

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

void executeFlashAction(int deviceNumber, flashaddr flashLocation, unsigned int command) {
    // Obtain the mutex on the device
    // TODO: è l'indice corretto?
    SYSCALL(PASSEREN, semMutexDevices[FLASHINT][deviceNumber], 0, 0);
    *((flashaddr *)DEVREG(FLASHINT, deviceNumber, DATA0)) = flashLocation;

    // TODO: pandOS dice esplicitamente di disattivare gli interrupts e riattivarli dopo
    // la SYSCALL, ma che succede se si fa una syscall con gli interrupts disabilitati?
    DISABLEINTERRUPTS;

    *((unsigned int *)DEVREG(FLASHINT, deviceNumber, DATA0)) = command;
    // Wait for the device
    // The device ACK is handled by SYS5
    unsigned int deviceStatus = SYSCALL(IOWAIT, FLASHINT, deviceNumber, FALSE);

    ENABLEINTERRUPTS;

    // Release the mutex
    SYSCALL(VERHOGEN, semMutexDevices[FLASHINT][deviceNumber], 0, 0);

    if (deviceStatus != READY) {
        // TODO: Trap
    }
}

void readFrameFromFlash(int deviceNumber, flashaddr flashLocation, memaddr frameLocation) {
    unsigned int command = FLASHREAD | (flashLocation << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, flashLocation, command);
}

void writeFrameToFlash(int deviceNumber, flashaddr flashLocation, memaddr frameLocation) {
    unsigned int command = FLASHWRITE | (flashLocation << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, flashLocation, command);
}

void uTLB_PageFaultHandler() {
    // Get Current Process's Support Structure
    support_t *currentSupport = SYSCALL(GETSUPPORTPTR, 0, 0, 0);

    // TODO: dobbiamo controllare che currentSupport non sia NULL oppure no?
    // Secondo me no, la support structure viene creata quando si inizializza il processo,
    // anche se la sezione 4.8 parla di potenziali problemi dovuti a supportStructure NULL (anche se
    // fa riferimento a un altro contesto) --Sam

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
    if (swapTable[selectedFrame].sw_pte->pte_entryHI & VALIDON) {
        // Occupied

        // Get the occupied ASID and page number
        int occupiedASID = swapTable[selectedFrame].sw_asid;
        int occupiedPageNumber = swapTable[selectedFrame].sw_pageNo;
        
        // Disable interrupts
        DISABLEINTERRUPTS;

        // Mark the occupied entry as not valid

        pteEntry_t *occupiedPageTable = swapTable[selectedFrame].sw_pte;
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

    // TODO: pandOS non dice che bisogna aggiornare anche sw_pte, ma altrimenti rimarrebbe outdated
    swapTable[selectedFrame].sw_pte = &(currentSupport->sup_privatePgTbl[missingPageNumber]);

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