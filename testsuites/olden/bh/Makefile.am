##
##
##

rtems_tests_PROGRAMS = bh
#rtems_tests_PROGRAMS += modbh
bh_SOURCES =src/args.c \
            src/code.h \
            src/defs.h \
            src/newbh.c \
            src/ssplain.c \
            src/ssplain.h \
            src/stdinc.h \
            src/util.c \
            src/vectmath.h

bh_SOURCES += src/init.c ../shared/system.h
#modbh_SOURCES =src/args.c \
#            src/code.h \
#            src/defs.h \
#            src/modbh.c \
#            src/ssplain.c \
#            src/ssplain.h \
#            src/stdinc.h \
#            src/util.c \
#            src/vectmath.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

bh_LDLIBS = -lm
bh_LDADD = $(bh_LDLIBS)

#modbh_LDLIBS = -lm
#modbh_LDADD = $(modbh_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(bh_OBJECTS) $(bh_LDADD)
LINK_LIBS = $(bh_LDLIBS)

bh$(EXEEXT): $(bh_OBJECTS) $(bh_DEPENDENCIES)
	@rm -f bh$(EXEEXT)
	$(make-exe)

#LINK_OBJS = $(modbh_OBJECTS) $(modbh_LDADD)
#LINK_LIBS = $(modbh_LDLIBS)
#
#modbh$(EXEEXT): $(modbh_OBJECTS) $(modbh_DEPENDENCIES)
#	@rm -f modbh$(EXEEXT)
#	$(make-exe)

include $(top_srcdir)/../automake/local.am
