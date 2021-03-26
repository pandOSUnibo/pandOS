#include "debug.h"
#include "pandos_const.h"
#include "pandos_types.h"

char okbuf[2048];			/* sequence of progress messages */
char errbuf[128];			/* contains reason for failing */
char msgbuf[128];			/* nonrecoverable error message before shut down */
int sem[MAXSEM];
int onesem;
pcb_t	*procp[MAXPROC], *p, *qa, *q, *firstproc, *lastproc, *midproc;
char *mp = okbuf;

devreg termstat(memaddr * stataddr) {
	return((*stataddr) & STATUSMASK);
}

/* This function prints a string on specified terminal and returns TRUE if 
 * print was successful, FALSE if not   */
unsigned int termprint(char * str, unsigned int term) {
	memaddr * statusp;
	memaddr * commandp;
	devreg stat;
	devreg cmd;
	unsigned int error = FALSE;
	
	if (term < DEVPERINT) {
		/* terminal is correct */
		/* compute device register field addresses */
		statusp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
		commandp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));
		
		/* test device status */
		stat = termstat(statusp);
		if (stat == READY || stat == TRANSMITTED) {
			/* device is available */
			
			/* print cycle */
			while (*str != EOS && !error) {
				cmd = (*str << CHAROFFSET) | PRINTCHR;
				*commandp = cmd;

				/* busy waiting */
				stat = termstat(statusp);
				while (stat == BUSY)
					 stat = termstat(statusp);
				
				/* end of wait */
				if (stat != TRANSMITTED)
					error = TRUE;
				else
					/* move to next char */
					str++;
			} 
		}
		else
			/* device is not available */
			error = TRUE;
	}
	else
		/* wrong terminal device number */
		error = TRUE;

	return (!error);		
}

#ifdef DEBUGON
/* This function placess the specified character string in okbuf and
*	causes the string to be written out to terminal0 */
void addokbuf(char *strp) {
	char *tstrp = strp;
	while ((*mp++ = *strp++) != '\0')
		;
	mp--;
	termprint(tstrp, 0);
}
#else
void addokbuf(char *strp){}
#endif


void prntnum(unsigned long n, int base, char sign, char *outbuf)
{
    int i = 12;
    int j = 0;

    do{
        outbuf[i] = "0123456789ABCDEF"[n % base];
        i--;
        n = n/base;
    }while( n > 0);

    if(sign != ' '){
        outbuf[0] = sign;
        ++j;
    }

    while( ++i < 13){
       outbuf[j++] = outbuf[i];
    }

    outbuf[j] = 0;

}

void printHex(unsigned int n) {
    char buf[100];
    buf[0] = '0';
    buf[1] = 'x';
    prntnum(n, 16, ' ', buf + 2);
    addokbuf(buf);
}

void printDec(unsigned int n) {
    char buf[100];
    prntnum(n, 10, ' ', buf);
    addokbuf(buf);
}