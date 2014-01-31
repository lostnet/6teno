bin_PROGRAMS = teno
EXTRA_teno_SOURCES = gpiokeys.c gemenipr.c

if GPIO
 gpio = gpiokeys.c
else
 gpio =
endif
if GEMENI
 gemeni = gemenipr.c
else
 gemeni =
endif
teno_SOURCES = fwork.c $(gemeni) $(gpio)

install:
	 $(INSTALL) $(INSTALLARGS) $(DONGLE) "load teno"
