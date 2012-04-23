
#ifndef __SPARC64_UNITED_LIST_BTREE_H
#define __SPARC64_UNITED_LIST_BTREE_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_unitedlistbtree_ops;

#define SPARC64_SET_UNITEDLISTPQ_OPERATIONS(id) \
  (spillpq_ops[id] = &sparc64_unitedlistbtree_ops)

#ifdef __cplusplus
}
#endif

#endif
