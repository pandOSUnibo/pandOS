#include <umps3/umps/libumps.h>

#include "pandos_const.h"

#include "exceptions.h"
#include "initial.h"
#include "interrupts.h"
#include "scheduler.h"
#include "syscalls.h"

#define INT  0			///< External Device Interrupt
#define MOD  1			///< TLB-Modification Exception
#define TLBL 2			///< TLB Invalid Exception: on a Load instr. or instruction fetch
#define TLBS 3			///< TLB Invalid Exception: on a Store instr.
#define ADEL 4			///< Address Error Exception: on a Load or instruction fetch
#define ADES 5			///< Address Error Exception: on a Store instr.
#define IBE  6			///< Bus Error Exception: on an instruction fetch
#define DBE  7			///< Bus Error Exception: on a Load/Store data access
#define SYS  8			///< Syscall Exception
#define BP   9			///< Breakpoint Exception
#define RI   10			///< Reserved Instruction Exception
#define CPU  11			///< Coprocessor Unusable Exception
#define OV   12			///< Arithmetic Overflow Exception


cpu_t elapsedTime() {
	cpu_t clockTime;

	STCK(clockTime);
	return clockTime - sliceStart;
}

/**
 * @brief Pass Up or Die, for handling exceptions numbered 9 and above,
 * Program Trap and TLB exceptions. Which action between the 2 is taken 
 * depends on whether the current process was proveded a NULL value for
 * its Support Structure pointer when it was created or not.
 * 
 * @param index Either 0 (PGFAULTEXCEPT) or 1 (GENERALEXCEPT), indicating
 * that the exception to be handled is respectively a TLB exception or any
 * of the other cases listed above.
 */
HIDDEN void passUpOrDie(int index) {
	support_t *supportStructure = currentProcess->p_supportStruct;

	if (supportStructure == NULL) {
		// Die: Kill the process using a SYS2-like call
		termProcess();
	}
	else {
		// Pass up the exception to the Support Level
		supportStructure->sup_exceptState[index] = *EXCSTATE;
		context_t *context = &(supportStructure->sup_exceptContext[index]);
		LDCXT(context->c_stackPtr, context->c_status, context->c_pc);
	}
}

/**
 * @brief Copies bytes from an address to
 * another.
 * 
 * @param dest Destination address.
 * @param src Source address.
 * @param len Length of the bytes to be copied.
 * @return void* A pointer to the destination
 * address.
 * @remark This function is also added by the compiler
 * in order to support struct copying.
 */
void* memcpy(void *dest, const void *src, size_t len) {
	char *d = dest;
	const char *s = src;

	while (len--) {
		*d++ = *s++;
	}
	return dest;
}

void resume() {
	if (currentProcess == NULL)
		schedule();
	else
		LDST(EXCSTATE);
}

/**
 * @brief Handles a TLB exception.
 *  
 */
HIDDEN void TLBExceptionHandler() {
	passUpOrDie(PGFAULTEXCEPT);
}

/**
 * @brief Handles a Program Trap.
 * 
 */
HIDDEN void trapHandler() {
	passUpOrDie(GENERALEXCEPT);
}

/**
 * @brief Handles syscalls.
 * 
 * @param KUp Value of the KUp bit in the CAUSE
 * register.
 */
HIDDEN void syscallHandler(unsigned int KUp) {
	unsigned int sysId = EXCSTATE->reg_a0;

	// Get arguments for syscalls
	unsigned int arg1 = EXCSTATE->reg_a1;
	unsigned int arg2 = EXCSTATE->reg_a2;
	unsigned int arg3 = EXCSTATE->reg_a3;
	memaddr resultAddress = (memaddr) &(EXCSTATE->reg_v0);

	if (sysId <= 8) {
		// KUp is 0 in kernel mode and 0x00000008
		// in user mode
		if (KUp == 0) {
			switch (sysId) {
			case CREATEPROCESS:
				createProcess((state_t *) arg1, (support_t *) arg2);
				break;
			case TERMPROCESS:
				termProcess();
				break;
			case PASSEREN:
				passeren((int *) arg1);
				break;
			case VERHOGEN:
				verhogen((int *) arg1);
				break;
			case IOWAIT:
				ioWait(arg1, arg2, arg3);
				break;
			case GETTIME:
				getTime((cpu_t *) resultAddress);
				break;
			case CLOCKWAIT:
				clockWait();
				break;
			case GETSUPPORTPTR:
				getSupportPtr((support_t **) resultAddress);
				break;
			default:
				// Invalid SYSCALL ID: Kill
				termProcess();
				break;
			}

			// The process was not blocked or put in the ready
			// queue
			resume();
		}
		else {
			// Attempted to perform a kernel syscall
			// while in user mode. Call a trap with cause
			// RI (Reserved Instruction)
			EXCSTATE->cause &= ~GETEXECCODE;
			EXCSTATE->cause |= RI << CAUSESHIFT;
			trapHandler();
		}
	}
	else {
		// Pass to the Support Level
		passUpOrDie(GENERALEXCEPT);
	}
}


void exceptionHandler() {
	state_t *exceptionState = EXCSTATE;

	unsigned int cause = (exceptionState->cause & GETEXECCODE) >> CAUSESHIFT;
	// Increment the PC by one word so that when control returns to the
	// process, it does not perform a syscall again
	exceptionState->pc_epc += WORDLEN;

	switch (cause) {
	case INT:
		// Interrupt
		interruptsHandler(exceptionState);
		break;
	case MOD:
	case TLBL:
	case TLBS:
		TLBExceptionHandler();
		break;
	case ADEL:
	case ADES:
	case IBE:
	case DBE:
	case BP:
	case RI:
	case CPU:
	case OV:
		// Traps
		trapHandler();
		break;
	case SYS:
		// Syscalls
		syscallHandler(exceptionState->status & USERPON);
		break;
	default:
		PANIC();
		break;
	}
}
