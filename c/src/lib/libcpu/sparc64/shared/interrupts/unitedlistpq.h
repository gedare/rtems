
#ifndef __SPARC64_UNITED_LIST_PQ_H
#define __SPARC64_UNITED_LIST_PQ_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_unitedlistpq_ops;

#define SPARC64_SET_UNITEDLISTPQ_OPERATIONS(id) \
  (spillpq[id].ops = &sparc64_unitedlistpq_ops)

#ifdef __cplusplus
}
#endif

#endif
