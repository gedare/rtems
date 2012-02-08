##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = dijkstra \
					dijkstra_big_HWPQ \
					dijkstra_big_heap
#					dijkstra_small_sort_HWPQ dijkstra_small_sort_HWPQ_spill \
#					dijkstra_big_sort_HWPQ_spill \
#					dijkstra_small_heap \
#					dijkstra_large_sort_HWPQ dijkstra_large_sort dijkstra_large_heap

#SUBDIRS = sha_small sha_large crc dijkstra susan_smoothing \
#					 dijkstra_large_sort_OSPQ \
#					dijkstra_small_sort_HWPQ dijkstra_small_sort_OSPQ \
#					dijkstra_small_sort_HWPQ_spill \
#					dijkstra_big_sort_HWPQ_spill \
#					dijkstra_small_sort dijkstra_large_sort \
#					dijkstra_small_heap dijkstra_large_heap \
#					dijkstra_small_3_heap

if MPTESTS
endif

if CXXTESTS
endif

if NETTESTS
endif

include $(top_srcdir)/../automake/subdirs.am
include $(top_srcdir)/../automake/local.am
