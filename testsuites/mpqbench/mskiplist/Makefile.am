##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mskiplist
mskiplist_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									system.h pqbench.c \
									mskiplist.c mskiplist.h

#dist_rtems_tests_DATA = mskiplist.scn
#dist_rtems_tests_DATA += mskiplist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mskiplist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mskiplist_OBJECTS) $(mskiplist_LDADD)
LINK_LIBS = $(mskiplist_LDLIBS)

mskiplist$(EXEEXT): $(mskiplist_OBJECTS) $(mskiplist_DEPENDENCIES)
	@rm -f mskiplist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
