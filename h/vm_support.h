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

#include "support.h"

#define FRAMENUMBER  (2 * UPROCMAX)
#define UNKNOWNDATAPAGE -1

/**
 * @brief Contains information regarding the swap pool.
 */
extern swap_t swapTable[FRAMENUMBER];

/**
 * @brief Semaphore used to regulate access to the swap pool.
 * 
 * @remark This semaphore should always be accessed with
 * SYSCALL(...).
 */
extern semaphore semSwapPool;

/**
 * @brief Contains the index of the first .data page for each
 * active user process. UNKNOWNDATAPAGE means that no information
 * is available.
 */
extern int dataPages[UPROCNUMBER];

/**
 * @brief Handles TLB Page Fault exceptions.
 */
void uTLB_PageFaultHandler();

#endif