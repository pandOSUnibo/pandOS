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

typedef struct pteEntry_t {
    unsigned int pte_entryHI;
    unsigned int pte_entryLO;
} pteEntry_t;


typedef struct context_t {
    unsigned int c_stackPtr;
    unsigned int c_status;
    unsigned int c_pc;
} context_t;


typedef struct support_t {
    int       sup_asid;             ///< Process ID
    state_t   sup_exceptState[2];   ///< Old state exceptions
    context_t sup_exceptContext[2]; ///< New contexts for passing up
    pteEntry_t sup_privatePgTbl[USERPGTBLSIZE]; ///< User page table
    int sup_stackTLB[500];
    int sup_stackGen[500];
} support_t;


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
                    *p_next_sib,    ///< Pointer to next sibling. 
                    *p_prev_sib;    ///< Pointer to previous sibling.

    // Process status information
    state_t     p_s;    ///< Processor state.
    cpu_t p_time;    ///< Cpu time used by process.
    int *p_semAdd;    ///< Pointer to semaphore on which process is blocked.
    support_t *p_supportStruct; ///< Support structure.
    // Add more fields here

} pcb_t, *pcb_PTR;

/**
 * @brief Represents a size.
 */
typedef unsigned int size_t;

/**
 * @brief Represents a boolean value.
 */
typedef int bool;

/**
 * @brief Represents a semaphore.
 */
typedef int semaphore;

/* Page swap pool information structure type */
typedef struct swap_t {
    int         sw_asid;   ///< ASID number
    int         sw_pageNo; ///< Page's virtual page number
    pteEntry_t *sw_pte;    ///< Page's PTE entry
} swap_t;
#endif
