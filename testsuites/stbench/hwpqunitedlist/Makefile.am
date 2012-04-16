##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = hwpqunitedlist
hwpqunitedlist_SOURCES = ../shared/init.c ../shared/stbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									stbench.c system.h

#dist_rtems_tests_DATA = hwpqunitedlist.scn
#dist_rtems_tests_DATA += hwpqunitedlist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

hwpqunitedlist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(hwpqunitedlist_OBJECTS) $(hwpqunitedlist_LDADD)
LINK_LIBS = $(hwpqunitedlist_LDLIBS)

hwpqunitedlist$(EXEEXT): $(hwpqunitedlist_OBJECTS) $(hwpqunitedlist_DEPENDENCIES)
	@rm -f hwpqunitedlist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
