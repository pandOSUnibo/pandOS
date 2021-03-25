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

typedef int semaphore;
#define DEVICE_TYPES   6
#define DEVICE_INSTANCES 8

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern pcb_t *readyQueue;
extern pcb_t *currentProcess;

extern semaphore semDevices[DEVICE_TYPES][DEVICE_INSTANCES];
extern semaphore semIntTimer;

int main(void);

#endif