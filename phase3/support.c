#include "support.h"
#include "vm_support.h"

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
HIDDEN int stackSup;

/**
 * @brief Deallocates a support structure.
 * 
 * @param support Pointer to the structure that will be
 * deallocated.
 */
void deallocSupport(support_t *support){
    freeSupport[stackSup] = support;
    stackSup++;
}

/**
 * @brief Allocates a support structure.
 * 
 * @return Pointer to the newly allocated structured.
 */
support_t* allocSupport() {
    support_t *sup = NULL;
    if (stackSup != 0){
        stackSup--;
        sup = freeSupport[stackSup];
    }
    return sup;
}

void initSupport() {
    stackSup = 0;
    int i;
    for (i = 0; i < UPROCNUMBER; i++){
        deallocSupport(&support_table[i]);
        dataPages[i] = UNKNOWNDATAPAGE;
    }
}