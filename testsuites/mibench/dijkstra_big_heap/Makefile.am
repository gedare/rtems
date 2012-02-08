##
##  $Id$
##



rtems_tests_PROGRAMS = dijkstra_big_heap
dijkstra_big_heap_SOURCES = init.c FileSystemImage.c

#dist_rtems_tests_DATA = dijkstra_big_heap.scn
#dist_rtems_tests_DATA += dijkstra_big_heap.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am



LINK_OBJS = $(dijkstra_big_heap_OBJECTS) $(dijkstra_big_heap_LDADD)
LINK_LIBS = $(dijkstra_big_heap_LDLIBS)

dijkstra_big_heap$(EXEEXT): $(dijkstra_big_heap_OBJECTS) $(dijkstra_big_heap_DEPENDENCIES)
	@rm -f dijkstra_big_heap$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
