##
##
##

rtems_tests_PROGRAMS = mst
mst_SOURCES =src/args.c \
						src/hash.c \
						src/hash.h \
						src/main.c \
						src/makegraph.c \
						src/mst.h \
						src/ssplain.c \
						src/ssplain.h

mst_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

mst_LDLIBS = -lm
mst_LDADD = $(mst_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(mst_OBJECTS) $(mst_LDADD)
LINK_LIBS = $(mst_LDLIBS)

mst$(EXEEXT): $(mst_OBJECTS) $(mst_DEPENDENCIES)
	@rm -f mst$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
