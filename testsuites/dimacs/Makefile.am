##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = 

if MPTESTS
SUBDIRS += 
endif

if CXXTESTS
SUBDIRS += sq hwpqdimacs
endif

if NETTESTS
## loopback tests a network loopback interface
SUBDIRS += 
endif

include $(top_srcdir)/../automake/subdirs.am
include $(top_srcdir)/../automake/local.am
