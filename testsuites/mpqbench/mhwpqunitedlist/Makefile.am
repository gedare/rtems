##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqunitedlist
mhwpqunitedlist_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h

#dist_rtems_tests_DATA = mhwpqunitedlist.scn
#dist_rtems_tests_DATA += mhwpqunitedlist.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqunitedlist_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqunitedlist_OBJECTS) $(mhwpqunitedlist_LDADD)
LINK_LIBS = $(mhwpqunitedlist_LDLIBS)

mhwpqunitedlist$(EXEEXT): $(mhwpqunitedlist_OBJECTS) $(mhwpqunitedlist_DEPENDENCIES)
	@rm -f mhwpqunitedlist$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
