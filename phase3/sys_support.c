#include <umps3/umps/libumps.h>

#include "support.h"
#include "sys_support.h"
#include "init_proc.h"

#define PRINTSEM 3
#define TERMRDSEM 4
#define TERMWRSEM 5

semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

// TODO - passare current support come parametro?
#define GETDEVNUMBER(support) support->sup_asid-1

#define TERMSTATUSMASK 0x000000FF
#define TERMTRANSHIFT 8
#define TERMRECVSHIFT 8

#define EOL '\n'

// TODO - Linee troppo lunghe!!!

void resumeSupport(support_t *currentSupport){
    // TODO - va incrementato anche t9?
    currentSupport->sup_exceptState->pc_epc += WORDLEN;
    LDCXT(currentSupport->sup_exceptState->reg_sp, currentSupport->sup_exceptState->status, currentSupport->sup_exceptState->pc_epc);
}

void terminate(support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    // Check if the process holds a mutex semaphore
    if(semMutexDevices[PRINTSEM][devNumber] == 0){
        SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);
    }
    if(semMutexDevices[TERMWRSEM][devNumber] == 0){
         SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[TERMWRSEM][devNumber], 0, 0);
    }
    if(semMutexDevices[TERMRDSEM][devNumber] == 0){
         SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[TERMRDSEM][devNumber], 0, 0);
    }

    // TODO - check if holding mutual exclusion semaphore
    deallocSupport(currentSupport);
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void writeToPrinter(char *virtAddr, int len, support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    // Check if the address and the length are valid
    if(virtAddr >= (char *) VPNBASE && ((virtAddr + len) <= (char *) VPNTOP) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);
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
        SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);  
    } 
    else {
        terminate(currentSupport);
    }
}

void writeToTerminal(char *virtAddr, int len, support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    // Check if the address and the lenght are valid
    if(virtAddr >=  (char *) VPNBASE && ((virtAddr + len) <=  (char *) VPNTOP) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, (memaddr) &semMutexDevices[TERMWRSEM][devNumber], 0, 0);
        for (int i = 0; i < len; i++) {
            int ioStatus = OKCHARTRANS;
            if((*((int *)DEVREG(TERMINT, devNumber, TRANSTATUS)) & TERMSTATUSMASK) == READY && ioStatus == OKCHARTRANS) {
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
        SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);  
    } 
    else {
        terminate(currentSupport);
    }
}

// TODO - è possibile migliorare le condizioni del while?
void readTerminal(char *buffer, support_t *currentSupport){
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    int ioStatus;
    char recvd = ' ';
    SYSCALL(PASSEREN, (memaddr) &semMutexDevices[TERMRDSEM][devNumber], 0, 0);
    // Check if the buffer address is valid, if the device is ready and if the last character
    // read is different from the end of line
    while(buffer >=  (char *) VPNBASE && (buffer <=  (char *) VPNTOP) && (*((int *)DEVREG(TERMINT, devNumber, RECVSTATUS)) & TERMSTATUSMASK) == READY && recvd != EOL) {
        ioStatus = SYSCALL(IOWAIT, TERMINT, devNumber, 0);
        if(ioStatus == OKCHARTRANS){
            // check ioStatus if correct else break and return
            recvd = (char)(DEVREG(TERMINT, devNumber, RECVSTATUS) >> TERMRECVSHIFT);
            if(recvd != EOL) {
                *buffer = recvd;
                buffer++;
                retValue++;
            }
        }
        else{
            retValue = -(*((int *)DEVREG(TERMINT, devNumber, RECVSTATUS)));
            break;
        }
    }
    SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);
    // Terminate the process if the buffer is an invalid adress
    if((*((int *)DEVREG(TERMINT, devNumber, RECVSTATUS)) & TERMSTATUSMASK) != READY){
        retValue = -(*((int *)DEVREG(TERMINT, devNumber, RECVSTATUS)));
    }
    if((buffer >=  (char *) VPNBASE) && (buffer <=  (char *) VPNTOP)){
        currentSupport->sup_exceptState->reg_v0 = retValue;
    }
    else{
        terminate(currentSupport);
    }
}


void getTod(support_t *currentSupport) {
    STCK(currentSupport->sup_exceptState->reg_v0);
}


void syscallExceptionHandler(int sysId, support_t *currentSupport) {
    // Get arguments for syscalls
	volatile unsigned int arg1 = EXCSTATE->reg_a1;  //TODO: non si puo usare excstate a livello supporto
	volatile unsigned int arg2 = EXCSTATE->reg_a2;
    
    switch (sysId) {
    case TERMINATE:
        terminate(currentSupport);
        break;
    case GET_TOD:
        getTod(currentSupport);
        break;
    case WRITEPRINTER:
        writeToPrinter((char *) arg1, arg2, currentSupport);
        break;
    case WRITETERMINAL:
        writeToTerminal((char *) arg1, arg2, currentSupport);  
        break;
    case READTERMINAL:
        readTerminal((char *) arg1, currentSupport);
        break;
    default:
        trapExceptionHandler(currentSupport);
        break;
    }
    resumeSupport(currentSupport);
}

void trapExceptionHandler(support_t *currentSupport) {
    terminate(currentSupport);
}

void generalExceptionHandler() {
    
    support_t *currentSupport = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);

    // Get syscall code
    volatile unsigned int sysId = currentSupport->sup_exceptState[GENERALEXCEPT].reg_a0;

    if(sysId <= 13) {
        syscallExceptionHandler(sysId, currentSupport);
    }
    else{
        // Invalid syscall number, treat it as a trap
        trapExceptionHandler(currentSupport);
    }
}


