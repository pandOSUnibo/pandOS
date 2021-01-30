#include "pandos_queue.h"

pcb_t *headProcQ(pcb_t **tp){
    if (*tp == NULL) return NULL;
    return (**tp).p_next;
}

pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
    if (*tp == NULL) return NULL;

    pcb_t *toRemove = *tp;
    int found = FALSE;
    do {
        // Comparison made between memory location, not pcb_t content
        // TODO: check if good idea
        if (toRemove == p) {
            found = TRUE;
            break;
        }

        toRemove = (*toRemove).p_next;

    } while ((*toRemove) != *tp);

    // Case of pcb not in given queue
    if (!found) return NULL;

    // Case of circular list with single element
    if ((*toRemove).p_next == toRemove) *tp = NULL;
    else {

        // Case of element to be removed pointed by sentinel (first element)
        if (toRemove == *tp) *tp = (**tp).p_next;

        // Changing pointers of previous and next pcbs in the list
        (*((*toRemove).p_prev)).p_next = (*toRemove).p_next;
        (*((*toRemove).p_next)).p_prev = (*toRemove).p_prev;
    }
}

void insertProcQ(pcb_t **tp, pcb_t *p) {
    if (*tp == NULL) {
        *tp = p;
        (*p).p_next = p;
        (*p).p_prev = p;
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

    pcb_t *toRemove = (**tp).p_prev;

    // Case of a circular list with single element
    if ((*toRemove).p_prev == toRemove) *tp = NULL;
    else {
        (**tp).p_prev = (*toRemove).p_prev;
        (*(*toRemove).p_prev).p_next = *tp;
    }

    // Resetting pointers of removed process
    (*toRemove).p_next = NULL;
    (*toRemove).p_prev = NULL;

    return toRemove;
}
