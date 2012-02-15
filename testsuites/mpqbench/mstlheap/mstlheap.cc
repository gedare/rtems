
#include "mstlheap.h"

pq_t* the_heap;

uint64_t pq_t::first() {
  pq_node_t *n = m_pq.front();
  return PQ_NODE_TO_KV(n);
}

uint64_t pq_t::pop() {
  uint64_t kv = first();
  pop_heap( m_pq.begin(), m_pq.end(), pq_node_min_compare() );
  m_pq.pop_back(); // pop_heap moved the front to the back
  return kv;
}

void pq_t::insert(uint64_t kv) {
  pq_node_t *n = new pq_node_t(kv);
  m_pq.push_back(n);
  push_heap( m_pq.begin(), m_pq.end(), pq_node_min_compare() );
}
