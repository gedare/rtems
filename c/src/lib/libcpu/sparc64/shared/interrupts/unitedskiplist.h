
#ifndef __SPARC64_UNITED_SKIPLIST_H
#define __SPARC64_UNITED_SKIPLIST_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_unitedskiplist_ops;

#define SPARC64_SET_UNITEDSKIPLIST_OPERATIONS(id) \
  (spillpq[id].ops = &sparc64_unitedskiplist_ops)

#ifdef __cplusplus
}
#endif

#endif
