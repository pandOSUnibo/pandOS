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
        // TODO: Gestire come Syscall 2 (kill)
    }
    else {
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
    LDST(&(currentProcess->p_s));
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

void termProcess() {
    termProcessRecursive(currentProcess);
    // TODO: Is it necessary?
    currentProcess = NULL;

    // Pass control to the scheduler
    schedule();
}

void getTime(cpu_t *resultAddress) {
    *resultAddress = currentProcess->p_time;
    resume();
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
                    createProcess((state_t *) arg1, (support_t *)arg2, arg3);
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

void passeren(int *semAdd) {
    *semAdd -= 1;
    if(*semAdd < 0) {
        insertBlocked(semAdd, currentProcess);
        schedule();
    }
}

void clockWait() {
    passeren(&semIntTimer);
}

void exceptionHandler() {
    // TODO: Controllare se il contenuto di BIOSDATAPAGE cambia
    state_t *exceptionState = EXCSTATE;
    unsigned int cause = (exceptionState->cause & GETEXECCODE) >> CAUSESHIFT;

    // TODO: Controllare correttezza. Al massimo si può salvaare
    // localmente lo state_t
    // Increment the PC by one word so that when control returns to the
    // process, it does not perform a syscall again
    exceptionState->pc_epc += WORDLEN;
    currentProcess->p_s = *exceptionState;

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