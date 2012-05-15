
#ifndef __SPARC64_SPLIT_SKIPLIST_H
#define __SPARC64_SPLIT_SKIPLIST_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_splitskiplist_ops;

#define SPARC64_SET_SPLITSKIPLIST_OPERATIONS(id) \
  (spillpq[id].ops = &sparc64_splitskiplist_ops)

#ifdef __cplusplus
}
#endif

#endif
