/**
 * @file exceptions.h
 * @author Luca Donno, Antonio Lopez, Samuele Marro, Edoardo Merli
 * @brief Exceptions handler
 * @version 0.2.0
 * @date 2021-03-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#define INT 0 ///< External Device Interrupt
#define MOD 1 ///< TLB-Modification Exception
#define TLBL 2 ///< TLB Invalid Exception: on a Load instr. or instruction fetch
#define TLBS 3 ///< TLB Invalid Exception: on a Store instr.
#define ADEL 4 ///< Address Error Exception: on a Load or instruction fetch
#define ADES 5 ///< Address Error Exception: on a Store instr.
#define IBE 6 ///< Bus Error Exception: on an instruction fetch
#define DBE 7 ///< Bus Error Exception: on a Load/Store data access
#define SYS 8 ///< Syscall Exception
#define BP 9 ///< Breakpoint Exception
#define RI 10 ///< Reserved Instruction Exception
#define CPU 11 ///< Coprocessor Unusable Exception
#define OV 12 ///< Arithmetic Overflow Exception

#include "initial.h"

void exceptionHandler();

#endif