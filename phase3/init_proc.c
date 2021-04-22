#include "init_proc.h"
#include "support.h"
#include "sys_support.h"
#include "syscalls.h"
#include "vm_support.h"

#include <umps3/umps/libumps.h>

#define VPNBASE 0x80000000
#define UPROCSTACKPG 0xBFFFF000

// TODO - Usare NOPROC oppure farlo custom
#define UNOCUPPIED (0x3F<<ASIDSHIFT)

// Semaphore used to wait the child process termination
semaphore masterSemaphore;

//TODO - inserire in un file separato la gestione delle support structure


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

    // Initialize processor state equal for all U-procs
    state_t p;
    p.pc_epc = UPROCSTARTADDR;
    p.reg_t9 = UPROCSTARTADDR;
    p.reg_sp = USERSTACKTOP;
    p.status = IEPON | IMON | TEBITON | USERPON;

    support_t *sup;
    sup = allocSupport();
    for (int id = 1; id <= UPROCNUMBER; id++) {
        p.entry_hi = (id << ASIDSHIFT);

        // Initialize Support Structure for the i-th U-proc
        sup->sup_asid = id;
        sup->sup_exceptContext[PGFAULTEXCEPT].c_pc = uTLB_PageFaultHandler; // TODO: metti uTLB_Handler
        sup->sup_exceptContext[GENERALEXCEPT].c_pc = 0; // TODO: metti GeneralExceptionHandler
        sup->sup_exceptContext[PGFAULTEXCEPT].c_status = IEPON | IMON | TEBITON;
        sup->sup_exceptContext[GENERALEXCEPT].c_status = IEPON | IMON | TEBITON; // GeneralExceptionHandler
        sup->sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = &(sup.sup_stackGen[499]);
        sup->sup_exceptContext[GENERALEXCEPT].c_stackPtr = &(sup.sup_stackGen[499]);

        int row;
        for(row = 0; row<USERPGTBLSIZE-1; row++) {
            sup->sup_privatePgTbl[row].pte_entryHI = VPNBASE + (row<<VPNSHIFT);
            sup->sup_privatePgTbl[row].pte_entryLO = DIRTYON | GLOBALON;
        }
            
        sup->sup_privatePgTbl[row].pte_entryHI = UPROCSTACKPG + (row<<VPNSHIFT);
        sup->sup_privatePgTbl[row].pte_entryLO = DIRTYON | GLOBALON;
        // TODO: Call SYS1 on U-proc
        SYSCALL(CREATEPROCESS, (int)&p, (int)sup, 0);

    }

    // TODO: è sempre UPROCNUMBER?
    for (int i = 0; i < UPROCNUMBER; i++){
        passeren(&masterSemaphore);
    }
    termProcess();
}