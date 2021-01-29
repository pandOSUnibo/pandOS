#include "pandos_queue.h"

void insertProcQ(pcb_t **tp, pcb_t *p) {
    if (*tp == NULL) {
        *tp = p;
        return;
    }

    (*p).p_next = *tp;
    (*p).p_prev = (**tp).p_prev;
    (*((*p).p_prev)).p_next = p;
    (**tp).p_prev = p;
    *tp = p;
}

pcb_t *removeProcQ(pcb_t **tp) {
    if(*tp == NULL) return NULL;

    pcb_t *toRemove = *tp;
    *tp = (**tp).p_next;
    if (toRemove == *tp) {
        *tp = NULL;
    } else {
        (*((*toRemove).p_prev)).p_next = (*toRemove).p_next;
        (*((*toRemove).p_next)).p_prev = (*toRemove).p_prev; 
    }

    (*toRemove).p_next = NULL;
    (*toRemove).p_prev = NULL;

    return toRemove;
}