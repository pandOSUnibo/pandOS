#include "exceptions.h"
#include "types.h"
#include "pandos_const.h"
#include <umps3/umps/libumps.h>

void exceptionHandler() {

    state_t exceptionState = *((state_t *) BIOSDATAPAGE);
    unsigned int cause = (exceptionState.cause & GETEXECCODE) >> 2;
    
    switch (cause) {
        case INT:
            // Interrupt
            break;
        case MOD:
        case TLBL:
        case TLBS:
        // TLB-Refill events
            uTLB_RefillHandler();
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
            break;
        case SYS:
            // Syscalls
            break;
        default:
            PANIC();
            break;
    };
}