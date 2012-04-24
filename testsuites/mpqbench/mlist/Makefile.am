##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mlist
mlist_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									system.h pqbench.c \
									mlist.c mlist.h

#dist_rtems_tests_DATA = mlist.scn
#dist_rtems_tests_DATA += mlist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mlist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mlist_OBJECTS) $(mlist_LDADD)
LINK_LIBS = $(mlist_LDLIBS)

mlist$(EXEEXT): $(mlist_OBJECTS) $(mlist_DEPENDENCIES)
	@rm -f mlist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
