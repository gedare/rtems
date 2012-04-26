##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mbptree
mbptree_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									system.h pqbench.c \
									mbptree.c mbptree.h

#dist_rtems_tests_DATA = mbptree.scn
#dist_rtems_tests_DATA += mbptree.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mbptree_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mbptree_OBJECTS) $(mbptree_LDADD)
LINK_LIBS = $(mbptree_LDLIBS)

mbptree$(EXEEXT): $(mbptree_OBJECTS) $(mbptree_DEPENDENCIES)
	@rm -f mbptree$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
