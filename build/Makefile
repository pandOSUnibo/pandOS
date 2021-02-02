# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld


# uMPS3-related paths

# Simplistic search for the umps3 installation prefix.
# If you have umps3 installed on some weird location, set UMPS3_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

TESTERS_DIR = ../testers
PHASE1_DIR = ../phase1

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3

# Compiler options
CFLAGS_LANG = -ffreestanding
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS3_INCLUDE_DIR) -Wall -O0 -I../h/

# Linker options
LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript

# Add the location of crt*.S to the search path
VPATH = $(UMPS3_DATA_DIR)

.PHONY : all clean

all : kernel.core.umps

kernel.core.umps : kernel
	umps3-elf2umps -k $<

kernel : p1test.o asl.o pcb.o crtso.o libumps.o
	$(LD) -o $@ $^ $(LDFLAGS) 

p1test.o: $(TESTERS_DIR)/p1test.c 
	$(CC) $(CFLAGS) -c -o $@ $<
	
asl.o : $(PHASE1_DIR)/asl.c 
	$(CC)  $(CFLAGS) -c -o $@ $<
pcb.o : $(PHASE1_DIR)/pcb.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean :
	-rm -f *.o kernel kernel.*.umps

# Pattern rule for assembly modules
%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<