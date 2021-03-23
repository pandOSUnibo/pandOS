#include "exceptions.h"
#include "pandos_const.h"

#define LOCALTIMERINT  0x00000200
#define TIMERINTERRUPT 0x00000400
#define DISKINTERRUPT  0x00000800
#define FLASHINTERRUPT 0x00001000
#define NETWINTERRUPT  0x00002000
#define PRINTINTERRUPT 0x00004000
#define TERMINTERRUPT  0x00008000

#define GETIP   0x0000FE00

#define DEVREGAREA ((devregarea_t *)RAMBASEADDR)

int mapToInt(unsigned int map) {
    switch (map) {
        case DEV0ON:
            return 0;
        case DEV1ON:
            return 1;
        case DEV2ON:
            return 2;
        case DEV3ON:
            return 3;
        case DEV4ON:
            return 4;
        case DEV5ON:
            return 5;
        case DEV6ON:
            return 6;
        case DEV7ON:
            return 7;
        default:
            PANIC();
            return;
    }
}

void nonTimerInterrupt(int index) {
    // index is in [0, 4]
    unsigned int istanceMap = DEVREGAREA->interrupt_dev[index];
    // Get the first active
    // TODO: Potrebbe causare starvation
    istanceMap &= -istanceMap;
    int istanceID = mapToInt(istanceMap);

    unsigned int status;
    if (index >= 3) {
        // Terminal device
        status = DEVREGAREA->devreg[index][istanceID].term.recv_status;
    }
    else {
        // DTP device
        status = DEVREGAREA->devreg[index][istanceID].dtp.status;
    }
}

void interruptsHandler(state_t *exceptionState) {

    unsigned int ip = (exceptionState->cause & GETIP);
    // Keep the least significant bit
    ip &= -ip;
    
    switch (ip) {
        case DEV0ON:
            break;
        case LOCALTIMERINT:
            /* code */
            break;
        case TIMERINTERRUPT:
            break;
        case DISKINTERRUPT:
            break;
        case FLASHINTERRUPT:
            break;
        case NETWINTERRUPT:
            break;
        case PRINTINTERRUPT:
            nonTimerInterrupt(3);
            break;
        case TERMINTERRUPT:
            nonTimerInterrupt(4);
            break;

                default:
            break;
    }
}
