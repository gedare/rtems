##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mstlheap
mstlheap_SOURCES = ../shared/init.c ../shared/pqbench.h \
									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.cc system.h \
									mstlheap.cc mstlheap.h

#dist_rtems_tests_DATA = mstlheap.scn
#dist_rtems_tests_DATA += mstlheap.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mstlheap_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mstlheap_OBJECTS) $(mstlheap_LDADD)
LINK_LIBS = $(mstlheap_LDLIBS)

mstlheap$(EXEEXT): $(mstlheap_OBJECTS) $(mstlheap_DEPENDENCIES)
	@rm -f mstlheap$(EXEEXT)
	$(make-cxx-exe)

include $(top_srcdir)/../automake/local.am
