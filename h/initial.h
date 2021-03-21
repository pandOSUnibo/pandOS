/**
 * @file initial.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Nucleus Initialization
 * @version 0.2.0
 * @date 2021-03-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef INITIAL_H
#define INITIAL_H

#include "pandos_const.h"
#include "pandos_types.h"

#define PASSVEC_LOCATION 0x0FFFF900
#define TLBSP_START KERNELSTACK
#define EXCSP_START KERNELSTACK

#define SEMAPHORE int
#define DEVICE_ISTANCES 8

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern pcb_t *readyQueue;
extern pcb_t *currentProcess;

extern SEMAPHORE semDisk[DEVICE_ISTANCES];
extern SEMAPHORE semFlash[DEVICE_ISTANCES];
extern SEMAPHORE semNetwork[DEVICE_ISTANCES];
extern SEMAPHORE semPrinter[DEVICE_ISTANCES];
extern SEMAPHORE semTerminalTrans[DEVICE_ISTANCES];
extern SEMAPHORE semTerminalRecv[DEVICE_ISTANCES];
extern SEMAPHORE semIntTimer;

int main(void);

#endif