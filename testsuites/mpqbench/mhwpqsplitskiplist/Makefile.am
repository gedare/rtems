##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqsplitskiplist
mhwpqsplitskiplist_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h ../shared/hwpqbench.c

#dist_rtems_tests_DATA = mhwpqsplitskiplist.scn
#dist_rtems_tests_DATA += mhwpqsplitskiplist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqsplitskiplist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqsplitskiplist_OBJECTS) $(mhwpqsplitskiplist_LDADD)
LINK_LIBS = $(mhwpqsplitskiplist_LDLIBS)

mhwpqsplitskiplist$(EXEEXT): $(mhwpqsplitskiplist_OBJECTS) $(mhwpqsplitskiplist_DEPENDENCIES)
	@rm -f mhwpqsplitskiplist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
