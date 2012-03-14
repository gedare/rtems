##
##  $Id$
##


rtems_tests_PROGRAMS = mhwpq
mhwpq_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h

#dist_rtems_tests_DATA = mhwpq.scn
#dist_rtems_tests_DATA += mhwpq.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpq_OBJECTS) $(mhwpq_LDADD)
LINK_LIBS = $(mhwpq_LDLIBS)

mhwpq$(EXEEXT): $(mhwpq_OBJECTS) $(mhwpq_DEPENDENCIES)
	@rm -f mhwpq$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
