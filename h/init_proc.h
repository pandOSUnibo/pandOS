/**
 * @file init_proc.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Process initialization functions.
 * @version 0.1
 * @date 2021-04-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef INITPROC_H
#define INITPROC_H

#include "pandos_types.h"

#define VPNBASE 0x80000000
#define VPNTOP  0x8001EFFF
#define UPROCSTACKPG 0xBFFFF000


extern semaphore masterSemaphore;

/**
 * @brief First process that will be loaded by pandOS.
 */
extern void test(void);
 
#endif