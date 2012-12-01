##
##
##

rtems_tests_PROGRAMS = perimeter
perimeter_SOURCES =src/args.c \
									 src/main.c \
									 src/maketree.c \
									 src/perimeter.h \
									 src/ssplain.c \
									 src/ssplain.h

perimeter_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

perimeter_LDLIBS = -lm
perimeter_LDADD = $(perimeter_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(perimeter_OBJECTS) $(perimeter_LDADD)
LINK_LIBS = $(perimeter_LDLIBS)

perimeter$(EXEEXT): $(perimeter_OBJECTS) $(perimeter_DEPENDENCIES)
	@rm -f perimeter$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
