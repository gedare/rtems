##
##  $Id$
##



rtems_tests_PROGRAMS = dijkstra_big_HWPQ
dijkstra_big_HWPQ_SOURCES = init.c FileSystemImage.c

#dist_rtems_tests_DATA = dijkstra_big_HWPQ.scn
#dist_rtems_tests_DATA += dijkstra_big_HWPQ.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am



LINK_OBJS = $(dijkstra_big_HWPQ_OBJECTS) $(dijkstra_big_HWPQ_LDADD)
LINK_LIBS = $(dijkstra_big_HWPQ_LDLIBS)

dijkstra_big_HWPQ$(EXEEXT): $(dijkstra_big_HWPQ_OBJECTS) $(dijkstra_big_HWPQ_DEPENDENCIES)
	@rm -f dijkstra_big_HWPQ$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
