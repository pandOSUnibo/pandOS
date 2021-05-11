#include <umps3/umps/libumps.h>

#include "initial.h"
#include "exceptions.h"

#include "init_proc.h"
#include "vm_support.h"
#include "sys_support.h"

#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)
#define PFNMASK 0xFFFFF000
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

unsigned int address;

int AReplacementFound = 1000; //TODO: Rimuovere 

// TODO: Guardare ottimizzazioni

// TODO: Fare un typedef per il tipo register (=unsigned int)?

pteEntry_t* findEntry(unsigned int pageNumber) {
    return &(currentProcess->p_supportStruct->sup_privatePgTbl[pageNumber]);
}

// TODO - possibile soluzione al loop?
// Funzione che ripristina lo stato salvato nella support structure
// e non quello nella bios data page
void resumeVM(support_t *currentSupport){
    LDST((state_t *) &(currentSupport->sup_exceptState[PGFAULTEXCEPT]));
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

void A2break(){

}

void executeFlashAction(int deviceNumber, unsigned int primaryBlock, unsigned int command, support_t *currentSupport) {
    // Obtain the mutex on the device
    memaddr primaryAddress = (primaryBlock << PFNSHIFT) + POOLSTART;
    SYSCALL(PASSEREN, (memaddr) &semMutexDevices[FLASHINT][deviceNumber], 0, 0);
    *((unsigned int *)DEVREG(FLASHINT, deviceNumber, DATA0)) = primaryAddress;

    // Disabling interrupt doesn't interfere with SYS5, since SYSCALLS aren't
    // interrupts
    DISABLEINTERRUPTS;
    
    *((unsigned int *)DEVREG(FLASHINT, deviceNumber, COMMAND)) = command;
    address = primaryAddress;
    // Wait for the device
    // The device ACK is handled by SYS5
    unsigned int deviceStatus = SYSCALL(IOWAIT, FLASHINT, deviceNumber, FALSE);

    ENABLEINTERRUPTS;
    // Release the mutex
    SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[FLASHINT][deviceNumber], 0, 0);

    if (deviceStatus != READY) {
        // Release the mutex on the swap pool semaphore
        SYSCALL(VERHOGEN, (memaddr) &semSwapPool, 0, 0);
        // Raise a trap
        trapExceptionHandler(currentSupport);
    }
}

void readFrameFromFlash(int deviceNumber, unsigned int flashBlock, unsigned int primaryBlock, support_t *currentSupport) {
    unsigned int command = FLASHREAD | (flashBlock << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, primaryBlock, command, currentSupport);
}

void writeFrameToFlash(int deviceNumber, unsigned int flashBlock, unsigned int primaryBlock, support_t *currentSupport) {
    unsigned int command = FLASHWRITE | (flashBlock << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, primaryBlock, command, currentSupport);
}

void uTLB_PageFaultHandler() {
    // Get Current Process's Support Structure
    support_t *currentSupport = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);

    // If it's a TLB-Modification exception, treat it as a program trap
    if((currentSupport->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE) >> CAUSESHIFT == 1){
        generalExceptionHandler();
    }

    // Gain mutual exclusion over the Swap Pool table
    SYSCALL(PASSEREN, (memaddr) &semSwapPool, 0, 0);

    // Determine the ASID and the missing page number
    int currentASID = currentSupport->sup_asid;
    int missingPageNumber = GETVPN(currentSupport->sup_exceptState[PGFAULTEXCEPT].entry_hi);
    
    // Pick a frame replacement by calling page replacement algorithm
    int selectedFrame = findReplacement();
    AReplacementFound = selectedFrame;
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
        writeFrameToFlash(occupiedASID-1, occupiedPageNumber, GETPFN(occupiedPageTable->pte_entryLO), currentSupport);

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
    SYSCALL(VERHOGEN, (memaddr) &semSwapPool, 0, 0);
    A2break();    
    // Return control to the process by loading the processor state
    // Qui fa un loop, mettere il bp su A2break e notare come
    // vengano sempre ripetute queste due funzioni ()
    //resume();
    resumeVM(currentSupport); // TODO - possibile soluzione
}

unsigned int debugEntryLo;

void uTLB_RefillHandler() {
    // Get the page number
    unsigned int pageNumber = GETVPN(EXCSTATE->entry_hi);

    pteEntry_t *entry = findEntry(pageNumber);

    debugEntryLo = entry->pte_entryLO;

    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);
    TLBWR();

    // Return control to the process by loading the processor state
    resume();
}