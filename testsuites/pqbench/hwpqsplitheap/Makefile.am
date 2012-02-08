##
##  $Id$
##



rtems_tests_PROGRAMS = hwpqsplitheap
hwpqsplitheap_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i \
									pqbench.c system.h

#dist_rtems_tests_DATA = hwpqsplitheap.scn
#dist_rtems_tests_DATA += hwpqsplitheap.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am



LINK_OBJS = $(hwpqsplitheap_OBJECTS) $(hwpqsplitheap_LDADD)
LINK_LIBS = $(hwpqsplitheap_LDLIBS)

hwpqsplitheap$(EXEEXT): $(hwpqsplitheap_OBJECTS) $(hwpqsplitheap_DEPENDENCIES)
	@rm -f hwpqsplitheap$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
