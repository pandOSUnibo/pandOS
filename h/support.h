/**
 * @file support.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief 
 * @version 0.3.1
 * @date 2021-04-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SUPPORT_H
#define SUPPORT_H

#include "pandos_types.h"
#include "pandos_const.h"

#define UPROCNUMBER UPROCMAX
/**
 * @brief Insert in the free list the structure passed.
 * 
 * @param support Pointer to the new free support structure.
 */
void deallocSupport(support_t *support);

/**
 * @brief Allocates a new support structure.
 * 
 * @return The allocated support structure. Returns NULL if memory is not available.
 */
support_t* allocSupport();

/**
 * @brief Initialize the support structures
 * 
 */
void initSupport();

#endif
