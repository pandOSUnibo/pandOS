#ifndef PANDOS_QUEUE_H_INCLUDED
#define PANDOS_QUEUE_H_INCLUDED

#include "pandos_const.h"
#include "pandos_types.h"

void initPcbs();

void freePcb(pcb_t *p);

pcb_t *allocPcb();

pcb_t *mkEmptyProcQ();

int emptyProcQ(pcb_t *tp);

void insertProcQ(pcb_t **tp, pcb_t *p);

pcb_t *headProcQ(pcb_t **tp);

pcb_t *removeProcQ(pcb_t **tp);

pcb_t *outProcQ(pcb_t **tp, pcb_t *p);

#endif