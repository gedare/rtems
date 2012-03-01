##
##  $Id$
##


rtems_tests_PROGRAMS = hwpq
hwpq_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h \
									../../support/src/hwpqlib.c

#dist_rtems_tests_DATA = hwpq.scn
#dist_rtems_tests_DATA += hwpq.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(hwpq_OBJECTS) $(hwpq_LDADD)
LINK_LIBS = $(hwpq_LDLIBS)

hwpq$(EXEEXT): $(hwpq_OBJECTS) $(hwpq_DEPENDENCIES)
	@rm -f hwpq$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
