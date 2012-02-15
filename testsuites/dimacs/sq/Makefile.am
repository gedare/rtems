##
##  $Id: Makefile.am,v 1.27 2011/05/05 22:29:23 joel Exp $
##

rtems_tests_PROGRAMS = sq
sq_SOURCES = init.c system.h
sq_SOURCES += main.cc sp.cc smartq.cc parser_gr.cc timer.cc
sq_SOURCES += sp.h nodearc.h smartq.h stack.h values.h
sq_SOURCES += parser_ss.cc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

sq_LDLIBS = -lm -lz

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(sq_OBJECTS) $(sq_LDADD)
LINK_LIBS = $(sq_LDLIBS)

sq$(EXEEXT): $(sq_OBJECTS) $(sq_DEPENDENCIES)
	@rm -f sq$(EXEEXT)
	$(make-cxx-exe)

include $(top_srcdir)/../automake/local.am
