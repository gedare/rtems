##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = miheap msplay mlist mskiplist mbptree \
					mrbtree

if sun4u
SUBDIRS += mhwpqsplitheap mhwpqsplitrbtree 
SUBDIRS += mhwpqunitedlist mhwpq mhwpqunitedsearch
endif
if MPTESTS
endif

if CXXTESTS
SUBDIRS += mstlheap
endif

if NETTESTS
endif

include $(top_srcdir)/../automake/subdirs.am
include $(top_srcdir)/../automake/local.am
