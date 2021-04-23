/**
 * @file vm_support.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Virtual memory managment
 * @version 0.3.0
 * @date 2021-04-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "pandos_const.h"
#include "pandos_types.h"

#define FRAMENUMBER  (2 * UPROCMAX)

extern pteEntry_t swapTable[FRAMENUMBER];
extern semaphore semSwapPool;

void uTLB_PageFaultHandler();

#endif