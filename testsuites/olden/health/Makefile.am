##
##
##

rtems_tests_PROGRAMS = health
health_SOURCES =src/args.c \
								src/health.c \
								src/health.h \
								src/list.c \
								src/poisson.c \
								src/ssplain.c \
								src/ssplain.h

health_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

health_LDLIBS = -lm
health_LDADD = $(health_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(health_OBJECTS) $(health_LDADD)
LINK_LIBS = $(health_LDLIBS)

health$(EXEEXT): $(health_OBJECTS) $(health_DEPENDENCIES)
	@rm -f health$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
