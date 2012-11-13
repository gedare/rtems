##
##  $Id$
##


rtems_tests_PROGRAMS = mhwpqlib2
mhwpqlib2_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h

#dist_rtems_tests_DATA = mhwpqlib2.scn
#dist_rtems_tests_DATA += mhwpqlib2.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqlib2_OBJECTS) $(mhwpqlib2_LDADD)
LINK_LIBS = $(mhwpqlib2_LDLIBS)

mhwpqlib2$(EXEEXT): $(mhwpqlib2_OBJECTS) $(mhwpqlib2_DEPENDENCIES)
	@rm -f mhwpqlib2$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
