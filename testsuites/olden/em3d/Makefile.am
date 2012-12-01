##
##
##

rtems_tests_PROGRAMS = em3d
em3d_SOURCES =src/args.c \
              src/em3d.c \
              src/em3d.h \
              src/main.c \
              src/make_graph.c \
              src/make_graph.h \
              src/ssplain.c \
              src/ssplain.h \
              src/util.c \
              src/util.h

em3d_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

em3d_LDLIBS = -lm
em3d_LDADD = $(em3d_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(em3d_OBJECTS) $(em3d_LDADD)
LINK_LIBS = $(em3d_LDLIBS)

em3d$(EXEEXT): $(em3d_OBJECTS) $(em3d_DEPENDENCIES)
	@rm -f em3d$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
