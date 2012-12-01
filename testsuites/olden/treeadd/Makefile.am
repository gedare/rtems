##
##
##

rtems_tests_PROGRAMS = treeadd
treeadd_SOURCES = src/args.c \
									src/node.c \
									src/par-alloc.c \
									src/ssplain.c \
									src/ssplain.h \
									src/tree.h

treeadd_SOURCES += src/init.c ../shared/system.h

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

include $(top_srcdir)/shared/common.am

treeadd_LDLIBS = -lm
treeadd_LDADD = $(treeadd_LDLIBS)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include -I../lib

LINK_OBJS = $(treeadd_OBJECTS) $(treeadd_LDADD)
LINK_LIBS = $(treeadd_LDLIBS)

treeadd$(EXEEXT): $(treeadd_OBJECTS) $(treeadd_DEPENDENCIES)
	@rm -f treeadd$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
