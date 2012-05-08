##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqunitedskiplist
mhwpqunitedskiplist_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h ../shared/hwpqbench.c

#dist_rtems_tests_DATA = mhwpqunitedskiplist.scn
#dist_rtems_tests_DATA += mhwpqunitedskiplist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqunitedskiplist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqunitedskiplist_OBJECTS) $(mhwpqunitedskiplist_LDADD)
LINK_LIBS = $(mhwpqunitedskiplist_LDLIBS)

mhwpqunitedskiplist$(EXEEXT): $(mhwpqunitedskiplist_OBJECTS) $(mhwpqunitedskiplist_DEPENDENCIES)
	@rm -f mhwpqunitedskiplist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
