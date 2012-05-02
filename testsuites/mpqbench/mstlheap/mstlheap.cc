
#include "mstlheap.h"

long pq_t::first() {
  pq_node_t *n = m_pq.front();
  return PQ_NODE_TO_KV(n);
}

long pq_t::pop() {
  long kv = first();
  pop_heap( m_pq.begin(), m_pq.end(), pq_node_min_compare() );
  m_pq.pop_back(); // pop_heap moved the front to the back
  return kv;
}

void pq_t::insert(long kv) {
  pq_node_t *n = new pq_node_t(kv);
  m_pq.push_back(n);
  push_heap( m_pq.begin(), m_pq.end(), pq_node_min_compare() );
}

long pq_t::search(int k) {
  return (long)-1;

}

long pq_t::extract(int k) {
  return (long)-1;

}
