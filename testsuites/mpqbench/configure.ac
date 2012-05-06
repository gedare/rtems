## Process this file with autoconf to produce a configure script.
## 
## $Id: configure.ac,v 1.29 2007/08/17 01:07:10 ccj Exp $

AC_PREREQ(2.60)
AC_INIT([rtems-c-src-tests-mpqbench],[_RTEMS_VERSION],[http://www.rtems.org/bugzilla])
AC_CONFIG_SRCDIR([miheap])
RTEMS_TOP([../..],[..])

RTEMS_CANONICAL_TARGET_CPU

AM_INIT_AUTOMAKE([no-define foreign 1.10])
AM_MAINTAINER_MODE

RTEMS_ENABLE_CXX

RTEMS_ENV_RTEMSBSP

RTEMS_PROJECT_ROOT

RTEMS_PROG_CC_FOR_TARGET

RTEMS_PROG_CXX_FOR_TARGET
RTEMS_CANONICALIZE_TOOLS

RTEMS_CHECK_CUSTOM_BSP(RTEMS_BSP)
RTEMS_CHECK_CPUOPTS([RTEMS_MULTIPROCESSING])
RTEMS_CHECK_CXX(RTEMS_BSP)
RTEMS_CHECK_CPUOPTS([RTEMS_NETWORKING])


CXXTESTS=$HAS_CPLUSPLUS
AS_IF([test $HAS_CPLUSPLUS = yes],[
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS([cstdio cstdlib iostream],[],[CXXTESTS=no])
  AC_CACHE_CHECK(
  [if iostream supports std:: namespace],
  [rtems_cv_HAVE_IOSTREAM_STD_NAMESPACE],
  [AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
      [[#include <iostream>]], 
      [[std::cout << "hello" << std::endl;]])
    ],
    [rtems_cv_HAVE_IOSTREAM_STD_NAMESPACE=yes],
    [rtems_cv_HAVE_IOSTREAM_STD_NAMESPACE=no])
  ])
  AS_IF([test $rtems_cv_HAVE_IOSTREAM_STD_NAMESPACE = no],[CXXTESTS=no])
  AS_IF([test $CXXTESTS = no],[
    AC_MSG_WARN([disabling C++ tests])
  ])
  AC_LANG_POP
])

AM_CONDITIONAL([CXXTESTS],[test $CXXTESTS = "yes"])
AM_CONDITIONAL(NETTESTS,test "$rtems_cv_RTEMS_NETWORKING" = "yes")
AM_CONDITIONAL(MPTESTS,test "$rtems_cv_RTEMS_MULTIPROCESSING" = "yes")

## GAB: hack to check for custom code
AM_CONDITIONAL([sun4u],[test x"$RTEMS_CPU_MODEL" = x"sun4u"])

#FIXME: We should use a feature based check
AM_CONDITIONAL(LIBCHIP,[test "$RTEMS_BSP_FAMILY" != "posix"])

# Explicitly list all Makefiles here
AC_CONFIG_FILES([Makefile
mbptree/Makefile
mdetskiplist/Makefile
mhwpq/Makefile
mhwpqsplitheap/Makefile
mhwpqsplitrbtree/Makefile
mhwpqunitedlist/Makefile
mhwpqunitedsearch/Makefile
miheap/Makefile
mlist/Makefile
mrbtree/Makefile
mskiplist/Makefile
msplay/Makefile
mstlheap/Makefile
])

AC_OUTPUT
