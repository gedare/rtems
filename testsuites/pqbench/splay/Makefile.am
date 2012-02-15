##
##  $Id$
##


rtems_tests_PROGRAMS = splay
splay_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h \
									splay.c splay.h

#dist_rtems_tests_DATA = splay.scn
#dist_rtems_tests_DATA += splay.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am


AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(splay_OBJECTS) $(splay_LDADD)
LINK_LIBS = $(splay_LDLIBS)

splay$(EXEEXT): $(splay_OBJECTS) $(splay_DEPENDENCIES)
	@rm -f splay$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
