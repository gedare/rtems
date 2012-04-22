/*
 * STL Vector-based Heap implementation for priority queue
 *
 */

#ifndef __MSTLHEAP_H_
#define __MSTLHEAP_H_

#include "rtems/rtems/types.h"

#include <algorithm>
#include <vector>

#define PQ_NODE_TO_KV(n) ((((uint64_t)n->key) << 32UL) | (uint64_t)n->val)

class pq_node_t {
  public:
    pq_node_t(int key, int val) {
      this->key = key;
      this->val = val;
    }
    
    pq_node_t(uint64_t kv) {
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

    uint64_t first();
    uint64_t pop();

    void insert(uint64_t kv);
    uint64_t search( int key );
    uint64_t extract( int key );

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
