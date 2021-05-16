#include "init_proc.h"
#include "support.h"
#include "sys_support.h"
#include "syscalls.h"
#include "vm_support.h"

#include "pandos_const.h"

#include <umps3/umps/libumps.h>



// TODO - Eliminare
// #define UNOCUPPIED (0x3F<<ASIDSHIFT)
// Semaphore used to wait the child process termination
semaphore masterSemaphore;
support_t *globSup; // TODO - da rimuovere
unsigned int debugPrivatePgTbl; // TODO: da rimuovere

void debugEntry(){}

void initialize() {
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


void test(void) {
    initialize();

    // Initialize processor state equal for all U-procs
    state_t p;
    p.pc_epc = UPROCSTARTADDR;
    p.reg_t9 = UPROCSTARTADDR;
    p.reg_sp = USERSTACKTOP;
    p.status = IEPON | IMON | TEBITON | USERPON;

    support_t *sup;
    for (int id = 1; id <= UPROCNUMBER; id++) {
        p.entry_hi = (id << ASIDSHIFT);
        sup = allocSupport();
        globSup = sup;
        // Initialize Support Structure for the i-th U-proc
        sup->sup_asid = id;
        sup->sup_exceptContext[PGFAULTEXCEPT].c_pc = (memaddr) &uTLB_PageFaultHandler; // TODO: metti uTLB_Handler
        sup->sup_exceptContext[GENERALEXCEPT].c_pc = (memaddr) &generalExceptionHandler; 
        sup->sup_exceptContext[PGFAULTEXCEPT].c_status = IEPON | IMON | TEBITON;
        sup->sup_exceptContext[GENERALEXCEPT].c_status = IEPON | IMON | TEBITON; // GeneralExceptionHandler
        sup->sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = (memaddr) &(sup->sup_stackGen[499]);
        sup->sup_exceptContext[GENERALEXCEPT].c_stackPtr = (memaddr) &(sup->sup_stackGen[499]);

        int row;
        for(row = 0; row<USERPGTBLSIZE-1; row++) {
            sup->sup_privatePgTbl[row].pte_entryHI = VPNBASE + (row << VPNSHIFT) + (id << ASIDSHIFT);
            //debugPrivatePgTbl = sup->sup_privatePgTbl[row].pte_entryHI;
            sup->sup_privatePgTbl[row].pte_entryLO = DIRTYON;
            debugPrivatePgTbl = sup->sup_privatePgTbl[row].pte_entryLO;
        }
        
        debugEntry();
        sup->sup_privatePgTbl[row].pte_entryHI = UPROCSTACKPG + (id << ASIDSHIFT);
        debugPrivatePgTbl = sup->sup_privatePgTbl[row].pte_entryHI;
        sup->sup_privatePgTbl[row].pte_entryLO = DIRTYON; // TODO perchè c'era DIRTYON | GLOBALON? Anche su riga 70
        // TODO: Call SYS1 on U-proc
        SYSCALL(CREATEPROCESS, (memaddr) &p, (memaddr)sup, 0);

    }

    // TODO: è sempre UPROCNUMBER?
    for (int i = 0; i < UPROCNUMBER; i++){
        SYSCALL(PASSEREN, (memaddr) &masterSemaphore, 0, 0);
    }
    termProcess();
}