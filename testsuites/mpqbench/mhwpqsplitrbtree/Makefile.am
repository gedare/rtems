##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqsplitrbtree
mhwpqsplitrbtree_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h ../shared/hwpqbench.c

#dist_rtems_tests_DATA = mhwpqsplitrbtree.scn
#dist_rtems_tests_DATA += mhwpqsplitrbtree.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqsplitrbtree_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqsplitrbtree_OBJECTS) $(mhwpqsplitrbtree_LDADD)
LINK_LIBS = $(mhwpqsplitrbtree_LDLIBS)

mhwpqsplitrbtree$(EXEEXT): $(mhwpqsplitrbtree_OBJECTS) $(mhwpqsplitrbtree_DEPENDENCIES)
	@rm -f mhwpqsplitrbtree$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
