#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include <umps3/umps/libumps.h>

#include "pandos_types.h"

//#define DEBUGON

#define MAXPROC	20
#define	MAXSEM	MAXPROC


#define TRANSMITTED	5
#define ACK	1
#define PRINTCHR	2
#define CHAROFFSET	8
#define STATUSMASK	0xFF
#define	TERM0ADDR	0x10000254

typedef unsigned int devreg;

/* This function returns the terminal transmitter status value given its address */ 
devreg termstat(memaddr * stataddr);

/* This function prints a string on specified terminal and returns TRUE if 
 * print was successful, FALSE if not   */
unsigned int termprint(char * str, unsigned int term);


/* This function placess the specified character string in okbuf and
*	causes the string to be written out to terminal0 */
void addokbuf(char *strp);


void printHex(unsigned int n);
void printDec(unsigned int n);

void deadBreak();
void exBreak();
void intBreak();

#endif