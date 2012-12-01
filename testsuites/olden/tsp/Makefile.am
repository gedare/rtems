##
##
##

bin_PROGRAMS = tsp
tsp_SOURCES = src/args.c \
							src/build.c \
							src/main.c \
							src/ssplain.c \
							src/ssplain.h \
							src/tsp.c \
							src/tsp.h

tsp_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

tsp_LDLIBS = -lm
tsp_LDADD = $(tsp_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(tsp_OBJECTS) $(tsp_LDADD)
LINK_LIBS = $(tsp_LDLIBS)

tsp$(EXEEXT): $(tsp_OBJECTS) $(tsp_DEPENDENCIES)
	@rm -f tsp$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
