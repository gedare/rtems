##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = splay
splay_SOURCES = ../shared/init.c ../shared/stbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									stbench.c system.h \
									splay.c splay.h

#dist_rtems_tests_DATA = splay.scn
#dist_rtems_tests_DATA += splay.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

splay_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(splay_OBJECTS) $(splay_LDADD)
LINK_LIBS = $(splay_LDLIBS)

splay$(EXEEXT): $(splay_OBJECTS) $(splay_DEPENDENCIES)
	@rm -f splay$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
