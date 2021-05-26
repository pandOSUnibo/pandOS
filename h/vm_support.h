/**
 * @file vm_support.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Virtual memory management.
 * @version 0.3.1
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

#define FRAMENUMBER (2 * UPROCNUMBER)
#define UNKNOWNDATAPAGE -1


/**
 * @brief Contains the index of the first .data page for each
 * active user process. UNKNOWNDATAPAGE means that no information
 * is available.
 */
extern int dataPages[UPROCNUMBER];

/**
 * @brief Updates the TLB.
 * 
 * @param updatedEntry Pointer to the entry with the new TLB values.
 */
void updateTLB(pteEntry_t *updatedEntry);

/**
 * @brief Initializes support level structures.
 */
void initSwapStructs();

/**
 * @brief Handles TLB Page Fault exceptions.
 */
void uTLB_PageFaultHandler();

#endif