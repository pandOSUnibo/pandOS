/**
 * @file sys_support.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Syscalls provided by support level
 * @version 0.3.0
 * @date 2021-04-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SYSUPPORT_H
#define SYSUPPORT_H

#define DEVREG(IntlineNo, DevNo, Reg)  (DEVADDRBASE(IntlineNo, DevNo) +  (Reg * WORDLEN))
#define DEVADDRBASE(IntlineNo, DevNo) (memaddr)(0x10000054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10))

#include "pandos_const.h"
#include "pandos_types.h"

extern semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

extern void generalExceptionHandler();
extern void trapExceptionHandler(support_t *currentSupport);

#endif