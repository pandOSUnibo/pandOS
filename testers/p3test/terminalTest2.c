/* Does nothing but outputs to the printer and terminates */

#include "umps3/umps/libumps.h"

#include "h/tconst.h"
#include "h/print.h"

void main() {
	print(WRITEPRINTER, "printTest is ok\n");
	
	print(WRITETERMINAL, "Test Number 2 is ok\n");
	
	SYSCALL(TERMINATE, 0, 0, 0);
}
