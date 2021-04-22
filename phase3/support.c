#include "support.h"

HIDDEN support_t support_table[UPROCNUMBER];
// Pointers to the free Support Structure. A dummy node is present.
HIDDEN support_t *freeSupport[UPROCNUMBER+1];
// Pointer to the first block that can store a new free one.
HIDDEN support_t **stackSup;

void deallocSupport(support_t *support){
    *stackSup = support;
    stackSup = (stackSup + sizeof(support_t*));
}

support_t* allocSupport() {
    support_t *sup = NULL;
    if (stackSup != freeSupport){
        stackSup = (stackSup - sizeof(support_t*));
        sup = stackSup;
    }
    return sup;
}

void initSupport() {
    stackSup = freeSupport;
    int i ;
    for (i = 0; i < UPROCNUMBER; i++){
        deallocSupport(&support_table[i]);
    }
}