#ifndef PCB_H
#define PCB_H

#include "pandos_const.h"
#include "pandos_types.h"

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

pcb_t* mkEmptyProcQ();

int emptyProcQ(pcb_t *tp);

void insertProcQ(pcb_t **tp, pcb_t *p);

/**
 * @brief Return a pointer to the pcb that is the head 
 * of the process queue pointed by tp, without removing it. 
 * 
 * @param tp Pointer to the tail of the process queue to operate on.
 * @return A pointer to the process at the head position in the queue.
 * If the queue is empty (tp is NULL) return NULL
 */
pcb_t* headProcQ(pcb_t *tp);

pcb_t* removeProcQ(pcb_t **tp);

/**
 * @brief Remove and return the pcb pointed by p from the 
 * process queue pointed by tp.
 * 
 * @param tp Pointer to the pointer of the queue (sentinel).
 * @param p Pointer to the pcb to be removed.
 * @return A pointer to the removed pcb.
 * If the pcb pointed by p is not in the queue return NULL.
 */
pcb_t* outProcQ(pcb_t **tp, pcb_t *p);

int emptyChild(pcb_t *p);

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

pcb_t* outChild(pcb_t *p);

#endif