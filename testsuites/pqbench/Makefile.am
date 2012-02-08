##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = hwpq hwpqsplitheap hwpqunitedlist \
					iheap rbtree splay

if MPTESTS
endif

if CXXTESTS
SUBDIRS += stlheap
endif

if NETTESTS
endif

include $(top_srcdir)/../automake/subdirs.am
include $(top_srcdir)/../automake/local.am
