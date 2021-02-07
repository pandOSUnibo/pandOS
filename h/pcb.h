#ifndef PANDOS_PCB_H_INCLUDED
#define PANDOS_PCB_H_INCLUDED

#include "pandos_const.h"
#include "pandos_types.h"

/**
 * @brief Initializes the PCBs.
 * 
 */
void initPcbs();

/**
 * @brief Frees a certain PCB.
 * 
 * @param p A pointer to the PCB to be freed.
 */
void freePcb(pcb_t *p);

/**
 * @brief Allocates a PCB.
 * 
 * @return pcb_t* A pointer to the allocated PCB.
 */
pcb_t *allocPcb();

pcb_t *mkEmptyProcQ();

int emptyProcQ(pcb_t *tp);

void insertProcQ(pcb_t **tp, pcb_t *p);

pcb_t *headProcQ(pcb_t *tp);

pcb_t *removeProcQ(pcb_t **tp);

pcb_t *outProcQ(pcb_t **tp, pcb_t *p);

int emptyChild(pcb_t *p);

void insertChild(pcb_t *prnt,pcb_t *p);

pcb_t* removeChild(pcb_t *p);

pcb_t *outChild(pcb_t* p);

#endif