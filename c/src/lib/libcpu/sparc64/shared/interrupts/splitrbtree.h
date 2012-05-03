
#ifndef __SPARC64_SPLIT_RBTREE_H
#define __SPARC64_SPLIT_RBTREE_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_splitrbtree_ops;

#define SPARC64_SET_SPLITRBTREE_OPERATIONS(id) \
  (spillpq_ops[id] = &sparc64_splitrbtree_ops)


#ifdef __cplusplus
}
#endif

#endif
