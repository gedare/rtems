##
##  $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = mnewsplay
mnewsplay_SOURCES = ../shared/init.c ../shared/pqbench.h \
 									../shared/workload.c ../shared/workload.h \
									../shared/params.h ../shared/params.i ../shared/tasks.i \
									pqbench.c system.h \
									mnewsplay.c mnewsplay.h \
									rtemssapisplay.h rtemsscoresplay.h rtemsscoresplay.c

#dist_rtems_tests_DATA = mnewsplay.scn
#dist_rtems_tests_DATA += mnewsplay.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

mnewsplay_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(mnewsplay_OBJECTS) $(mnewsplay_LDADD)
LINK_LIBS = $(mnewsplay_LDLIBS)

mnewsplay$(EXEEXT): $(mnewsplay_OBJECTS) $(mnewsplay_DEPENDENCIES)
	@rm -f mnewsplay$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
