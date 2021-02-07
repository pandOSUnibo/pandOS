#ifndef ASL_H
#define ASL_H

#include "pandos_types.h"

int insertBlocked(int *semAdd, pcb_t *p);

pcb_t* removeBlocked(int *semAdd);

pcb_t* outBlocked(pcb_t *p);

/**
 * @brief Return a pointer to the pcb that is the head of 
 * process queue associated with semaphore semAdd. Return NULL if the queue
 * is empty or the semaphore is not present.
 * 
 * @param semAdd Semaphore identifier
 * @return pcb_t* 
 */
pcb_t* headBlocked(int *semAdd);

void initASL();

#endif