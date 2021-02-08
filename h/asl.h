#ifndef ASL_H
#define ASL_H

#include "pandos_types.h"

int insertBlocked(int *semAdd, pcb_t *p);

pcb_t* removeBlocked(int *semAdd);

/**
 * @brief Remove and return the pcb pointed by p from the
 * process queue of the semaphore on which p is blocked.
 * 
 * @param p Pointer to the pcb to be removed.
 * @return A pointer to the removed pcb.
 * If the pcb pointed by p is not in the process queue
 * of his semaphore (error situation) return NULL.
 */
pcb_t* outBlocked(pcb_t *p);

/**
 * @brief Return a pointer to the pcb that is the head of 
 * process queue associated with semaphore semAdd. Return NULL if the queue
 * is empty or the semaphore is not present.
 * 
 * @param semAdd Semaphore identifier
 * @return 
 */
pcb_t* headBlocked(int *semAdd);

void initASL();

#endif