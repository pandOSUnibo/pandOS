#include "exceptions.h"
#include "pandos_const.h"
#include "asl.h"
#include "pcb.h"
#include "scheduler.h"
#include <umps3/umps/libumps.h>

#define EXCSTATE ((state_t *) BIOSDATAPAGE)

void passUpOrDie(int index) {
    support_t *supportStructure = currentProcess->p_supportStruct;
    if (supportStructure == NULL) {
        // Die: Kill the process using a termProcess-like call
        termProcess();

    }
    else {
        // Pass up the exception to the Support Level
        supportStructure->sup_exceptState[index] = *EXCSTATE;
        context_t *context = &(supportStructure->sup_exceptContext[index]);
        LDCXT(context->c_stackPtr, context->c_status, context->c_pc);
    }
}

void* memcpy(void *dest, const void *src, size_t len) {
    char *d = dest;
    const char *s = src;
    while (len--) {
        *d++ = *s++;
    }
    return dest;
}

void resume() {
    LDST(EXCSTATE);
}

void interruptsHandler(){}

HIDDEN void TLBExceptionHandler() {
    passUpOrDie(PGFAULTEXCEPT);
}

void trapHandler() {
    
}

void termProcessRecursive(pcb_t *p) {
    pcb_t *child;
    
    // Handle all children
    while ((child = removeChild(p)) != NULL) {
        termProcessRecursive(child);
    }

    // Handle the process itself
    freePcb(p);
}

// SYS2
void termProcess() {
    termProcessRecursive(currentProcess);
    // currentProcess will be overwritten by the scheduler
    // but for good practice we remove the dangling reference
    currentProcess = NULL;

    // Pass control to the scheduler
    schedule();
}

// SYS3
void passeren(int *semAdd) {
    if(*semAdd > 0) {
        *semAdd -= 1;
    } else {
        currentProcess->p_s = *EXCSTATE;
        // update del CPU time
        insertBlocked(semAdd, currentProcess);
        schedule();
    }
}

// SYS4
void verhogen(int *semAdd){
    if(headBlocked(semAdd) != NULL) {
        // Process to be waked up
        pcb_t *unblockedProcess = removeBlocked(semAdd);
        // Added to the ready queue
        insertProcQ(&readyQueue, unblockedProcess);
    }
    else{
        *semAdd += 1;
    }
}

// SYS6
void getTime(cpu_t *resultAddress) {
    *resultAddress = currentProcess->p_time;
}


// SYS7
void clockWait() {
    passeren(&semIntTimer);
}

/**
 * @brief SYS1: creates a new process using the state and the support structures provided.
 * If no memory is available an error code -1 is placed in the caller's v0 register, otherwise 0.
 * 
 * @param statep Initial state of the new process.
 * @param supportp Support structure used from the SupportLevel.
 * 
 */
void createProcess(state_t *statep, support_t * supportp){
    pcb_t *newProc = allocPcb();
    unsigned int retValue = -1;
    if (newProc != NULL){
        newProc->p_supportStruct = supportp;
        newProc->p_s = *statep;
        insertChild(currentProcess, newProc);
        insertProcQ(&readyQueue, newProc);
        retValue = 1;
    }
    EXCSTATE->reg_v0 = retValue;
    EXCSTATE->pc_epc += WORDLEN; 
    LDST(EXCSTATE);
}

void blockProcess(int *sem){
    passaren(sem);
    // TODO: controllare se passaren blocca sempre sul semaforo
    // in caso negativo questa funzione è inutile.
    insertBlocked(sem, currentProcess);
}

/**
 * @brief SYS5: Wait for an I/O operation. It performs a P operation on the semaphore
 * of the selected (sub)device. The process state is saved and the scheduler is called.
 * 
 * @param intlNo Interrupt line number [3..7].
 * @param dNum  Device number [0..7].
 * @param waitForTermRead Terminal read or write.
 */
void ioWait(int intlNo, int dNum, int waitForTermRead){
    // Save the process state
    currentProcess->p_s = *EXCSTATE;
    currentProcess->p_time += TIMESLICE - getTIMER();
    currentProcess->p_s.pc_epc += WORDLEN;

    // Select the correct semaphore
    switch (intlNo)
    {
    case DISKINT:
        blockProcess(&semDisk[dNum]);
        break;
    case FLASHINT:
        blockProcess(&semFlash[dNum]);
        break;
    case NETWINT:
        blockProcess(&semNetwork[dNum]);
        break;
    case PRNTINT:
        blockProcess(&semPrinter[dNum]);
        break;
    case TERMINT:
        if (waitForTermRead)
            blockProcess(&semTerminalRecv[dNum]);
        else
            blockProcess(&semTerminalTrans[dNum]);
        break;
    default:
        break;
    }
    // TODO mantenere solo se passaren non blocca sul semaforo.
    schedule();
}

void syscallHandler(unsigned int KUp) {
    unsigned int sysId = EXCSTATE->reg_a0;
    // Get arguments for syscalls
    unsigned int arg1 = EXCSTATE->reg_a1;
    unsigned int arg2 = EXCSTATE->reg_a2;
    unsigned int arg3 = EXCSTATE->reg_a3;
    memaddr resultAddress = EXCSTATE->reg_v0;

    if (sysId <= 8) {
        // KUp is 0 in kernel mode and 0x00000008
        // in user mode
        if (KUp == 0) {
            switch (sysId){
                case CREATEPROCESS:
                    createProcess((state_t *) arg1, (support_t *)arg2);
                    break;
                case TERMPROCESS:
                    termProcess();
                    break;
                case PASSEREN:
                    passaren((int *) arg1);
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
                    getSupportPtr();
                    break;
                default:
                    PANIC();
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
            interruptsHandler();
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
    };
}