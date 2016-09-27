#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([audiocapturemgr], [0.10], [chaithen_sojan@cable.comcast.com])
AC_CONFIG_SRCDIR([src/audio_capture_manager.cpp])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
LT_INIT
LT_LANG([C++])
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_ARG_ENABLE([testapp],
              AS_HELP_STRING([--enable-testapp],[enable testapp]),
              [testapp=true;echo "testapp is enabled";],
              [testapp=false;echo "testapp is disabled";])
AM_CONDITIONAL([ENABLE_TESTAPP], [test x$testapp = xtrue])
AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT
