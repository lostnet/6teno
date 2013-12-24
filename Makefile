CC = msp430-gcc
CCARGS = -Wall
PROG = mspdebug
PROGARGS = -v 3800 # --force-reset
PROGDONGLE = rf2500
DEFS = -mmcu="msp430fr5739"
.SUFFIXES: .c .elf .inst .clr


.c.elf:
	$(CC) $(CCARGS) $(DEFS) -o $@ $*.c

.elf.inst:
	 $(PROG) $(PROGARGS) $(PROGDONGLE) "load $*.elf"; date > $@

.inst.clr:
	 $(PROG) $(PROGARGS) $(PROGDONGLE) "fill 0xca00 64 0"; date > $@
