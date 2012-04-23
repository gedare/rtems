##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = miheap msplay \
					mhwpqsplitheap mhwpqunitedlist mhwpq \
					mrbtree \
					mhwpqunitedsearch

if MPTESTS
endif

if CXXTESTS
#SUBDIRS += mstlheap
endif

if NETTESTS
endif

include $(top_srcdir)/../automake/subdirs.am
include $(top_srcdir)/../automake/local.am
