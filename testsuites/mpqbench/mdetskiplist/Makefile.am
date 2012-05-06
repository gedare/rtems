##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mdetskiplist
mdetskiplist_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									system.h pqbench.c \
									mdetskiplist.c mdetskiplist.h

#dist_rtems_tests_DATA = mdetskiplist.scn
#dist_rtems_tests_DATA += mdetskiplist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mdetskiplist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mdetskiplist_OBJECTS) $(mdetskiplist_LDADD)
LINK_LIBS = $(mdetskiplist_LDLIBS)

mdetskiplist$(EXEEXT): $(mdetskiplist_OBJECTS) $(mdetskiplist_DEPENDENCIES)
	@rm -f mdetskiplist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
