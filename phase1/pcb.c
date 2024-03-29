#include <umps3/umps/const.h>

#include "pcb.h"

/**
 * @brief A static array containing all
 * possible process descriptors.
 * 
 */
HIDDEN pcb_t pcbFree_table[MAXPROC];
/**
 * @brief List of all free process
 * descriptors.
 * 
 */
HIDDEN pcb_t *pcbFree_h;

void initPcbs() {
    pcbFree_h = &(pcbFree_table[0]);

    // Create the pcbFree list
    for (int i = 0; i < MAXPROC - 1; ++i) {
        pcbFree_table[i].p_next = &(pcbFree_table[i + 1]);
    }
    pcbFree_table[MAXPROC - 1].p_next = NULL;

    // The other fields of pcb_t are not
    // initialized because they are handled
    // by allocPcb()
}

void freePcb(pcb_t *p) {
   p->p_next = pcbFree_h;
   pcbFree_h = p; 
}

pcb_t* allocPcb() {
    pcb_t *head = NULL;
    // Check for available memory
    if (pcbFree_h != NULL){
        head = pcbFree_h;
        pcbFree_h = head->p_next;
        
        // Clean PCB 
        head->p_next = NULL;
        head->p_prev = NULL;
        head->p_prnt = NULL;
        head->p_child = NULL;
        head->p_next_sib = NULL;
        head->p_prev_sib = NULL;
        head->p_semAdd = NULL;
        head->p_time = 0;
        
        // Clean p_s
        head->p_s.cause = 0;
        head->p_s.entry_hi = 0;
        for (int i = 0; i < STATE_GPR_LEN; ++i) {
            head->p_s.gpr[i] = 0;
        }
        head->p_s.hi = 0;
        head->p_s.lo = 0;
        head->p_s.pc_epc = 0;
        head->p_s.status = 0;

        // Clean p_supportStruct
        head->p_supportStruct = NULL;
    }
    return head;
}

pcb_t* mkEmptyProcQ() {
    // An empty process queue is a NULL pointer
    return NULL;
}

int emptyProcQ(pcb_t *tp) {
    // An empty process queue is equal to a NULL pointer
    return tp == NULL;
}


pcb_t* headProcQ(pcb_t *tp){
    if (tp == NULL) return NULL;

    // Previous element from tail is head
    return tp->p_prev;
}

pcb_t* outProcQ(pcb_t **tp, pcb_t *p){
    if (*tp == NULL) return NULL;

    pcb_t *toRemove = *tp;
    int found = FALSE;
    do {
        // Comparison made between memory location
        if (toRemove == p) {
            found = TRUE;
            break;
        }

        toRemove = toRemove->p_next;

    } while (toRemove != *tp);

    // Case of PCB not in given queue
    if (!found) return NULL;

    // Case of circular list with single element
    if (toRemove->p_next == toRemove) *tp = NULL;
    else {

        // Case of element to be removed pointed by sentinel (first element)
        if (toRemove == *tp) *tp = (*tp)->p_next;

        // Changing pointers of previous and next PCBs in the list
        toRemove->p_prev->p_next = toRemove->p_next;
        toRemove->p_next->p_prev = toRemove->p_prev;
    }

    // Resetting pointers of removed process
    toRemove->p_next = NULL;
    toRemove->p_prev = NULL;

    return toRemove;
}

void insertProcQ(pcb_t **tp, pcb_t *p) {
    // Performs a head-insert
    if (*tp == NULL) {
        // The list is empty
        *tp = p;
        p->p_prev = p;
        p->p_next = p;
        return;
    }

    p->p_next = *tp;
    p->p_prev = (*tp)->p_prev;
    p->p_prev->p_next = p;
    (*tp)->p_prev = p;
    *tp = p;
}

pcb_t* removeProcQ(pcb_t **tp) {
    if(*tp == NULL) return NULL;

    pcb_t *toRemove = (*tp)->p_prev;

    // Case of a circular list with single element
    if (toRemove->p_prev == toRemove) *tp = NULL;
    else {
        (*tp)->p_prev = toRemove->p_prev;
        toRemove->p_prev->p_next = *tp;
    }

    // Resetting pointers of removed process
    toRemove->p_next = NULL;
    toRemove->p_prev = NULL;

    return toRemove;
}

int emptyChild(pcb_t *p) {
    return p->p_child == NULL;
}

void insertChild(pcb_t *prnt, pcb_t *p) {
    // Perform a head-insert to the children
    // list

    if (prnt->p_child != NULL) {
        // The children list is empty
        prnt->p_child->p_prev_sib = p;
    }

    p->p_prnt = prnt;
    p->p_next_sib = prnt->p_child;
    p->p_prev_sib = NULL;
    prnt->p_child = p;
}

pcb_t* removeChild(pcb_t *p) {
    // Perform a head-remove to the
    // children list

    if (p->p_child == NULL) {
        return NULL;
    }
    else {
        pcb_t *child = p->p_child;
        p->p_child = child->p_next_sib;

        if (child->p_next_sib != NULL){
            child->p_next_sib->p_prev_sib = NULL;
        }

        child->p_prnt = NULL;
        child->p_next_sib = NULL;

        return child;
    }
}

pcb_t* outChild(pcb_t *p) {
    if (p->p_prnt == NULL) {
        // p has no parent
        return NULL;
    }

    if (p->p_prev_sib != NULL) {
        p->p_prev_sib->p_next_sib = p->p_next_sib;
    }
    
    if (p->p_next_sib != NULL) {
        p->p_next_sib->p_prev_sib = p->p_prev_sib;
    }

    // If p was the first child, p->p_next_sib
    // becomes the new first child
    if (p->p_prnt->p_child == p) {
        p->p_prnt->p_child = p->p_next_sib;
    }

    p->p_prev_sib = NULL;
    p->p_next_sib = NULL;
    p->p_prnt = NULL;

    return p;
}
