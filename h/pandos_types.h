/**
 * @file pandos_types.h
 * @brief This header file contains utility types definitions.
 * 
 */

#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

#include <umps3/umps/types.h>
#include "pandos_const.h"

typedef signed int cpu_t;

/**
 * @brief Represents a memory address.
 * 
 */
typedef unsigned int memaddr;

/**
 * @struct pcb_t
 * @brief Process Control Block.
 * 
 * @remark When inside a queue, by convention
 * p_next points to the PCB that will exit the queue
 * before it, while p_prev points to the PCB that will
 * exit the queue after it.
 * 
 */
typedef struct pcb_t {
    // Process queue fields
    struct pcb_t   *p_next;            ///< Pointer to next entry.
    struct pcb_t   *p_prev;         ///< Pointer to previous entry.

    // Process tree fields
    struct pcb_t    *p_prnt,         ///< Pointer to parent.
                    *p_child,        ///< Pointer to 1st child.
                    *p_next_sib,    ///< Pointer to next sibling         
                    *p_prev_sib;    ///< Pointer to previous sibling.
    
    // Process status information
    state_t     p_s;    ///< Processor state.
    cpu_t p_time;    ///< Cpu time used by process.
    int *p_semAdd;    ///< Pointer to semaphore on which process is blocked.
    // Add more fields here 

}  pcb_t, *pcb_PTR;
    
#endif
