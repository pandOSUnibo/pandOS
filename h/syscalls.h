/**
 * @file syscalls.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Syscall handler functions.
 * @version 0.2.0
 * @date 2021-03-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "pandos_types.h"


/**
 * @brief SYS1: creates a new process using the state and the support structures provided.
 * If no memory is available an error code -1 is placed in the caller's v0 register, otherwise 0.
 * 
 * @param statep Initial state of the new process.
 * @param supportp Support structure used by the support level.
 * 
 */
void createProcess(state_t * statep, support_t * supportp);

/**
 * @brief  SYS2: kills the running process and all its progeny.
 * 
 */
void termProcess();

/**
 * @brief SYS3 (P): performs a P operation on the semaphore provided.
 * 
 * @param semAdd Pointer to the semaphore to perform the P on.
 */
void passeren(semaphore *semAdd);

/**
 * @brief SYS4 (V): performs a V operation on the semaphore provided.
 * 
 * @param semAdd Pointer to the semaphore to perform the V on.
 * @return Pointer to the PCB of the process that was eventually unblocked
 * by the V operation, NULL if there was no process to be unblocked.
 */
pcb_t* verhogen(semaphore *semAdd);

/**
 * @brief SYS5: waits for an I/O operation. It performs a P operation on the semaphore
 * of the selected (sub)device. The process state is saved and the scheduler is called.
 * 
 * @param intlNo Interrupt line number in [3, 7].
 * @param dNum  Device number in [0, 7].
 * @param waitForTermRead Terminal read or write.
 */
void ioWait(int intlNo, int dNum, bool waitForTermRea);

/**
 * @brief SYS6: stores the real time since the
 * beginning of the process' execution in the
 * provided address.
 * 
 * @param resultAddress Address where the real
 * time will be stored.
 */
void getTime(cpu_t * resultAddress);

/**
 * @brief SYS7: blocks the process until the next
 * Interval Timer tick.
 */
void clockWait();

/**
 * @brief SYS8: stores the pointer to the process'
 * support structure in the provided address.
 * 
 * @param resultAddress Address where the support structure
 * will be stored.
 */
void getSupportPtr(support_t ** resultAddress);

#endif