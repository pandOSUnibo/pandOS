<<<<<<< HEAD
#ifndef PANDOS_ASL_H_INCLUDED
#define PANDOS_ASL_H_INCLUDED

#include "pandos_const.h"
#include "pandos_types.h"

int insertBlocked(int *semAdd,pcb_t *p);
=======
#ifndef ASL_H
#define ASL_H

#include "pandos_types.h"

int insertBlocked(int *semAdd, pcb_t *p);
>>>>>>> ca6294b0cd0475c779f6465966a1a87caaa95b71

pcb_t* removeBlocked(int *semAdd);

pcb_t* outBlocked(pcb_t *p);

pcb_t* headBlocked(int *semAdd);

void initASL();

#endif