#include "asl.h"
#include "exceptions.h"
#include "initial.h"
#include "pandos_const.h"
#include "pcb.h"
#include "scheduler.h"

#define GETIP   0x0000FE00
#define IPSHIFT 2
#define TRANS_CHAR 5
#define RECVD_CHAR TRANS_CHAR

#define DEVREGAREA ((devregarea_t *)RAMBASEADDR)

int mapToInt(unsigned int map) {
    switch (map) {
        case 0x00000001:
            return 0;
        case 0x00000002:
            return 1;
        case 0x00000004:
            return 2;
        case 0x00000008:
            return 3;
        case 0x00000010:
            return 4;
        case 0x00000020:
            return 5;
        case 0x00000040:
            return 6;
        case 0x00000080:
            return 7;
        default:
            PANIC();
            return;
    }
}

void unblockLoad(int deviceType, int instanceID, unsigned int statusCode) {
    pcb_t *unblockedProc;
    unblockedProc = verhogen(&(semDevices[deviceType][instanceID]));
    
    if(unblockedProc != NULL) {
        unblockedProc->p_s.reg_v0 = statusCode;
        insertProcQ(&readyQueue, unblockedProc);
    }
}

void nonTimerInterrupt(int deviceType) {
    // deviceType is in [0, 4]
    unsigned int instanceMap = DEVREGAREA->interrupt_dev[deviceType];
    // Get the device instance with highest priority
    instanceMap &= -instanceMap;
    int instanceID = mapToInt(instanceMap);
    unsigned int statusCode;
    
    if (deviceType == 4)
    {
        // Terminal device 
        termreg_t *termStatus = &(DEVREGAREA->devreg[deviceType][instanceID].term);
        
        if (termStatus->recv_status == RECVD_CHAR){
            statusCode = termStatus->recv_status;
            DEVREGAREA->devreg[deviceType][instanceID].term.recv_command = ACK;
            unblockLoad(deviceType, instanceID, statusCode);
        }
        if (termStatus->transm_status == TRANS_CHAR)
        {
            statusCode = termStatus->transm_status;
            DEVREGAREA->devreg[deviceType][instanceID].term.transm_command = ACK;
            unblockLoad(deviceType + 1, instanceID, statusCode);
        }
    }
    else {
        // DTP device
        statusCode = DEVREGAREA->devreg[deviceType][instanceID].dtp.status;
        DEVREGAREA->devreg[deviceType][instanceID].dtp.command = ACK;
        unblockLoad(deviceType, instanceID, statusCode);
    }
    resume();
}

void pltInterrupt() {
    // Load a very large time
    setTIMER(MUSEC_TO_TICKS(100000UL));
    currentProcess->p_s = *EXCSTATE;
    insertProcQ(readyQueue, currentProcess);
    currentProcess = NULL; // TODO: Togliere?
    schedule();
}

void intervalTimerInterrupt() {
    LDIT(100000UL);

    // Free all processes
    pcb_t *blockedProcess = NULL;
    while ((blockedProcess = removeBlocked(&semIntTimer)) != NULL) {
        insertProcQ(readyQueue, blockedProcess);
    }

    // Reset the semaphore value
    semIntTimer = 0;

    // There are two main cases:
    // - The interrupt happens while a process is in execution
    // and EXCSTATE contains the state of the process
    // - The interrupt happens while the scheduler is in WAIT
    // mode and EXCSTATE contains the state of the nucleus
    // while in WAIT mode.
    // In both cases, we can resume by loading EXCSTATE
    resume();
}

void interruptsHandler(state_t *exceptionState) {
    unsigned int ip = (exceptionState->cause & GETIP);
    // Keep the least significant bit
    ip &= -ip;
    
    switch (ip) {
        case LOCALTIMERINT:
            pltInterrupt();
            break;
        case TIMERINTERRUPT:
            intervalTimerInterrupt();
            break;
        case DISKINTERRUPT:
        case FLASHINTERRUPT:
        case NETWINTERRUPT:
        case PRINTINTERRUPT:
        case TERMINTERRUPT:
            nonTimerInterrupt(mapToInt(ip >> IPSHIFT) - 3);
            break;
        default:
            break;
    }
}
