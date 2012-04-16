##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mrbtree
mrbtree_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h \
									mrbtree.c mrbtree.h

#dist_rtems_tests_DATA = mrbtree.scn
#dist_rtems_tests_DATA += mrbtree.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mrbtree_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mrbtree_OBJECTS) $(mrbtree_LDADD)
LINK_LIBS = $(mrbtree_LDLIBS)

mrbtree$(EXEEXT): $(mrbtree_OBJECTS) $(mrbtree_DEPENDENCIES)
	@rm -f mrbtree$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
