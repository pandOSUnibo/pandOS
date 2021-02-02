#ifndef PANDOS_ASL_H_INCLUDED
#define PANDOS_ASL_H_INCLUDED

#include "pandos_const.h"
#include "pandos_types.h"

int insertBlocked(int *semAdd,pcb_t *p);

pcb_t* removeBlocked(int *semAdd);

pcb_t* outBlocked(pcb_t *p);

pcb_t* headBlocked(int *semAdd);

void initASL();

#endif