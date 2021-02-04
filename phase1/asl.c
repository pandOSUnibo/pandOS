#include "pcb.h"
#include "asl.h"

#define MAXSEMD MAXPROC + 2

HIDDEN semd_t semd_table[MAXSEMD];
HIDDEN semd_t *semdFree_h;
HIDDEN semd_t *semd_h;

semd_t* findPrevSemd(int *semAdd) {
    semd_t *prev = semd_h;
    while (semAdd > prev->s_next->s_semAdd) {
        prev = prev->s_next;
    }

    return prev;
}

int insertBlocked(int *semAdd, pcb_t *p) {
    semd_t* prev = findPrevSemd(semAdd);
    semd_t* element = prev->s_next;
    if (element->s_semAdd == semAdd){
        insertProcQ(&(element->s_procQ), p);
    }
    else {
        if (semdFree_h == NULL){
            return TRUE;
        }

        semd_t* newSemd = semdFree_h;
        semdFree_h = semdFree_h->s_next;

        prev->s_next = newSemd;
        newSemd->s_next = element;
        newSemd->s_semAdd = semAdd;
        newSemd->s_procQ = mkEmptyProcQ();
        insertProcQ(&(newSemd->s_procQ), p);
    }

    return FALSE;
}

pcb_t* removeBlocked(int *semAdd) {
    semd_t* prev = findPrevSemd(semAdd);
    semd_t* element = prev->s_next;

    if (element->s_semAdd == semAdd){
        pcb_t* first = removeProcQ(&(element->s_procQ));
        if (emptyProcQ(element->s_procQ)){
            prev->s_next = element->s_next;
            element->s_next = semdFree_h;
            semdFree_h = element;
        }

        return first;
    }
    return NULL;
}

pcb_t* outBlocked(pcb_t *p) {
    if (p == NULL) return NULL;

    semd_t *prev = findPrevSemd(p->p_semAdd);
    semd_t *element = prev->s_next;

    // Error condition: semaphore not in ASL
    if(element->s_semAdd != p->p_semAdd) return NULL;

    return outProcQ(&(element->s_procQ), p);
}

pcb_t* headBlocked(int *semAdd) {
    semd_t* prev = findPrevSemd(semAdd);
    semd_t* element = prev->s_next;

    if (element->s_semAdd == semAdd){
        return headProcQ(element->s_procQ);
    }
    return NULL;
}

void initASL() {
    semdFree_h = &(semd_table[2]);
    for (int i = 2; i < MAXSEMD - 1; i++) {
        semd_table[i].s_next = &(semd_table[i + 1]);
    }
    semd_table[MAXSEMD - 1].s_next = NULL;

    semd_h = &semd_table[0];
    semd_h->s_semAdd = MINPOINT;
    semd_h->s_next = &semd_table[1];
    
    semd_table[1].s_next = NULL;
    semd_table[1].s_semAdd = MAXPOINT;
}