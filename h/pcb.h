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

pcb_t* headProcQ(pcb_t *tp);

pcb_t* removeProcQ(pcb_t **tp);

pcb_t* outProcQ(pcb_t **tp, pcb_t *p);

int emptyChild(pcb_t *p);

void insertChild(pcb_t *prnt, pcb_t *p);

pcb_t* removeChild(pcb_t *p);

pcb_t* outChild(pcb_t *p);

#endif