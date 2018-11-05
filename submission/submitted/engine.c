/*
 Verzijl
 Not-so-basic implementation of the matching engine:

   * doubly-linked list structure +
   * idtable array for quick finds of orders +
   * limits array for quick finds of insertion points when not unique limit

   and

   * quickfind3() lookup in sparse array using most-recent-bids list
   * node allocation from pool

 O(1) add for next bid/ask with same price, O(log N) for first-not-head limit
 O(1) cancel
 O(1) executions

 Still big pre-factors though...
 */

#include <stdio.h>
#include <stdbool.h>
#include "string.h"
#include "engine.h"
#include "pool.h"

//  -------------------------------------------------- DATA STRUCTURES

#include "liststructure.c"
#include "pool.c"

//  -------------------------------------------------- GLOBALS
// Stores all live bids and asks*/
list * bids;
list * asks;
// The next ID which will be assigned an order */
t_orderid nextid;
// Pool for nodes
pool_t * anodespool = NULL;
pool_t * bnodespool = NULL;

//  -------------------------------------------------- UTILITIES

bool hit(t_price bid, t_price ask) { 
  return bid >= ask;
}

void send_exec(t_order * o1, t_order * o2) { //printf("* send_exec\n");
  /* Call the execution report callback to notify both parties to the trade
   o1 and o2 are assumed to be the same price on opposite sides */

  t_execution exec = (t_execution) (*o1);
  exec.size = o1->size > o2->size ? o2->size : o1->size;
  execution(exec);
  // rename trader field on exec to old's name
  memcpy(exec.trader, o2->trader, STRINGLEN);
  exec.side = !exec.side;
  execution(exec);
}

void trade(t_order * order, node * offer, list * ll) {

  send_exec(order, &offer->order);

  if (order->size >= offer->order.size) { // new completely fills offer, remove offer from book

    order->size -= offer->order.size;
    offer->order.size = 0;
    remove_node(offer, ll, (ll->side)? anodespool : bnodespool);

  } else { // new partially fills old, order fulfilled w/o queueing to book

    offer->order.size -= order->size;
    order->size = 0;
  }
}

bool cross(t_order * order) {

  list * book = is_ask(order->side) ? bids : asks; // if ask, book is bids and vice versa
  if (!book->head) { // nothing to trade against
    return 0;
  }

  if (order->side) { // sell order

      node * iterator = book->head;
      while (iterator && hit(iterator->order.price, order->price)) {
        trade(order, iterator, book);
        if (!order->size) {
          return 1;
        }
        iterator = iterator->next;
      }

  } else { // buy order
    if (hit(order->price, book->head->order.price)) { // if there's a spread, trade through existing orders one-by-one till fulfill or fail.

      node * iterator = book->head;
      while (iterator && hit(order->price, iterator->order.price)) {
        trade(order, iterator, book);
        if (!order->size) {
          return 1;
        }
        iterator = iterator->next;
      }
    }
  }
  return 0;
}



void queue(t_order * order) {
  if (order->side) { //Ask=1, Bid=0
    add_node(nextid, order, asks, anodespool);
  } else {
    add_node(nextid, order, bids, bnodespool);
  }
}


// -------------------------------------------------- IMPLEMENTATION

void init() {
  nextid = 1;

  anodespool = pool_create(MAX_LIVE_ORDERS, sizeof(node));
  bnodespool = pool_create(MAX_LIVE_ORDERS, sizeof(node));

  asks = malloc(sizeof(list));
  asks->head = NULL;
  asks->side = 1;
  memset(asks->limits, 0, MAX_PRICE * sizeof(void *));
  memset(asks->recents, 0, MAX_RECENTS * sizeof(t_price));

  bids = malloc(sizeof(list));
  bids->head = NULL;
  bids->side = 0;
  memset(bids->limits, 0, MAX_PRICE * sizeof(void *));
  memset(bids->recents, 0, MAX_RECENTS * sizeof(t_price));

}

void destroy() {
  delete_list(bids, bnodespool);
  delete_list(asks, anodespool);
  pool_destroy(anodespool);
  pool_destroy(bnodespool);
}

t_orderid limit(t_order order) {
  if (!cross(&order)) {
    queue(&order); // order is on stack, so someone must copy: happens in add_node
  }
  return nextid++;
}

void cancel(t_orderid orderid) { // if not found do nothing

  node * node = find_by_id(orderid, bids);
  if (node) { // orderid in bids
    remove_node(node, bids, bnodespool);
  } else { // orderid in asks
    node = find_by_id(orderid, asks);
    if (node) {
      remove_node(node, asks, anodespool);
    }
  }
}

