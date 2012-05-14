
#ifndef __SPARC64_SPLIT_SPLAY_H
#define __SPARC64_SPLIT_SPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_splitsplay_ops;

#define SPARC64_SET_SPLITSPLAY_OPERATIONS(id) \
  (spillpq_ops[id] = &sparc64_splitsplay_ops)

#ifdef __cplusplus
}
#endif

#endif
