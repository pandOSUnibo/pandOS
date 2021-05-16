/**
 * @file vm_support.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Virtual memory management.
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

/**
 * @brief Contains information regarding the swap pool.
 */
extern swap_t swapTable[FRAMENUMBER];

/**
 * @brief Semaphore to regulate access to the swap pool.
 * 
 * @remark This semaphore should always be accessed with
 * SYSCALL(...).
 */
extern semaphore semSwapPool;

/**
 * @brief Handler for TLB Page Faults.
 */
void uTLB_PageFaultHandler();

#endif