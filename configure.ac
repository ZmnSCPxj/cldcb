#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cldcb], [0.1A], [ZmnSCPxj@protonmail.com])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE([subdir-objects tar-ustar])
AC_CONFIG_SRCDIR([client/plugin.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_REQUIRE_AUX_FILE([test-driver])
LT_INIT

# Checks for programs.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11
AC_LANG([C++])

AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AX_VALGRIND_CHECK()
AM_CONDITIONAL([USE_VALGRIND], [test x"$enable_valgrind" = xyes])

# Checks for libraries.
AX_PTHREAD([:],[
	AC_MSG_ERROR([Need pthread,])
])
AX_LIB_EV

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netdb.h stddef.h stdlib.h string.h sys/socket.h unistd.h], [:], [
	AC_MSG_ERROR([Missing required header file ${ac_header}.])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2 ftruncate gethostname memset socket strerror], [:], [
	AC_MSG_ERROR([Missing required system function ${ac_func}.])
])

# Outputs.
AC_SUBST([PTHREAD_CC])
AC_SUBST([PTHREAD_CFLAGS])
AC_SUBST([PTHREAD_LIBS])

AC_CONFIG_FILES([Makefile
		 client/Makefile
		 common/Makefile
		 integration/Makefile
                 server/Makefile
		])
AC_CONFIG_SUBDIRS([external/libsodium
                   external/secp256k1
		  ])
AC_OUTPUT
