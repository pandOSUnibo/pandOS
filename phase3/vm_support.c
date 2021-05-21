#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>

#include "exceptions.h"
#include "init_proc.h"
#include "initial.h"
#include "sys_support.h"
#include "vm_support.h"


// Importante: POOLSTART deve essere un multiplo di 4K
#define POOLSTART (RAMSTART + (32 * PAGESIZE))
#define POOLEND (POOLSTART + FRAMENUMBER * PAGESIZE)
#define PFNSHIFT 12
#define PFNMASK 0xFFFFF000

#define STACKPG 31

// Get PFN from an entry_low
#define GETPFN(T) ((T - POOLSTART) >> PFNSHIFT)
// Get VPN frame number from an entry_hi
#define GETVPN(T) ((UPROCSTACKPG <= T && T <= USERSTACKTOP) ? STACKPG : ((T - VPNBASE) >> VPNSHIFT))
#define SETPFN(TO, N) TO = (TO & ~PFNMASK) | ((N << PFNSHIFT) + POOLSTART)

// TODO: Usarlo anche per le fasi precedenti?
#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON | IMON)

#define FLASHADDRSHIFT 8

#define HEADERPAGE 0
#define HEADERTEXTSIZE 0x0014
#define PAGESHIFT 12
#define FRAMETOADDRESS(T) ((T << PAGESHIFT) + POOLSTART)

HIDDEN swap_t swapTable[FRAMENUMBER];

/**
 * @brief Semaphore used to regulate access to the swap pool.
 * 
 * @remark This semaphore should always be accessed with
 * SYSCALL(...).
 */
HIDDEN semaphore semSwapPool;

int dataPages[UPROCNUMBER];


void initSwapStructs(){
    // Semaphore initialization
    for (int i = 0; i < DEVICE_TYPES; i++){
        for (int j = 0; j < DEVICE_INSTANCES; j++){
            semMutexDevices[i][j] = 1;
        }       
    }
    semSwapPool = 1;
    masterSemaphore = 0;
    initSupport();

    // Swap table initialization
    for (int i = 0; i < FRAMENUMBER; i++){
        swapTable[i].sw_asid = NOPROC;
    }
}

// TODO: Guardare ottimizzazioni

/**
 * @brief Finds the matching entry for a page number.
 * 
 * @param pageNumber User process page number.
 * @return Pointer to the corresponding entry.
 */
pteEntry_t* findEntry(unsigned int pageNumber) {
    return &(currentProcess->p_supportStruct->sup_privatePgTbl[pageNumber]);
}

// TODO - possibile soluzione al loop?
// Funzione che ripristina lo stato salvato nella support structure
// e non quello nella bios data page
void resumeVM(support_t *currentSupport){
    LDST((state_t *) &(currentSupport->sup_exceptState[PGFAULTEXCEPT]));
}



/**
 * @brief Finds the index of a frame that will contain the
 * new page.
 * 
 * @return The index of a frame that will contain the new page.
 */
int findReplacement() {
    static int currentReplacementIndex = 0;

    int i = 0;
    while((swapTable[(currentReplacementIndex + i) % FRAMENUMBER].sw_pte->pte_entryLO & VALIDON) && (i < FRAMENUMBER))
        ++i;

    // If all frames are occupied, pick currentReplacementIndex + 1
    i = (i == FRAMENUMBER) ? 1 : i;
    
    return currentReplacementIndex = (currentReplacementIndex + i) % FRAMENUMBER;
}

/**
 * @brief Updates the TLB.
 * 
 * @param updatedEntry Pointer to the entry with the new TLB values.
 */
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

/**
 * @brief Executes a command (read or write) for a flash
 * device.
 * 
 * @param deviceNumber Index of the device in [0, DEVICE_INSTANCES).
 * @param primaryPage Index of the page in primary memory.
 * @param command Command to be issued.
 * @param currentSupport Pointer to the support structure of the current
 * process.
 * 
 * @remark DEVICE_INSTANCES is defined in pandos_const.h.
 */
void executeFlashAction(int deviceNumber, unsigned int primaryPage, unsigned int command, support_t *currentSupport) {
    // Debug
    if (deviceNumber < 0 || deviceNumber > 7) {
        PANIC();
    }

    // Obtain the mutex on the device
    memaddr primaryAddress = (primaryPage << PFNSHIFT) + POOLSTART ;
    SYSCALL(PASSEREN, (memaddr) &semMutexDevices[FLASHSEM][deviceNumber], 0, 0);
    dtpreg_t *flashRegister = (dtpreg_t *) DEV_REG_ADDR(FLASHINT, deviceNumber);
    flashRegister->data0 = primaryAddress;

    // Disabling interrupt doesn't interfere with SYS5, since SYSCALLS aren't
    // interrupts
    DISABLEINTERRUPTS;
    
    flashRegister->command = command;
    // Wait for the device
    // The device ACK is handled by SYS5
    unsigned int deviceStatus = SYSCALL(IOWAIT, FLASHINT, deviceNumber, FALSE);

    ENABLEINTERRUPTS;
    // Release the mutex
    SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[FLASHSEM][deviceNumber], 0, 0);

    if (deviceStatus != READY) {
        // Release the mutex on the swap pool semaphore
        SYSCALL(VERHOGEN, (memaddr) &semSwapPool, 0, 0);
        // Raise a trap
        trapExceptionHandler(currentSupport);
    }
}

/**
 * @brief Reads a frame from a flash device and stores it in
 * primary memory.
 * 
 * @param deviceNumber Index of the device in [0, DEVICE_INSTANCES).
 * @param flashBlock Index of the block in the flash device.
 * @param primaryPage Index of the page in primary memory.
 * @param currentSupport Pointer to the support structure of the current
 * process.
 * 
 * @remark DEVICE_INSTANCES is defined in pandos_const.h.
 */
void readFrameFromFlash(int deviceNumber, unsigned int flashBlock, unsigned int primaryPage, support_t *currentSupport) {
    unsigned int command = FLASHREAD | (flashBlock << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, primaryPage, command, currentSupport);
}

/**
 * @brief Reads a frame from primary memory and stores it in
 * a flash device.
 * 
 * @param deviceNumber Index of the device in [0, DEVICE_INSTANCES).
 * @param flashBlock Index of the block in the flash device.
 * @param primaryPage Index of the page in primary memory.
 * @param currentSupport Pointer to the support structure of the current
 * process.
 * 
 * @remark DEVICE_INSTANCES is defined in pandos_const.h.
 */
void writeFrameToFlash(int deviceNumber, unsigned int flashBlock, unsigned int primaryPage, support_t *currentSupport) {
    unsigned int command = FLASHWRITE | (flashBlock << FLASHADDRSHIFT);
    executeFlashAction(deviceNumber, primaryPage, command, currentSupport);
}


unsigned int debugOcc;
unsigned int currAsid;
unsigned int inPage;
unsigned int outPage;
swap_t table;
unsigned int debugPage;

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
    // TODO - trovare una gestione più carina per la pagina dello stack.
    int missingPageNumber = GETVPN(currentSupport->sup_exceptState[PGFAULTEXCEPT].entry_hi);
    // Pick a frame replacement by calling page replacement algorithm
    int selectedFrame = findReplacement();

    // TODO - debug
    currAsid = currentASID;
    outPage = selectedFrame;
    inPage = missingPageNumber;
    table = swapTable[0];

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
        debugPage = occupiedPageTable->pte_entryLO;
        // Update the TLB, if needed
        updateTLB(occupiedPageTable);

        // Re-enable interrupts
        ENABLEINTERRUPTS;
        // Update process x's backing store
        // TODO aggiungere controllo sul dirty bit
        if(occupiedPageTable->pte_entryLO & DIRTYON){
            debugOcc = swapTable[selectedFrame].sw_pageNo;
            writeFrameToFlash(occupiedASID-1, occupiedPageNumber, selectedFrame, currentSupport);
        }

    }

    // Read the contents from the flash device
    readFrameFromFlash(currentASID - 1, missingPageNumber, selectedFrame, currentSupport);
    // Update the swap pool table by setting the new ASID, page number and pointer to the process's page table entry
    swapTable[selectedFrame].sw_asid = currentASID;
    swapTable[selectedFrame].sw_pageNo = missingPageNumber;
    swapTable[selectedFrame].sw_pte = &(currentSupport->sup_privatePgTbl[missingPageNumber]);

    // If the page contains header information, use it to identify
    // the first data page
    if (missingPageNumber != HEADERPAGE || dataPages[currentASID - 1] == UNKNOWNDATAPAGE) {
        memaddr headerAddress = FRAMETOADDRESS(selectedFrame);
        memaddr textSize = *((memaddr *)(headerAddress + HEADERTEXTSIZE));
        dataPages[currentASID - 1] = textSize >> PAGESHIFT;
    }

    DISABLEINTERRUPTS;
    
    // Update the process' page table
    currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO |= VALIDON;
    SETPFN(currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO, selectedFrame);

    // If the page is a text page, mark it as not dirty
    if (dataPages[currentASID - 1] != UNKNOWNDATAPAGE && missingPageNumber < dataPages[currentASID - 1]) {
        currentSupport->sup_privatePgTbl[missingPageNumber].pte_entryLO &= ~DIRTYON;
    }

    // Update the TLB
    updateTLB(&(currentSupport->sup_privatePgTbl[missingPageNumber]));

    ENABLEINTERRUPTS;
    
    SYSCALL(VERHOGEN, (memaddr) &semSwapPool, 0, 0);

    // Return control to the process by loading the processor state
    // Qui fa un loop, mettere il bp su A2break e notare come
    // vengano sempre ripetute queste due funzioni ()
    resumeVM(currentSupport);
}

/**
 * @brief Handles TLB Refill exceptions.
 */
void uTLB_RefillHandler() {
    // Get the page number
    volatile unsigned int pageNumber;
    pageNumber = GETVPN(EXCSTATE->entry_hi);

    pteEntry_t *entry = findEntry(pageNumber);

    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);
    TLBWR();

    // Return control to the process by loading the processor state
    resume();
}