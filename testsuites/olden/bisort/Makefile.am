##
##
##

rtems_tests_PROGRAMS = bisort
bisort_SOURCES =src/args.c \
                src/bitonic.c \
                src/node.h \
                src/proc.h \
                src/ssplain.c \
                src/ssplain.h

bisort_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

bisort_LDLIBS = -lm
bisort_LDADD = $(bisort_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(bisort_OBJECTS) $(bisort_LDADD)
LINK_LIBS = $(bisort_LDLIBS)

bisort$(EXEEXT): $(bisort_OBJECTS) $(bisort_DEPENDENCIES)
	@rm -f bisort$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
