#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>

#include "init_proc.h"
#include "support.h"
#include "sys_support.h"
#include "vm_support.h"

semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

#define GETDEVNUMBER(support) (support->sup_asid - 1)

#define CHECKADDRTXT(START, END) ((START >= (char *) VPNBASE) && (END <= (char *) VPNTOP))

#define CHECKADDRSTCK(START, END) ((START >= (char *) UPROCSTACKPG) && (END <= (char *) USERSTACKTOP))

// Check if the given address is valid
#define CHECKADDR(START, END) (CHECKADDRTXT(START, END) || CHECKADDRSTCK(START, END))

#define DISABLEINTERRUPTS setSTATUS(getSTATUS() & (~IECON))
#define ENABLEINTERRUPTS setSTATUS(getSTATUS() | IECON | IMON)

#define PRINTCHR 2

#define TERMSTATUSMASK 0x000000FF
#define TERMTRANSHIFT 8
#define TERMRECVSHIFT 8

#define EOL '\n'
#define RECEIVECHAR 2


/**
 * @brief Returns control to the user process after
 * the execution of a support level service.
 * 
 * @param currentSupport Pointer to the support structure
 * of the current process.
 */
void resumeSupport(support_t *currentSupport){
    currentSupport->sup_exceptState[GENERALEXCEPT].pc_epc += WORDLEN;
    LDST(&currentSupport->sup_exceptState[GENERALEXCEPT]);
}

/**
 * @brief Handles support level termination of a process.
 * 
 * @param currentSupport Pointer to the support structure of the
 * current process.
 */
void terminate(support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    // Check if the process holds a mutex semaphore
    for(int i = 0; i < DEVICE_TYPES; i++) {
        if(semMutexDevices[i][devNumber] == 0){
            SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[i][devNumber], 0, 0);
        }
    }

    // Mark as unused all the pages
    for (int i = 0; i < USERPGTBLSIZE; i++){
        if(currentSupport->sup_privatePgTbl[i].pte_entryLO & VALIDON){
            DISABLEINTERRUPTS;
            currentSupport->sup_privatePgTbl[i].pte_entryLO &= ~VALIDON;
            updateTLB(&currentSupport->sup_privatePgTbl[i]);
            ENABLEINTERRUPTS;
        }
    }

    SYSCALL(VERHOGEN, (memaddr) &masterSemaphore, 0, 0);
    deallocSupport(currentSupport);
    SYSCALL(TERMPROCESS, 0, 0, 0);
}


/**
 * @brief Writes a string to the printer used by the current process.
 * 
 * @param virtAddr Pointer to the logical address of the first character
 * of the string.
 * @param len Lenght of the string.
 * @param currentSupport Pointer to the support structure of the current process.
 */
void writeToPrinter(char *virtAddr, int len, support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;

    // Check if the address and the length are valid
    if(CHECKADDR(virtAddr, virtAddr+len) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);
        dtpreg_t *writingRegister = (dtpreg_t *)DEV_REG_ADDR(PRNTINT, devNumber);
        for (int i = 0; i < len; i++) {
            if(writingRegister->status == READY) {
                DISABLEINTERRUPTS;
                writingRegister->data0 = ((unsigned int) *(virtAddr + i));
                writingRegister->command = PRINTCHR;
                SYSCALL(IOWAIT, PRNTINT, devNumber, FALSE);
                ENABLEINTERRUPTS;
                retValue++;
            }
            else {
                // Return the negative of the device status
                retValue = -(writingRegister->status);
                break;
            }
        }

        currentSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = retValue;
        SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[PRINTSEM][devNumber], 0, 0);  
    } 
    else {
        terminate(currentSupport);
    }
}

/**
 * @brief Writes a string to the terminal used by the current process.
 * 
 * @param virtAddr Pointer to the logical address of the first character
 * of the string.
 * @param len Lenght of the string.
 * @param currentSupport Pointer to the support structure of the current process.
 */
void writeToTerminal(char *virtAddr, int len, support_t *currentSupport) {
    int devNumber = GETDEVNUMBER(currentSupport);
    unsigned int retValue = 0;

    // Check if the address and the length are valid
    if(CHECKADDR(virtAddr, virtAddr+len) && len <= 128 && len >= 0) {
        SYSCALL(PASSEREN, (memaddr) &semMutexDevices[TERMWRSEM][devNumber], 0, 0);
        volatile termreg_t *writingRegister = (termreg_t *) DEV_REG_ADDR(TERMINT, devNumber);
        unsigned int ioStatus = OKCHARTRANS;
        for (int i = 0; i < len; i++) {
            if(((writingRegister->transm_status & TERMSTATUSMASK) == READY) && ((ioStatus & TERMSTATUSMASK) == OKCHARTRANS)) {
                DISABLEINTERRUPTS;  
                writingRegister->transm_command = (((unsigned int) *(virtAddr + i)) << TERMTRANSHIFT) | TRANSMITCHAR;
                ioStatus = SYSCALL(IOWAIT, TERMINT, devNumber, FALSE);
                ENABLEINTERRUPTS;
                retValue++;
            }
            else{
                // Return the negative of the device status
                retValue = -(ioStatus);
                break;
            }
        }

        currentSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = retValue;
        SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[TERMWRSEM][devNumber], 0, 0);  
    } 
    else {
        terminate(currentSupport);
    }
}


/**
 * @brief Reads a string from the terminal used by the current process.
 * 
 * @param buffer Logical address of the first character that will store
 * the string.
 * @param currentSupport Pointer to the support structure of the current process.
 * 
 * @remark In case of buffer overflow, the process is terminated.
 */
void readTerminal(char *buffer, support_t *currentSupport){
    int devNumber = GETDEVNUMBER(currentSupport);
    int retValue = 0;
    int ioStatus;
    char recvd = ' ';
    termreg_t *readingRegister = (termreg_t *) DEV_REG_ADDR(TERMINT, devNumber);

    SYSCALL(PASSEREN, (memaddr) &semMutexDevices[TERMRDSEM][devNumber], 0, 0);

    // Check if the buffer address is valid, if the device is ready and if the last character
    // read is different from the end of line
    while(CHECKADDR(buffer, buffer) && ((readingRegister->recv_status & TERMSTATUSMASK) == READY) && (recvd != EOL)) {
        DISABLEINTERRUPTS;
        readingRegister->recv_command = RECEIVECHAR;
        ioStatus = SYSCALL(IOWAIT, TERMINT, devNumber, TRUE);
        ENABLEINTERRUPTS;
        if((ioStatus & TERMSTATUSMASK) == OKCHARTRANS) {
            // Check if ioStatus is correct, otherwise break and return
            recvd = (ioStatus >> TERMTRANSHIFT);
            if(recvd != EOL) {
                *buffer = recvd;
                buffer++;
                retValue++;
            }
        }
        else {
            break;
        }
    }

    // Check the exit condition of the while loop
    // Terminate the process if the buffer is an invalid adress
    if((readingRegister->recv_status & TERMSTATUSMASK) != READY || (ioStatus & TERMSTATUSMASK) != OKCHARTRANS) {
        retValue = -(ioStatus);
    }

    SYSCALL(VERHOGEN, (memaddr) &semMutexDevices[TERMRDSEM][devNumber], 0, 0);

    if(CHECKADDR(buffer, buffer)){
        currentSupport->sup_exceptState[GENERALEXCEPT].reg_v0 = retValue;
    }
    else{
        terminate(currentSupport);
    }
}

/**
 * @brief Returns the value of the TOD clock.
 * 
 * @param currentSupport Pointer to the support structure of the current process.
 */
void getTod(support_t *currentSupport) {
    STCK(currentSupport->sup_exceptState[GENERALEXCEPT].reg_v0);
}

/**
 * @brief Handles support level syscalls.
 * 
 * @param sysId Syscall id.
 * @param currentSupport Pointer to the support structure of the current process.
 */
void syscallExceptionHandler(int sysId, support_t *currentSupport) {
    // Get arguments for syscalls
    volatile unsigned int arg1 = currentSupport->sup_exceptState[GENERALEXCEPT].reg_a1;
    volatile unsigned int arg2 = currentSupport->sup_exceptState[GENERALEXCEPT].reg_a2;

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


