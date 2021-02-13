#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

/**************************************************************************** 
 *
 * This header file contains utility types definitions.
 * 
 ****************************************************************************/

#include <umps3/umps/types.h>
#include "pandos_const.h"

typedef signed int cpu_t;
typedef unsigned int memaddr;

/**
 * @brief Process control block
 * 
 */
typedef struct pcb_t {
	// Process queue fields
    struct pcb_t   *p_next;							///< Ptr to next entry			
    struct pcb_t   *p_prev; 						///< ptr to previous entry

	// Process tree fields
	struct pcb_t	*p_prnt, 						///< Ptr to parent				
					*p_child,						///< Ptr to 1st child			
					*p_next_sib,					///< Ptr to next sibling 		
					*p_prev_sib;					///< Ptr to prev. sibling
	
	// Process status information
	state_t     p_s;              ///< Processor state        
	cpu_t p_time;				  ///< Cpu time used by proc
	int *p_semAdd;				  ///< Ptr to semaphore on which proc is blocked 
	// Add more fields here 

}  pcb_t, *pcb_PTR;
	
#endif
