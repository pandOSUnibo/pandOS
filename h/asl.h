#ifndef ASL_H
#define ASL_H

#include "pandos_types.h"

/**
 * @brief Inserts a PCB at the tail of the
 * process queue associated with the semaphore
 * with the given physical address.
 * 
 * @param semAdd Physical address of the process
 * queue.
 * @param p Pointer to the PCB to be inserted.
 * @return TRUE if a new semaphore descriptor is
 * allocated, FALSE otherwise.
 * @remark TRUE and FALSE are defined in
 * umps3/umps/const.h.
 */
int insertBlocked(int *semAdd, pcb_t *p);

/**
 * @brief Removes the PCB at the head of the
 * process queue associated with the semaphore
 * with the given physical address
 * 
 * @param semAdd Physical address of the process
 * queue.
 * @return The head of the process queue if a
 * descriptor with physical address semAdd was
 * found, NULL otherwise.
 */
pcb_t* removeBlocked(int *semAdd);

/**
 * @brief Removes and returns the pcb pointed by p from the
 * process queue of the semaphore on which p is blocked.
 * 
 * @param p Pointer to the pcb to be removed.
 * @return A pointer to the removed pcb.
 * If the pcb pointed by p is not in the process queue
 * of his semaphore (error situation) returns NULL.
 */
pcb_t* outBlocked(pcb_t *p);

/**
 * @brief Returns a pointer to the pcb that is the head of 
 * process queue associated with semaphore semAdd.
 * 
 * @param semAdd Semaphore identifier
 * @return The head of the process list associated with the
 * semaphore semAdd if it isn't empty, NULL otherwise.
 */
pcb_t* headBlocked(int *semAdd);

/**
 * @brief Initializes the ASLs.
 * 
 * @remark This function should be called before any other
 * ASL-related function.
 */
void initASL();

#endif