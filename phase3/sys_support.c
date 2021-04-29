#include "sys_support.h"
#include "init_proc.h"

#include "initial.h"

#define PRINTSEM 3
#define TERMRDSEM 4
#define TERMWRSEM 5

semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

#define DEVADDRBASE(IntlineNo, DevNo) (memaddr)(0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10))
#define DEVREG(IntlineNo, DevNo, Reg)  (DEVADDRBASE(IntlineNo, DevNo) +  (Reg * WORDLEN))
// TODO - passare current support come parametro?
#define GETDEVNUMBER(support) support->sup_asid-1

#define TERMSTATUSMASK 0x000000FF
#define TERMTRANSHIFT 8
#define TERMRECVSHIFT 8


void writeToPrinter(char *virtAddr, int len, support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    // Check if the address and the length are valid
    if(virtAddr >= VPNBASE && ((virtAddr + len) <= VPNTOP) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, semMutexDevices[PRINTSEM][devNumber], 0, 0);
        for (int i = 0; i < len; i++) {
            if(*((int *)DEVREG(PRNTINT, devNumber, STATUS)) == READY) {
                *((char *)DEVREG(PRNTINT, devNumber, DATA0)) = *(virtAddr);
                virtAddr++;
                retValue++;
                SYSCALL(IOWAIT, PRNTINT, devNumber, 0);
            }
            else{
                // Return the negative of the device status
                retValue = -(*((int *)DEVREG(PRNTINT, devNumber, STATUS)));
                break;
            }
        }
        currentSupport->sup_exceptState->reg_v0 = retValue;
        SYSCALL(VERHOGEN, semMutexDevices[PRINTSEM][devNumber], 0, 0);  
    } 
    else {
        terminate();
    }
}

void writeToTerminal(char *virtAddr, int len, support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    // Check if the addres and the lenght are valid
    if(virtAddr >= VPNBASE && ((virtAddr + len) <= VPNTOP) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, semMutexDevices[TERMWRSEM][devNumber], 0, 0);
        for (int i = 0; i < len; i++) {
        int ioStatus = 5;
            if((*((int *)DEVREG(TERMINT, devNumber, TRANSTATUS)) & TERMSTATUSMASK) == READY && ioStatus == 5) {
                *((char *)DEVREG(TERMINT, devNumber, TRANCOMMAND)) = (TRANSMITCHAR | (*(virtAddr)<<TERMTRANSHIFT));
                virtAddr++;
                retValue++;
                ioStatus = SYSCALL(IOWAIT, TERMINT, devNumber, 0);
            }
            else{
                // Return the negative of the device status
                retValue = -(*((int *)DEVREG(TERMINT, devNumber, TRANSTATUS)));
                break;
            }
        }
        currentSupport->sup_exceptState->reg_v0 = retValue;
        SYSCALL(VERHOGEN, semMutexDevices[PRINTSEM][devNumber], 0, 0);  
    } 
    else {
        terminate();
    }
}

void readTerminal(char *buffer, support_t *currentSupport){
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    SYSCALL(PASSEREN, semMutexDevices[TERMRDSEM][devNumber], 0, 0);
    int ioStatus;
    const char eol = '\n';
    char recvd;
    while(buffer >= VPNBASE && (buffer <= VPNTOP) && (*((int *)DEVREG(TERMINT, devNumber, RECVSTATUS)) & TERMSTATUSMASK) == READY) {
        ioStatus = SYSCALL(IOWAIT, TERMINT, devNumber, 0);
        // check ioStatus if correct else break and return
        recvd = (char)(DEVREG(TERMINT, devNumber, RECVSTATUS) >> TERMRECVSHIFT);
        if(recvd == eol) {

        }

        buffer++; // ma si fa così o += WORDLEN?
    }
    // TODO - continue
    }
}

void getTod() {
    STCK(EXCSTATE->reg_v0);
}

void terminate() {
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void generalExceptionHandler() {
    // Get syscall code
    volatile unsigned int sysId = EXCSTATE->reg_a0;
    // Increment the PC by one word so that when control returns to the
	// process, it does not perform a syscall again
    // TODO - va incrementato anche t9?
    support_t *currentSupport = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    currentSupport->sup_exceptState->pc_epc += WORDLEN;

    if(sysId<=13){
        syscallExceptionHandler(sysId, currentSupport);
    }
    else{
        trapExceptionHandler();
    }
}

void syscallExceptionHandler(int sysId, support_t *currentSupport) {
    // Get arguments for syscalls
	volatile unsigned int arg1 = EXCSTATE->reg_a1;
	volatile unsigned int arg2 = EXCSTATE->reg_a2;
	volatile unsigned int arg3 = EXCSTATE->reg_a3;
    switch (sysId){
    case TERMINATE:
        terminate();
        break;
    case GET_TOD:
        getTod();
        break;
    case WRITEPRINTER:
        writeToPrinter(arg1, arg2, currentSupport);
        break;
    case WRITETERMINAL:
        writeToTerminal(arg1, arg2, currentSupport);  
        break;
    case READTERMINAL(arg1, currentSupport):
        break;
    default:
        break;
    }
}

// TODO - capire cosa fare dei codici maggiori di 13
void trapExceptionHandler() {
    
}