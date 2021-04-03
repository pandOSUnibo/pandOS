#include "asl.h"
#include "pcb.h"

#define MAXSEMD (MAXPROC + 2)
/**
 * @struct semd_t
 * @brief Semaphore definition.
 * 
 */
typedef struct semd_t {
    struct semd_t *s_next; ///< Pointer to next element on queue.
    int *s_semAdd;         ///< Pointer to the semaphore.
    
    pcb_t *s_procQ; ///< Pointer to tail of the queue of processes
                    ///< blocked on this semaphore.    
} semd_t;

/**
 * @brief A static array containing all
 * possible semaphore descriptors.
 * 
 */
HIDDEN semd_t semd_table[MAXSEMD];

/**
 * @brief List containing all free
 * semaphore descriptors.
 * 
 * @remark Contains two dummy nodes
 * for better management.
 * 
 */
HIDDEN semd_t *semdFree_h;

/**
 * @brief List containing all active
 * semaphore descriptors.
 * 
 */
HIDDEN semd_t *semd_h;

HIDDEN semd_t* findPrevSemd(int *semAdd) {
    // By returning the previous element instead
    // of the element itself, it is possible
    // to perform more operations on it (such as
    // removing it from the list)
    semd_t *prev = semd_h;
    while (semAdd > prev->s_next->s_semAdd) {
        prev = prev->s_next;
    }

    return prev;
}

int insertBlocked(int *semAdd, pcb_t *p) {
    semd_t *prev = findPrevSemd(semAdd);
    semd_t *element = prev->s_next;

    // If element->s_semAdd == semAdd, the
    // element was found
    if (element->s_semAdd == semAdd) {
        p->p_semAdd = semAdd;
        insertProcQ(&(element->s_procQ), p);
    }
    else {
        if (semdFree_h == NULL){
            // The free list is empty, cannot allocate
            // more semaphore descriptors
            return TRUE;
        }

        // Allocate a new semaphore descriptor
        semd_t *newSemd = semdFree_h;
        semdFree_h = semdFree_h->s_next;

        prev->s_next = newSemd;
        newSemd->s_next = element;
        newSemd->s_semAdd = semAdd;
        newSemd->s_procQ = mkEmptyProcQ();
        p->p_semAdd = semAdd;

        // Insert the p into the semaphore
        // descriptor's process list
        insertProcQ(&(newSemd->s_procQ), p);
    }

    return FALSE;
}

pcb_t* removeBlocked(int *semAdd) {
    semd_t *prev = findPrevSemd(semAdd);
    semd_t *element = prev->s_next;

    // If element->s_semAdd == semAdd, the
    // element was found
    if (element->s_semAdd == semAdd){
        pcb_t *first = removeProcQ(&(element->s_procQ));
        if (emptyProcQ(element->s_procQ)){
            // The process queue is now empty, deallocate
            // the semaphore descriptor
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

    // Get pointer to previous and actual semaphore
    // descriptor in ASL on which p is blocked
    semd_t *prev = findPrevSemd(p->p_semAdd);
    semd_t *element = prev->s_next;

    // Error condition: semaphore not in ASL
    if(element->s_semAdd != p->p_semAdd) return NULL;

    // Remove p from the queue of the semaphore descriptor
    pcb_t *removed = outProcQ(&(element->s_procQ), p);

    // If the queue becomes empty, remove the descriptor from ASL
    if (emptyProcQ(element->s_procQ)) {
        prev->s_next = element->s_next;
        element->s_next = semdFree_h;
        semdFree_h = element;
    }

    return removed;
}

pcb_t* headBlocked(int *semAdd) {
    semd_t *prev = findPrevSemd(semAdd);
    semd_t *element = prev->s_next;

    // If element->s_semAdd == semAdd, the
    // element was found
    if (element->s_semAdd == semAdd){
        return headProcQ(element->s_procQ);
    }
    return NULL;
}

void initASL() {
    // The first two elements of
    // semd_table contain the dummy head
    // and the dummy tail, respectively

    // Points to the first "real" element
    semdFree_h = &(semd_table[2]);

    // Create the free list from the "real"
    // elements
    for (int i = 2; i < MAXSEMD - 1; ++i) {
        semd_table[i].s_next = &(semd_table[i + 1]);
    }
    semd_table[MAXSEMD - 1].s_next = NULL;

    // Create the dummies
    semd_h = &semd_table[0];
    semd_h->s_semAdd = MINPOINT;
    semd_h->s_next = &semd_table[1];
    
    semd_table[1].s_next = NULL;
    semd_table[1].s_semAdd = MAXPOINT;
}