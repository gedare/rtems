
#ifndef __SPARC64_SPLIT_SPLAY_H
#define __SPARC64_SPLIT_SPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

extern sparc64_spillpq_operations sparc64_splitsplay_ops;

#define SPARC64_SET_SPLITSPLAY_OPERATIONS(id) \
  (spillpq_ops[id] = &sparc64_splitsplay_ops)

typedef struct rtems_splay_tree_node_s rtems_splay_tree_node;
struct rtems_splay_tree_node_s
{
    rtems_splay_tree_node  * leftlink;
    rtems_splay_tree_node  * rightlink;
    rtems_splay_tree_node  * uplink;
    int    cnt;
};

typedef struct
{
    rtems_splay_tree_node  * root;    /* root node */

    /* Statistics, not strictly necessary, but handy for tuning  */

    int    lookups;  /* number of splookup()s */
    int    lkpcmps;  /* number of lookup comparisons */
    
    int    enqs;    /* number of spenq()s */
    int    enqcmps;  /* compares in spenq */
    
    int    splays;
    int    splayloops;

} rtems_splay_tree;


#define _Container_of(_node, _container_type, _node_field_name) \
( \
  (_container_type*) \
  ( (uintptr_t)(_node) - offsetof(_container_type, _node_field_name) ) \
)

#define _Splay_Initialize_empty( _tree, _compare ) do {\
  (_tree)->lookups = 0;\
  (_tree)->lkpcmps = 0;\
  (_tree)->enqs = 0;\
  (_tree)->enqcmps = 0;\
  (_tree)->splays = 0;\
  (_tree)->splayloops = 0;\
  (_tree)->root = NULL;\
} while (0)

#define _Splay_is_empty( _tree ) ((_tree)->root == NULL)

#ifdef __cplusplus
}
#endif

#endif
