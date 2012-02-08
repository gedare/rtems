##
##  $Id$
##



rtems_tests_PROGRAMS = stlheap
stlheap_SOURCES = ../shared/init.c ../shared/pqbench.h \
									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.cc system.h \
									stlheap.cc stlheap.h

#dist_rtems_tests_DATA = stlheap.scn
#dist_rtems_tests_DATA += stlheap.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am



AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(stlheap_OBJECTS) $(stlheap_LDADD)
LINK_LIBS = $(stlheap_LDLIBS)

stlheap$(EXEEXT): $(stlheap_OBJECTS) $(stlheap_DEPENDENCIES)
	@rm -f stlheap$(EXEEXT)
	$(make-cxx-exe)

include $(top_srcdir)/../automake/local.am
