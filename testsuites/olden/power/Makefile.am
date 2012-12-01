##
##
##

rtems_tests_PROGRAMS = power
power_SOURCES =src/args.c \
							 src/build.c \
							 src/compute.c \
							 src/main.c \
							 src/power.h \
							 src/ssplain.c \
							 src/ssplain.h

power_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

power_LDLIBS = -lm
power_LDADD = $(power_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(power_OBJECTS) $(power_LDADD)
LINK_LIBS = $(power_LDLIBS)

power$(EXEEXT): $(power_OBJECTS) $(power_DEPENDENCIES)
	@rm -f power$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
