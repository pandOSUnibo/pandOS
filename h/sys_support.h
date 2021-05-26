/**
 * @file sys_support.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Support-level syscalls.
 * @version 0.3.1
 * @date 2021-04-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SYSUPPORT_H
#define SYSUPPORT_H

#include "pandos_const.h"
#include "pandos_types.h"

#define DEVREG(IntlineNo, DevNo, Reg)  (DEVADDRBASE(IntlineNo, DevNo) +  (Reg * WORDLEN))
#define DEVADDRBASE(IntlineNo, DevNo) (memaddr)(0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10))

#define FLASHSEM 1
#define PRINTSEM 3
#define TERMRDSEM 4
#define TERMWRSEM 5


/**
 * @brief Support level semaphores to regulate access to
 * devices.
 * 
 * @remark These semaphores should always be accessed with
 * SYSCALL(...).
 */
extern semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

/**
 * @brief Handles general support level exceptions.
 */
extern void generalExceptionHandler();

/**
 * @brief Handles support level traps.
 * 
 * @param currentSupport Pointer to the support structure of
 * the process that caused the trap.
 */
extern void trapExceptionHandler(support_t *currentSupport);

#endif