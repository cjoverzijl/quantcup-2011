#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "liststructure.h"

node * find_by_id(t_orderid id, list * ll) {
  return ll->idtable[id];
}

node * quickfind1(t_price limit, node * addresses[] ) {
  //prototype for finding nearest HIGHER node --- NOT speed-optimal, but should work
  //takes the limit as starting pivot since we use only the non-bunched array, and returns the index of the next better node

  int pivot; // = find_earlier(limit, addresses);
  while ( pivot && !addresses[pivot] ) {
    pivot--;
  }
  if (pivot) return addresses[pivot];
  if (addresses[0]) return addresses[0];
  return NULL;
}

node * quickfind2(t_price limit, list * ll) {
  // linear linked-list walk

  node * current = ll->head;
  while (current && current->next && current->next->order.price >= limit ) {
    current = current->next;
  }
  if (!current) return NULL;
  return current;
}

node * quickfind3(t_price limit, list * ll ) {
  //find something recent and walk from there

  node * current = better_recent_price(limit, ll );
  if(!current) current = ll->head;

  while (current && current->next && current->next->order.price >= limit ) {
    current = current->next;
  }
  if (!current) return NULL;
  return current;
}

node * better_recent_price(t_price limit, list * ll){ // pull a close recent price, but only return if node actually exists

  int i;
  for (i=0;i<MAX_RECENTS;i++){
    if (ll->side) { //asks
      if ( ll->recents[i] < limit && ll->limits[ll->recents[i]] ) return ll->limits[ll->recents[i]];
    } else { //bids
      if ( ll->recents[i] > limit && ll->limits[ll->recents[i]] ) return ll->limits[ll->recents[i]];
    }
  }
  return NULL;
}

void add_recent_price(t_price limit, list * ll) {
  // MAX_RECENTS recent prices; don't even have to be uncanceled bc we'll lookup in ll->limits[]
  int i;
  for (i=MAX_RECENTS-1;i>0;i--) {
    ll->recents[i] = ll->recents[i-1];
  }
  ll->recents[0] = limit;
}



void add_node(t_orderid id, t_order * order, list * ll, pool_t * nodespool ) {
  /*  add a new node, hopefully more memory optimal than reference implementation
   has access to own structure, rather than pointers to node */

  node * rv = pool_alloc(nodespool); //malloc(sizeof(node));
  rv->id = id;
  rv->order = *order;
  int limit = order->price;

  // Positioning of the node in the list

  if (ll->limits[limit]) { // easy case: not first node, and not first node at this limit

    rv->next = ll->limits[limit]->next;   // my next is your next, even if NULL
    rv->prev = ll->limits[limit];         // my prev is you
    ll->limits[limit]->next = rv;         // your next is me, but your prev is untouched
    if (rv->next) rv->next->prev = rv;    // my next's prev is me

  } else { // hard case: first node at this limit AND maybe first node of all

//    node * nextbetter = quickfind1(limit, ll->limits);
//    node * nextbetter = quickfind2(limit, ll);
    node * nextbetter = quickfind3(limit, ll);

    if (nextbetter) { // first limit at this price, but not first of all

      rv->next = nextbetter->next;
      rv->prev = nextbetter;
      nextbetter->next = rv; // nextbetter->prev untouched

    } else { // I am the FIRST node

      rv->next = NULL;
      rv->prev = NULL;
      ll->head = rv;

    }
  }

  ll->idtable[id] = rv;
  ll->limits[limit] = rv;
  add_recent_price(limit, ll);

}

void remove_node(node * rv, list * ll, pool_t * nodespool ) {

  if( rv->next && rv->prev ) { //generic

    rv->prev->next = rv->next;
    rv->next->prev = rv->prev;
    if (rv == ll->limits[rv->order.price]) {
      if (rv->order.price == rv->prev->order.price) { //latest of set @same limit
        ll->limits[rv->order.price] = rv->prev;
      } else { //latest and only
        ll->limits[rv->order.price] = NULL;
      }
    }
    
  } else if (!rv->prev && rv->next ) { //head

    rv->next->prev = NULL;
    ll->head = rv->next;
    if ( rv == ll->limits[rv->order.price] ) { // if head is latest at the price, set entry null
      ll->limits[rv->order.price] = NULL;
    }
    
  } else if ( rv->prev && !rv->next) { //tail

    rv->prev->next = NULL;
    if ( (rv == ll->limits[rv->order.price]) && (rv->order.price == rv->prev->order.price) ) { // if tail, point array at last
      ll->limits[rv->order.price] = rv->prev;
    } else { // but set null if it was the only one
      ll->limits[rv->order.price] = NULL;
    }

  } else { // head = only node

    ll->limits[rv->order.price] = NULL;
    ll->head = NULL;

  }

  ll->idtable[rv->id] = NULL;
  pool_free(nodespool,rv);

}

void delete_list(list * list, pool_t * nodespool) {
  while( list->head ) { // free from head on
    node * head = list->head;
    list->head = list->head->next;
    pool_free(nodespool,head); //free(head);
  }
}
