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

#define MUSEC_TO_TICKS(T)         (T) * (*((cpu_t *) TIMESCALEADDR))

extern cpu_t sliceStart;

void schedule();

#endif