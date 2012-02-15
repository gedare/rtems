##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = miheap
miheap_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									system.h pqbench.c \
									miheap.c miheap.h

#dist_rtems_tests_DATA = miheap.scn
#dist_rtems_tests_DATA += miheap.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

miheap_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(miheap_OBJECTS) $(miheap_LDADD)
LINK_LIBS = $(miheap_LDLIBS)

miheap$(EXEEXT): $(miheap_OBJECTS) $(miheap_DEPENDENCIES)
	@rm -f miheap$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
