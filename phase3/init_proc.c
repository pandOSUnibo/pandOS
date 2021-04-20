#include "init_proc.h"
#include "sys_support.h"
#include "vm_support.h"

// TODO - Usare NOPROC oppure farlo custom
#define UNOCUPPIED (0x3F<<ASIDSHIFT)
#define UPROCNUMBER UPROCMAX

// Semaphore used for wait the child process to end 
semaphore masterSemaphore;

//TODO - inserire in un file separato la gestione delle support structure
HIDDEN support_t support_table[UPROCNUMBER];
// Pointers to the free Support Structure. A dummy node is present.
HIDDEN support_t *freeSupport[UPROCNUMBER+1];
// Pointer to the first block that can store a new free one.
HIDDEN support_t **stackSup;

// TODO - sistemare la gestione della support free list

/**
 * @brief Insert in the free list the structure passed.
 * 
 * @param support Pointer to the new free support structure.
 */
void deallocSupport(support_t *support){
    *stackSup = support;
    stackSup = (stackSup + sizeof(support_t*));
}

/**
 * @brief Allocates a new support structure.
 * 
 * @return The allocated support structure. Returns NULL if memory is not available.
 */
support_t* allocSupport() {
    support_t *supp = NULL;
    if (stackSup != freeSupport){
        stackSup = (stackSup - sizeof(support_t*));
        supp = stackSup;
    }
    return supp;
}

/**
 * @brief Initialize the support structures
 * 
 */
void initSupport() {
    stackSup = freeSupport;
    int i ;
    for (i = 0; i < UPROCNUMBER; i++){
        deallocSupport(&support_table[i]);
    }
}

void initialize() {
    // Semaphore initialization
    for (int i = 0; i < DEVICE_TYPES; i++){
        for (int j = 0; i < DEVICE_INSTANCES; j++){
            semMutexDevices[i][j] = 1;
        }       
    }
    semSwapPool = 1;
    masterSemaphore = 0;

    initSupport();

    // Swap table initialization
    for (int i = 0; i < FRAMENUMBER; i++){
        swapTable[i].pte_entryHI = UNOCUPPIED;
    }
    
}

int test(void) {
    initialize();
    state_t p;
    p.pc_epc = UPROCSTARTADDR;
    p.reg_t9 = UPROCSTARTADDR;
    p.reg_sp = USERSTACKTOP;
    p.status = IEPON | IMON | TEBITON | USERPON;
    for (int i = 1; i <= UPROCNUMBER; i++){
        p.entry_hi = (i<<ASIDSHIFT);
    }
     
}