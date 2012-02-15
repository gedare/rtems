##
##  $Id: Makefile.am,v 1.27 2011/05/05 22:29:23 joel Exp $
##

rtems_tests_PROGRAMS = hwpqdimacs
hwpqdimacs_SOURCES = init.c system.h
hwpqdimacs_SOURCES += hwpq.cc hwpq.h
hwpqdimacs_SOURCES += main.cc sp.cc parser_gr.cc parser_ss.cc timer.cc
hwpqdimacs_SOURCES += sp.h nodearc.h stack.h values.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

hwpqdimacs_LDLIBS = -lm -lz

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(hwpqdimacs_OBJECTS) $(hwpqdimacs_LDADD)
LINK_LIBS = $(hwpqdimacs_LDLIBS)

hwpqdimacs$(EXEEXT): $(hwpqdimacs_OBJECTS) $(hwpqdimacs_DEPENDENCIES)
	@rm -f hwpqdimacs$(EXEEXT)
	$(make-cxx-exe)

include $(top_srcdir)/../automake/local.am
