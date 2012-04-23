##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqunitedsearch
mhwpqunitedsearch_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h ../shared/hwpqbench.c

#dist_rtems_tests_DATA = mhwpqunitedsearch.scn
#dist_rtems_tests_DATA += mhwpqunitedsearch.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqunitedsearch_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqunitedsearch_OBJECTS) $(mhwpqunitedsearch_LDADD)
LINK_LIBS = $(mhwpqunitedsearch_LDLIBS)

mhwpqunitedsearch$(EXEEXT): $(mhwpqunitedsearch_OBJECTS) $(mhwpqunitedsearch_DEPENDENCIES)
	@rm -f mhwpqunitedsearch$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
