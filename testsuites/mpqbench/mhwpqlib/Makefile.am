##
##  $Id$
##


rtems_tests_PROGRAMS = mhwpqlib
mhwpqlib_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h

#dist_rtems_tests_DATA = mhwpqlib.scn
#dist_rtems_tests_DATA += mhwpqlib.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqlib_OBJECTS) $(mhwpqlib_LDADD)
LINK_LIBS = $(mhwpqlib_LDLIBS)

mhwpqlib$(EXEEXT): $(mhwpqlib_OBJECTS) $(mhwpqlib_DEPENDENCIES)
	@rm -f mhwpqlib$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
