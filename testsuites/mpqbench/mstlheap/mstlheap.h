/*
 * STL Vector-based Heap implementation for priority queue
 *
 */

#ifndef __MSTLHEAP_H_
#define __MSTLHEAP_H_

#include "rtems/rtems/types.h"
#include "../shared/pqbench.h"

#include <algorithm>
#include <vector>

#define PQ_NODE_TO_KV(n) ((((long)n->key) << 32UL) | (long)n->val)

class pq_node_t {
  public:
    pq_node_t(int key, int val) {
      this->key = key;
      this->val = val;
    }
    
    pq_node_t(long kv) {
      this->key = kv_key(kv);
      this->val = kv_value(kv);
    }

    // should be private, but makes comparisons and merging kv harder
    int key;
    int val;
};

class pq_t {
  public:
    pq_t() { ; }

    ~pq_t() { ; }

    long first();
    long pop();

    void insert(long kv);
    long search( int key );
    long extract( int key );

    class pq_node_min_compare {
      public:
        bool operator() ( const pq_node_t *a, const pq_node_t *b ) const {
          return a->key > b->key;
        }
    };

  private:
    std::vector<pq_node_t *> m_pq;
};

#endif
