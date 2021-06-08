#include <umps3/umps/libumps.h>

#include "pandos_const.h"

#include "init_proc.h"
#include "support.h"
#include "sys_support.h"
#include "vm_support.h"

/**
 * @brief Semaphore to handle process creation and
 * termination.
 * 
 * @remark This semaphore should always be accessed with
 * SYSCALL(...).
 */
semaphore masterSemaphore;




void test(void) {
    initSwapStructs();

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
        // Initialize Support Structure for the i-th U-proc
        sup->sup_asid = id;
        sup->sup_exceptContext[PGFAULTEXCEPT].c_pc = (memaddr) &uTLB_PageFaultHandler;
        sup->sup_exceptContext[GENERALEXCEPT].c_pc = (memaddr) &generalExceptionHandler; 
        sup->sup_exceptContext[PGFAULTEXCEPT].c_status = IEPON | IMON | TEBITON;
        sup->sup_exceptContext[GENERALEXCEPT].c_status = IEPON | IMON | TEBITON;
        sup->sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = (memaddr) &(sup->sup_privatePgTbl[499]);
        sup->sup_exceptContext[GENERALEXCEPT].c_stackPtr = (memaddr) &(sup->sup_stackGen[499]);

        int row;
        for(row = 0; row<USERPGTBLSIZE-1; row++) {
            sup->sup_privatePgTbl[row].pte_entryHI = VPNBASE + (row << VPNSHIFT) + (id << ASIDSHIFT);
            sup->sup_privatePgTbl[row].pte_entryLO = DIRTYON;
        }

        sup->sup_privatePgTbl[row].pte_entryHI = UPROCSTACKPG + (id << ASIDSHIFT);
        sup->sup_privatePgTbl[row].pte_entryLO = DIRTYON;
        // Call SYS1 on U-proc
        SYSCALL(CREATEPROCESS, (memaddr) &p, (memaddr)sup, 0);
    }

    for (int i = 0; i < UPROCNUMBER; i++){
        SYSCALL(PASSEREN, (memaddr) &masterSemaphore, 0, 0);
    }
    SYSCALL(TERMPROCESS, 0, 0, 0);
}