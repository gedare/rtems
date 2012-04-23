
#ifndef __SPARC64_UNITED_LIST_ST_H
#define __SPARC64_UNITED_LIST_ST_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_unitedlistst_ops;

#define SPARC64_SET_UNITEDLISTST_OPERATIONS(id) \
  (spillpq_ops[id] = &sparc64_unitedlistst_ops)

#ifdef __cplusplus
}
#endif

#endif
