/**
 * @file interrupts.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Interrupt handler functions.
 * @version 0.2.0
 * @date 2021-03-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <umps3/umps/types.h>

/**
 * @brief Handles interrupts.
 * 
 * @param exceptionState Pointer to the process
 * state at the time of the interrupt.
 */
void interruptsHandler(state_t *exceptionState);

#endif