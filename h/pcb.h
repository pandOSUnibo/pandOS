/**
 * @file pcb.h
 * @authors Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Process Control Block methods.
 * @version 0.2.0
 * @date 2021-02-13
 * 
 */

#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/**
 * @brief Initializes the PCBs.
 * 
 * @remark This function should be called
 * before any other PCB-related function.
 */
void initPcbs();

/**
 * @brief Deallocates the PCB.
 * 
 * @param p PCB to be released.
 * 
 * @remark After calling this function,
 * the PCB can't be used anymore.
 */
void freePcb(pcb_t *p);

/**
 * @brief Allocates a PCB and returns it.
 * 
 * All the records are cleaned before the allocation. 
 * 
 * @return The allocated PCB. Returns NULL if memory
 * is not available.
 */
pcb_t* allocPcb();

/**
 * @brief Creates a new empty process queue.
 * 
 * @return The new process queue.
 */
pcb_t* mkEmptyProcQ();

/**
 * @brief Checks if a process queue is empty.
 * 
 * @param tp Queue to be checked.
 * @return TRUE if tp is empty, FALSE otherwise.
 * 
 * @remark TRUE and FALSE are defined in
 * umps3/umps/const.h.
 */
int emptyProcQ(pcb_t *tp);

/**
 * @brief Inserts a PCB in a queue.
 * 
 * @param tp Pointer to the queue (sentinel).
 * @param p Pointer to the PCB to be inserted.
 */
void insertProcQ(pcb_t **tp, pcb_t *p);

/**
 * @brief Returns a pointer to the head of the
 * process queue, without removing it. 
 * 
 * @param tp Pointer to the tail of the process queue.
 * @return A pointer to the head of the queue.
 * Returns NULL if the queue is empty.
 */
pcb_t* headProcQ(pcb_t *tp);

/**
 * @brief Removes the head of the PCB queue.
 * 
 * @param tp Pointer to the queue (sentinel).
 * @return A pointer to the removed PCB.
 */
pcb_t* removeProcQ(pcb_t **tp);

/**
 * @brief Removes the PCB pointed by p from the 
 * process queue pointed by tp and returns it.
 * 
 * @param tp Pointer to the queue (sentinel).
 * @param p Pointer to the PCB to be removed.
 * @return A pointer to the removed PCB.
 * Retuns NULL if the PCB pointed by p is not in the queue.
 */
pcb_t* outProcQ(pcb_t **tp, pcb_t *p);

/**
 * @brief Checks whether the PCB pointed by p has children.
 * 
 * @param p Pointer to the PCB to be checked.
 * @return TRUE if the PCB pointed by p doesn't have any
 * children, FALSE otherwise.
 * @remark TRUE and FALSE are defined in
 * umps3/umps/const.h.
 */
int emptyChild(pcb_t *p);

/**
 * @brief Inserts the PCB pointed by p as a child of the
 * PCB pointed by prnt.
 * 
 * @param prnt Pointer to the PCB who will become parent of p.
 * @param p Pointer to the PCB who will become child of prnt.
 */
void insertChild(pcb_t *prnt, pcb_t *p);

/**
 * @brief Remove and return the first child of
 * the PCB pointed by p.
 * 
 * @param p Pointer to the PCB whose first child will be removed.
 * @return A pointer to the first child of the PCB 
 * pointed by p. Returns NULL if p has no children.
 */
pcb_t* removeChild(pcb_t *p);

/**
 * @brief Removes and returns the PCB pointed by p from the list of
 * his parent's children.
 * 
 * @param p Pointer to the PCB that will be removed and returned.
 * @return A pointer to the removed PCB. Returns NULL if the PCB
 * pointed by p doesn't have a parent.
 */
pcb_t* outChild(pcb_t *p);

#endif