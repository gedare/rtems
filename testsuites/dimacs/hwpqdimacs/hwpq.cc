
#include <stdlib.h>
#include <stdio.h>                // has stderr, etc.
#include <string.h>               // has memset
#include "sp.h"
#include "assert.h"
#include "hwpq.h"

#include <libcpu/spillpq.h>     /* bad */
#include <libcpu/unitedlistpq.h> /* bad */
#include <libcpu/splitheappq.h> /* bad */
#include <libcpu/hwpqlib.h>

#define USE_LIBPQ


//------------------------------------------------------------
// HWPQ::Init()
// HWPQ::HWPQ()
// HWPQ::~HWPQ()
//------------------------------------------------------------

void HWPQ::Init()
{
  reInit();
}

//reInit is same as Init but does not clear statistics
void HWPQ::reInit()
{
//  assert (!PopMin());
}

HWPQ::HWPQ(int size)
{
  // FIXME: queue number, ops
#if defined(USE_LIBPQ)
  hwpqlib_initialize(0, 1);
  hwpqlib_pq_initialize(0, SPILLPQ_POLICY_DEFAULT, &sparc64_splitheappq_ops, size);
#else
  sparc64_spillpq_initialize(4, SPILLPQ_POLICY_DEFAULT, &sparc64_unitedlistpq_ops, size);
#endif
  Init();
}

HWPQ::~HWPQ()
{
}

//------------------------------------------------------------
// HWPQ::Enqueue()
// HWPQ::Update()
//     This adds a node to a HWPQ or updates an existing node
//     due to change in its priority (dist to source).
//------------------------------------------------------------

Node *HWPQ::Enqueue(Node *node)
{
  node->where = IN_HEAP;
#if defined(USE_LIBPQ)
  hwpqlib_insert(0, NODE_TO_KV(node));
#else
  HWDS_ENQUEUE(4, node->dist, node);
#endif
  return node; 
}

Node *HWPQ::Update(Node *node, int dist)
{
  uint64_t kv, kv2;
  int key;
  kv = NODE_TO_KV(node);
  key = KV_TO_K(kv);
#if defined(USE_LIBPQ)
  //kv2 = hwpqlib_extract(4, kv);
  //assert(kv == kv2);
  node->dist = dist;
  hwpqlib_insert(0, NODE_TO_KV(node));
#else
  HWDS_EXTRACT(4, key, kv); // TODO: why not one op for update prio?
  if ( kv == (uint64_t)-1 ) {
    HWDS_GET_PAYLOAD(4, kv);
    //assert(NODE_TO_KV(node) == kv);
  }
  node->dist = dist;
  HWDS_ENQUEUE(4, node->dist, node);
#endif
  return node;
}

//------------------------------------------------------------
// HWPQ::PopMin()
//   Get the highest priority (smallest dist) node in the open set.
//------------------------------------------------------------

Node *HWPQ::PopMin()
{
  uint64_t kv;
  Node *node;
  int key;
#if defined(USE_LIBPQ)
  do {
    kv = hwpqlib_pop( 0, 0UL );
    node = (Node*)KV_TO_V(kv);
    key = (int)KV_TO_K(kv);
  } while ( kv != (uint64_t)-1 && node->dist != key );
  if ( kv == (uint64_t)-1 )
    kv = 0UL;
#else
  HWDS_FIRST( 4, kv );
  if (kv != (uint64_t)-1 ) {
    uint64_t kv2;
    key = KV_TO_K(kv);
    HWDS_EXTRACT(4, key, kv2); // TODO: why not just one op for pop?
  }
  else
    return NULL;
#endif
  return (Node*)KV_TO_V(kv); // FIXME: truncates to 32 bits..
}

//------------------------------------------------------------
// HWPQ::PrintStats()
//     Prints stats specific to hwpq implementation
//------------------------------------------------------------

void HWPQ::PrintStats(long tries)
{
#ifdef ALLSTATS
#endif
}

//------------------------------------------------------------
// HWPQ::dijkstra()
//
//------------------------------------------------------------
#ifdef SINGLE_PAIR
bool HWPQ::dijkstra(Node *source, Node *sink, SP *sp)
#else
void HWPQ::dijkstra(Node *source, SP *sp)
#endif
{
  Node *currentNode, *newNode;   // newNode is beyond our current range
  Arc *arc, *lastArc;            // last arc of the current node
#ifdef SINGLE_PAIR
  bool reached;
#endif

  reInit();                        // reset indices
  sp->curTime++;
  source->tStamp = sp->curTime;

#ifdef SINGLE_PAIR
  if (source == sink) return(true);
  if (sink == NULL)
    reached = true;
  else
    reached = false;
#endif

  Enqueue(source);

  while(currentNode = PopMin()) {
    #ifdef SINGLE_PAIR
      // check if done
      if (currentNode == sink) {
        reached = true;
        break;
      }
      // do not need to search past sink distance
      if ((sink != NULL) && (sink->tStamp == sp->curTime) &&
          (currentNode->dist >= sink->dist))
        continue;
    #endif

    //    printf(">>>scanning d %lld\n", currentNode->dist);
    sp->cScans++;
    assert(currentNode->tStamp == sp->curTime);
    assert(currentNode->where != IN_SCANNED);
    // scan node
    lastArc = (currentNode + 1)->first - 1;
    for ( arc = currentNode->first; arc <= lastArc; arc++ ) {
      newNode = arc->head;                      // where our arc ends up
      if (newNode->tStamp != sp->curTime)
        sp->initNode(newNode);
      if ( currentNode->dist + arc->len < newNode->dist )
      {
        if ( newNode->where == IN_SCANNED ) {
          printk("cScans: %lld\n", sp->cScans);
          printk("cUpdates: %lld\n", sp->cUpdates);
         
          printk("newNode: %llX\n", newNode);
          printk("\tdist: %lld\n", newNode->dist);
          printk("\tparent: %llX\n", newNode->parent);
          printk("\ttStamp: %u\n", newNode->tStamp);
          while(1);
          assert(newNode->where != IN_SCANNED);
        }

        newNode->parent = currentNode;                // update sp tree
        sp->cUpdates++;  
        if (newNode->where == IN_HEAP) {
          Update(newNode, currentNode->dist + arc->len);
        }
        else {
          newNode->dist = currentNode->dist + arc->len;
          Enqueue(newNode);
        }

        #ifdef SINGLE_PAIR
          if (newNode == sink) {
            reached = true;
            break;
          }
        #endif
      }
    }
  }

#ifdef SINGLE_PAIR
  return(reached);
#endif
}

