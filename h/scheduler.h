/**
 * @file scheduler.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Nucleus Initialization
 * @version 0.2.0
 * @date 2021-03-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

extern volatile cpu_t sliceStart;   

/**
 * @brief Selects a process and executes it.
 * 
 * @remark The selected process will be also stored
 * in currentProcess (defined in initial.h).
 */
void schedule();

#endif