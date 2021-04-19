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

#include "pandos_const.h"
#include "pandos_types.h"

extern semaphore semMutexDevices[DEVICE_TYPES][DEVICE_INSTANCES];

#endif