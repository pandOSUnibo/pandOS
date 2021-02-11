#ifndef PCB_H
#define PCB_H

#include "pandos_const.h"
#include "pandos_types.h"

/**
 * @brief Initializes the PCBs.
 * 
 * @remark This function should be called
 * before any other PCB-related function.
 */
void initPcbs();

/**
 * @brief Return the pcb to memory.
 * The pcb can't be used anymore.
 * 
 * @param p pcb to be released
 */
void freePcb(pcb_t *p);

/**
 * @brief Allocate a pcb and return it,
 * if no more memory is avaible return NULL.
 * All the records are cleaned before the allocation. 
 * 
 * @return pcb_t* 
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
 * @param tp 
 * @return TRUE if tp is empty, FALSE otherwise.
 * 
 * @remark TRUE and FALSE are defined in
 * umps3/umps/const.h.
 */
int emptyProcQ(pcb_t *tp);

void insertProcQ(pcb_t **tp, pcb_t *p);

/**
 * @brief Returns a pointer to the pcb that is the head 
 * of the process queue pointed by tp, without removing it. 
 * 
 * @param tp Pointer to the tail of the process queue to operate on.
 * @return A pointer to the process at the head position in the queue.
 * If the queue is empty (tp is NULL) return NULL
 */
pcb_t* headProcQ(pcb_t *tp);

pcb_t* removeProcQ(pcb_t **tp);

/**
 * @brief Removes and return the pcb pointed by p from the 
 * process queue pointed by tp.
 * 
 * @param tp Pointer to the pointer of the queue (sentinel).
 * @param p Pointer to the pcb to be removed.
 * @return A pointer to the removed pcb.
 * If the pcb pointed by p is not in the queue return NULL.
 */
pcb_t* outProcQ(pcb_t **tp, pcb_t *p);

/**
 * @brief Checks whether the pcb pointed by p has children.
 * 
 * @param p Pointer to the pcb to be checked.
 * @return TRUE if the pcb pointed by p doesn't have any
 * children, FALSE otherwise.
 */
int emptyChild(pcb_t *p);

/**
 * @brief Inserts the pcb pointed by p as a child of the
 * pcb pointed by prnt.
 * 
 * @param prnt Pointer to the pcb who will become parent of p.
 * @param p Pointer to the pcb who will become child of prnt.
 */
void insertChild(pcb_t *prnt, pcb_t *p);

/**
 * @brief Remove and return the first child of
 * the pcb pointed by p.
 * 
 * @param p Pointer to the pcb whose first child will be removed.
 * @return A pointer to the first child of the pcb
 * pointed by p. If p has no children return NULL.
 */
pcb_t* removeChild(pcb_t *p);

/**
 * @brief Removes and return the pcb pointed by p from the list of
 * his parent's children.
 * 
 * @param p Pointer to the pcb that will be removed and returned.
 * @return A pointer to the removed pcb. If the pcb pointed by p
 * doesn't have any parent return NULL.
 */
pcb_t* outChild(pcb_t *p);

#endif