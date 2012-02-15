
#ifndef HWPQ_H
#define HWPQ_H

#include "nodearc.h"         // dfn of node and arc

class SP;                // defined in sp.h

#include "rtems/rtems/types.h"
#define KV_TO_V(kv) ((uint32_t)kv)
#define KV_TO_K(kv) (kv>>32)
#define NODE_TO_KV(node) (uint64_t)((((uint64_t)node->dist)<<32UL)|(uint64_t)node)
extern "C" {
  extern void sparc64_hwpq_drain_queue( int qid ); /* bad */
}

class HWPQ {
 private:

 public:
   HWPQ(int size);
   ~HWPQ();
   void Init();
   void reInit();
#ifdef SINGLE_PAIR
   bool dijkstra(Node *source, Node *sink, SP *sp); // run dijkstra's algorithm
#else
   void dijkstra(Node *source, SP *sp);            // run dijkstra's algorithm
#endif
   void PrintStats(long tries);

   Node *Enqueue(Node *node);
   Node *Update(Node *node, int dist);
   Node *PopMin();
};

#endif
