##
##  $Id: Makefile.am,v 1.27 2011/05/05 22:29:23 joel Exp $
##

rtems_tests_PROGRAMS = gzfile
gzfile_SOURCES = init.c system.h FileSystemImage.c

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

gzfile_LDLIBS = -lz

LINK_OBJS = $(gzfile_OBJECTS) $(gzfile_LDADD)
LINK_LIBS = $(gzfile_LDLIBS)

gzfile$(EXEEXT): $(gzfile_OBJECTS) $(gzfile_DEPENDENCIES)
	@rm -f gzfile$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
