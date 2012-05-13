##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqsplitsplay
mhwpqsplitsplay_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h ../shared/hwpqbench.c

#dist_rtems_tests_DATA = mhwpqsplitsplay.scn
#dist_rtems_tests_DATA += mhwpqsplitsplay.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqsplitsplay_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqsplitsplay_OBJECTS) $(mhwpqsplitsplay_LDADD)
LINK_LIBS = $(mhwpqsplitsplay_LDLIBS)

mhwpqsplitsplay$(EXEEXT): $(mhwpqsplitsplay_OBJECTS) $(mhwpqsplitsplay_DEPENDENCIES)
	@rm -f mhwpqsplitsplay$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
