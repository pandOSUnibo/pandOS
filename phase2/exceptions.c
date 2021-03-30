#include "exceptions.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "asl.h"
#include "pcb.h"
#include "scheduler.h"
#include "interrupts.h"
#include <umps3/umps/libumps.h>

#include "debug.h"

HIDDEN void termProcess();

// TODO: Considerare se mettere le syscalls in un file separato

/**
 * @brief Calculates the real time elapsed since the beginning of the time
 * slice, which was set at the last time that this process was selected by the scheduler
 * 
 * @return The real time elapsed since the beginning of the time slice 
 */
cpu_t elapsedTime() {
    cpu_t clockTime;
    STCK(clockTime);
    return clockTime - sliceStart;
}

/**
 * @brief Pass Up or Die, for handling exceptions numbered 9 and above,
 * Program Trap and TLB exceptions. Which action between the 2 is taken 
 * depends on whether the current process was proveded a NULL value for
 * its Support Structure pointer when it was created or not.
 * 
 * @param index Either 0 (PGFAULTEXCEPT) or 1 (GENERALEXCEPT), indicating
 * that the exception to be handled is respectively a TLB exception or any
 * of the other cases listed above.
 */
HIDDEN void passUpOrDie(int index) {
    support_t *supportStructure = currentProcess->p_supportStruct;
    if (supportStructure == NULL) {
        // Die: Kill the process using a SYS2-like call
        termProcess();
    }
    else {
        // Pass up the exception to the Support Level
        supportStructure->sup_exceptState[index] = *EXCSTATE;
        context_t *context = &(supportStructure->sup_exceptContext[index]);
        LDCXT(context->c_stackPtr, context->c_status, context->c_pc);
    }
}

/**
 * @brief Copies bytes from an address to
 * another.
 * 
 * @param dest Destination address.
 * @param src Source address.
 * @param len Length of the bytes to be copied.
 * @return void* A pointer to the destination
 * address.
 * @remark This function is also added by the compiler
 * in order to support struct copying.
 */
void* memcpy(void *dest, const void *src, size_t len) {
    char *d = dest;
    const char *s = src;
    while (len--) {
        *d++ = *s++;
    }
    return dest;
}

// TODO resume deve distinguere i casi in cui c'è un processo a cui ritornare il controllo
// e quelli in cui non c'è alcun processo corrente (WAIT() dello scheduler)

void resume() {
    if(currentProcess == NULL)
        schedule();
    else
        LDST(EXCSTATE);
}

/**
 * @brief Handles a TLB exception.
 *  
 */
HIDDEN void TLBExceptionHandler() {
    passUpOrDie(PGFAULTEXCEPT);
}

/**
 * @brief Handles a Program Trap.
 * 
 */
void trapHandler() {
    passUpOrDie(GENERALEXCEPT);
}

/**
 * @brief SYS1: creates a new process using the state and the support structures provided.
 * If no memory is available an error code -1 is placed in the caller's v0 register, otherwise 0.
 * 
 * @param statep Initial state of the new process.
 * @param supportp Support structure used from the SupportLevel.
 * 
 */
void createProcess(state_t *statep, support_t *supportp) {
    pcb_t *newProc = allocPcb();
    unsigned int retValue = -1;
    if (newProc != NULL){
        processCount++;
        newProc->p_supportStruct = supportp;
        newProc->p_s = *statep;
        insertChild(currentProcess, newProc);
        insertProcQ(&readyQueue, newProc);
        retValue = 1;
    }
    EXCSTATE->reg_v0 = retValue;
}

/**
 * @brief Recursively terminates a process and
 * its children.
 * 
 * @param p Process to be terminated.
 */
HIDDEN void termProcessRecursive(pcb_t *p) {
    pcb_t *child;
    
    // Handle all children
    while ((child = removeChild(p)) != NULL) {
        outProcQ(&readyQueue, child);
        termProcessRecursive(child);
    }
    // Handle the process itself
    
    // A process is blocked on a device if the semaphore is
    // semIntTimer or an element of semDevices
    bool blockedOnDevice = 
    (p->p_semAdd >= (int *)semDevices &&
    p->p_semAdd < (int *)semDevices + sizeof(semaphore) * DEVICE_TYPES * DEVICE_INSTANCES) 
    || p->p_semAdd == &semIntTimer;

    // If the process is blocked on a user semaphore, remove it
    outBlocked(p);

    // For device processes, 
    if (!blockedOnDevice) {
        (*(p->p_semAdd))++;
    }

    freePcb(p);
    processCount--;
}

/**    else {
    }
 * @brief SYS2: terminates the process currently in execution and all his progeny recursively.
 * After that, the scheduler is called.
 * 
 */
HIDDEN void termProcess() {
    outChild(currentProcess);
    termProcessRecursive(currentProcess);
    // currentProcess will be overwritten by the scheduler
    // but for good practice we remove the dangling reference
    currentProcess = NULL;

    // Pass control to the scheduler
    schedule();
}

/**
 * @brief SYS3 (P): performs a P operation on the semaphore provided.
 * 
 * @param semAdd Pointer to the semaphore to perform the P on.
 */
void passeren(int *semAdd) {
    (*semAdd)--;

    if (*semAdd < 0) {
        currentProcess->p_s = *EXCSTATE;
        currentProcess->p_time += elapsedTime();
        insertBlocked(semAdd, currentProcess);
        currentProcess = NULL;
        schedule();
    }
}

pcb_t* verhogen(int *semAdd) {
    (*semAdd)++;

    pcb_t *unblockedProcess = NULL;
    if(*semAdd <= 0) {
        // Process to be waked up
        // If there are no longer any processes in the queue
        // (because they were terminated), removeBlocked will
        // return NULL
        unblockedProcess = removeBlocked(semAdd);
        if(unblockedProcess != NULL){
            insertProcQ(&readyQueue, unblockedProcess);
        }
    }
    
    return unblockedProcess;
}

// TODO: L'handler dell'interrupt
// dovrà salvare in v0

/**
 * @brief SYS5: waits for an I/O operation. It performs a P operation on the semaphore
 * of the selected (sub)device. The process state is saved and the scheduler is called.
 * 
 * @param intlNo Interrupt line number in [3, 7].
 * @param dNum  Device number in [0, 7].
 * @param waitForTermRead Terminal read or write.
 */
HIDDEN void ioWait(int intlNo, int dNum, bool waitForTermRead) {
    // Save the process state
    currentProcess->p_s = *EXCSTATE;
    softBlockCount++;
    // Select the correct semaphore
    switch (intlNo) {
        case DISKINT:
        case FLASHINT:
        case NETWINT:
        case PRNTINT:
            passeren(&semDevices[intlNo-3][dNum]);
            break;
        case TERMINT:
            if (waitForTermRead)
                passeren(&semDevices[4][dNum]);
            else
                passeren(&semDevices[5][dNum]);
            break;
        default:
            // Invalid interrupt line: kill
            termProcess();
            break;
    }
}

/**
 * @brief SYS6: stores the real time since the
 * beginning of the process' execution in the
 * provided address.
 * 
 * @param resultAddress Address where the real
 * time will be stored.
 */
HIDDEN void getTime(cpu_t *resultAddress) {
    *resultAddress = currentProcess->p_time + elapsedTime();
}


/**
 * @brief SYS7: blocks the process until the next
 * Interval Timer tick.
 */
HIDDEN void clockWait() {
    softBlockCount++;
    passeren(&semIntTimer);
}

/**
 * @brief SYS8: stores the pointer to the process'
 * support structure in the provided address.
 * 
 * @param resultAddress Address where the support structure
 * will be stored.
 */
HIDDEN void getSupportPtr(support_t **resultAddress) {
    *resultAddress = currentProcess->p_supportStruct;
}

/**
 * @brief Handles syscalls.
 * 
 * @param KUp Value of the KUp bit in the CAUSE
 * register.
 */
HIDDEN void syscallHandler(unsigned int KUp) {
    unsigned int sysId = EXCSTATE->reg_a0;
    // Get arguments for syscalls
    unsigned int arg1 = EXCSTATE->reg_a1;
    unsigned int arg2 = EXCSTATE->reg_a2;
    unsigned int arg3 = EXCSTATE->reg_a3;
    memaddr resultAddress = (memaddr) &(EXCSTATE->reg_v0);

    if (sysId <= 8) {
        // KUp is 0 in kernel mode and 0x00000008
        // in user mode
        if (KUp == 0) {
            switch (sysId) {
                case CREATEPROCESS:
                    createProcess((state_t *) arg1, (support_t *)arg2);
                    break;
                case TERMPROCESS:
                    termProcess();
                    break;
                case PASSEREN:
                    passeren((int *) arg1);
                    break;
                case VERHOGEN:
                    verhogen((int *) arg1);
                    break;
                case IOWAIT:
                    ioWait(arg1, arg2, arg3);
                    break;
                case GETTIME:
                    getTime((cpu_t *)resultAddress);
                    break;
                case CLOCKWAIT:
                    clockWait();
                    break;
                case GETSUPPORTPTR:
                    getSupportPtr((support_t **)resultAddress);
                    break;
                default:
                    // Invalid SYSCALL ID: Kill
                    termProcess();
                    break;
            }

            // The process was not blocked or put in the ready
            // queue
            resume();
        }
        else {
            // Attempted to perform a kernel syscall
            // while in user mode. Call a trap with cause
            // RI (Reserved Instruction)
            EXCSTATE->cause &= ~GETEXECCODE;
            EXCSTATE->cause |= RI << CAUSESHIFT;
            trapHandler();
        }
    }
    else {
        // Pass to the Support Level
        passUpOrDie(GENERALEXCEPT);
    }
}


void exceptionHandler() {
    // TODO: Controllare se il contenuto di BIOSDATAPAGE cambia
    state_t *exceptionState = EXCSTATE;

    unsigned int cause = (exceptionState->cause & GETEXECCODE) >> CAUSESHIFT;

    // TODO: Controllare correttezza. Al massimo si può salvare
    // localmente lo state_t
    // Increment the PC by one word so that when control returns to the
    // process, it does not perform a syscall again
    exceptionState->pc_epc += WORDLEN;

    switch (cause) {
        case INT:
            // Interrupt
            interruptsHandler(exceptionState);
            break;
        case MOD:
        case TLBL:
        case TLBS:
            TLBExceptionHandler();
            break;
        case ADEL:
        case ADES:
        case IBE:
        case DBE:
        case BP:
        case RI:
        case CPU:
        case OV:
            // Traps
            trapHandler();
            break;
        case SYS:
            // Syscalls
            syscallHandler(exceptionState->status & USERPON);
            break;
        default:
            PANIC();
            break;
    }
}