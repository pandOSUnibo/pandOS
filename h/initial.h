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

/**
 * @brief Number of active processes.
 */
extern unsigned int processCount;

/**
 * @brief Number of processes blocked on
 * device semaphores.
 */
extern unsigned int softBlockCount;

/**
 * @brief Queue of processes that are ready
 * to be executed.
 */
extern pcb_t *readyQueue;

/**
 * @brief Current active process.
 */
extern pcb_t *currentProcess;

/**
 * @brief Semaphores for non-timer devices.
 * 
 */
extern semaphore semDevices[DEVICE_TYPES][DEVICE_INSTANCES];

/**
 * @brief Semaphore for the Interval Timer.
 * 
 */
extern semaphore semIntTimer;

/**
 * @brief The entry point for pandOS.
 * 
 * @return int The exit code.
 */
int main(void);

#endif