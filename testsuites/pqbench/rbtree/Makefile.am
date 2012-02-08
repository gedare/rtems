##
##  $Id$
##



rtems_tests_PROGRAMS = rbtree
rbtree_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h \
									rbtree.c rbtree.h

#dist_rtems_tests_DATA = rbtree.scn
#dist_rtems_tests_DATA += rbtree.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am



LINK_OBJS = $(rbtree_OBJECTS) $(rbtree_LDADD)
LINK_LIBS = $(rbtree_LDLIBS)

rbtree$(EXEEXT): $(rbtree_OBJECTS) $(rbtree_DEPENDENCIES)
	@rm -f rbtree$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
