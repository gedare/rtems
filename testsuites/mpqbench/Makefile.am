##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../aclocal

SUBDIRS = mdetskiplist miheap msplay mlist mskiplist mbptree \
					mrbtree mnewsplay

if sun4u
SUBDIRS += mhwpqsplitheap mhwpqsplitrbtree mhwpqsplitskiplist mhwpqsplitsplay
SUBDIRS += mhwpqunitedlist mhwpq mhwpqunitedskiplist
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
