#include "support.h"

/**
 * @brief Contains the support level structures.
 */
HIDDEN support_t support_table[UPROCNUMBER];
/**
 * @brief Stack of free support structure pointers.
 * 
 * @remark Contains a dummy element.
 */
HIDDEN support_t *freeSupport[UPROCNUMBER+1];

/**
 * @brief Top of freeSupport.
 */
HIDDEN support_t **stackSup;

/**
 * @brief Deallocates a support structure.
 * 
 * @param support Pointer to the structure that will be
 * deallocated.
 */
void deallocSupport(support_t *support){
    *stackSup = support;
    stackSup = (stackSup + sizeof(support_t*));
}

/**
 * @brief Allocates a support structure.
 * 
 * @return Pointer to the newly allocated structured.
 */
support_t* allocSupport() {
    support_t *sup = NULL;
    if (stackSup != freeSupport){
        stackSup = (stackSup - sizeof(support_t*));
        sup = *stackSup;
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