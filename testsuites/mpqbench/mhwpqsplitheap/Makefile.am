##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mhwpqsplitheap
mhwpqsplitheap_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h ../shared/hwpqbench.c

#dist_rtems_tests_DATA = mhwpqsplitheap.scn
#dist_rtems_tests_DATA += mhwpqsplitheap.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mhwpqsplitheap_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mhwpqsplitheap_OBJECTS) $(mhwpqsplitheap_LDADD)
LINK_LIBS = $(mhwpqsplitheap_LDLIBS)

mhwpqsplitheap$(EXEEXT): $(mhwpqsplitheap_OBJECTS) $(mhwpqsplitheap_DEPENDENCIES)
	@rm -f mhwpqsplitheap$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
