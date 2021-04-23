#include "sys_support.h"
#include "init_proc.h"

#include "initial.h"

semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

#define DEVADDRBASE(IntlineNo, DevNo) (memaddr)(0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10))
#define DEVREG(IntlineNo, DevNo, Reg)  (DEVADDRBASE(IntlineNo, DevNo) +  (Reg * WORDLEN))

// TODO - DEVREG che tipo è ??
void writeToPrinter(char *virtAddr, int len) {
    int devNumber = currentProcess->p_supportStruct->sup_asid-1;
    // Check if the addres and the lenght are valid
    if(virtAddr >= VPNBASE && ((virtAddr + len) <= VPNTOP) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, semMutexDevices[3][devNumber], 0);
        for (int i = 0; i < len; i++) {
            if(*((int *)DEVREG(PRNTINT, devNumber, STATUS)) == READY) {
                *((char *)DEVREG(PRNTINT, devNumber, DATA0)) = *(virtAddr);
                virtAddr++;
                SYSCALL(IOWAIT, PRNTINT, devNumber, 0);
            }
        }
        SYSCALL(VERHOGEN, semMutexDevices[3][devNumber]);  
    } 
    else {
        terminate();
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

    if(sysId<=13){
        syscallExceptionHandler(sysId);
    }
    else{
        trapExceptionHandler();
    }
}

void syscallExceptionHandler(int sysId) {
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
        writeToPrinter(arg1, arg2);
        break;
    case WRITETERMINAL:
        break;
    case READTERMINAL:
        break;
    default:
        break;
    }
}

// TODO - capire cosa fare dei codici maggiori di 13
void trapExceptionHandler() {
    
}