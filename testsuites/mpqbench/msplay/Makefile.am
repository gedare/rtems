##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = msplay
msplay_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									pqbench.c system.h \
									msplay.c msplay.h

#dist_rtems_tests_DATA = msplay.scn
#dist_rtems_tests_DATA += msplay.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

msplay_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(msplay_OBJECTS) $(msplay_LDADD)
LINK_LIBS = $(msplay_LDLIBS)

msplay$(EXEEXT): $(msplay_OBJECTS) $(msplay_DEPENDENCIES)
	@rm -f msplay$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
