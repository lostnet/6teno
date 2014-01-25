EXTRA_teno_SOURCES = gpiokeys.c gemenipr.c

if GPIO
	gpio = gpiokeys.o
else
	gpio =	
endif
if GEMENI
	gemeni = gemenipr.o
else
	gemeni =	
endif
teno_LDADD = $(gemeni) $(gpio)
teno_SOURCES = main.c
bin_PROGRAMS = teno
