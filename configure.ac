AC_INIT([teno], [1.0], [lostnetworking@gmail.com])
AC_PREREQ(2.5)


host_alias=msp430
host=msp430
cross_compiling=yes
ac_tool_prefix=msp430-
INSTALL=mspdebug
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(1.10)

# ${CFLAGS=-mmcu=msp430fr5739}
CFLAGS=-mmcu=msp430fr5739
AC_PROG_CC

AC_PROG_INSTALL(mspdebug)

AC_ARG_ENABLE(gpio,
[ --enable-gpio enable gpio keyboard],
[case "${enableval}" in
	yes) gpio=true ;;
	no)  gpio=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-gpio]) ;;
esac],[gpio=true])
AM_CONDITIONAL(GPIO, [test x$gpio = xtrue])

AC_ARG_ENABLE(dongle,
[ --enable-dongle=name enable name debug dongle],
[case "${enableval}" in
	yes) dongle=rf2500;;
	no)  dongle=false ;;
	*)   dongle=${enableval} ;;
esac],[dongle=rf2500])
AC_SUBST(DONGLE, $dongle)

AC_ARG_ENABLE(gemeni,
[ --enable-gemeni enable gemeniPR protocol],
[case "${enableval}" in
	yes) gemeni=true ;;
	no)  gemeni=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-gemeni]) ;;
esac],[gemeni=false])
AM_CONDITIONAL(GEMENI, [test x$gemeni = xtrue])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
