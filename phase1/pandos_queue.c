#include "pandos_queue.h"

pcb_t *headProcQ(pcb_t **tp){
    if (*tp == NULL) return NULL;
    return (**tp).p_next;
}

pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
    if (*tp == NULL) return NULL;

    pcb_t *toRemove = *tp;
    bool found = false;
    do {
        // Comparison made between memory location, not pcb_t content
        // TODO: check if good idea
        if (toRemove == p) {
            found = true;
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

    // Resetting pointers of removed process
    (*toRemove).p_next = NULL;
    (*toRemove).p_prev = NULL;

    return toRemove;

}
