AC_PREREQ([2.69])
AC_INIT([compass], [0.1], [gkobeaga@bcamath.org])

AC_CONFIG_SRCDIR([src/compass.h])

AC_CONFIG_MACRO_DIR([m4])

dnl AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_HEADERS([config.h])

dnl Check for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Checks for libraries.
AC_CONFIG_MACRO_DIR([m4])

LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([ stdlib.h math.h string.h strings.h errno.h assert.h stddef.h unistd.h malloc.h sys/types.h sys/stat.h fcntl.h signal.h sys/socket.h netinet/in.h netdb.h sys/resource.h sys/param.h sys/times.h ])


dnl sys/time.h time.h

dnl Check for <sys/time.h> header
AC_CHECK_HEADER([sys/time.h],
  AC_DEFINE([HAVE_SYS_TIME_H], [1], [N/A]))

dnl Check for gettimeofday function
AC_CHECK_FUNC([gettimeofday],
   AC_DEFINE([HAVE_GETTIMEOFDAY], [1], [N/A]))

AC_CONFIG_LINKS([ src/compass.h:src/compass.h
                  src/env/env.h:src/env/env.h
                  src/env/stdc.h:src/env/stdc.h
                  src/util/dmp.h:src/util/dmp.h
                  src/data/neigh.h:src/data/neigh.h
                  src/tsp/tsp.h:src/tsp/tsp.h
                  src/op/op.h:src/op/op.h
                  src/op/ea/ea.h:src/op/ea/ea.h
                 ])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostbyname gethostname memset pow socket sqrt])

AC_OUTPUT(Makefile
          src/Makefile
          )
