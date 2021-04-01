/**
 * @file exceptions.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Exceptions handler
 * @version 0.2.0
 * @date 2021-03-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "pandos_types.h"

/**
 * @brief Calculates the real time elapsed since the beginning of the time
 * slice, which was set at the last time that this process was selected by the scheduler
 * 
 * @return The real time elapsed since the beginning of the time slice 
 */
cpu_t elapsedTime();

/**
 * @brief Gives control back to the process that was executing before the
 * exception, or call the scheduler if there was no process running (WAIT state).
 * 
 */
void resume();

/**
 * @brief Handles an exception.
 * 
 * @remark This procedure should not be called
 * directly: if correctly configured, the processor
 * will call it when an exception is raised.
 */
void exceptionHandler();

#endif