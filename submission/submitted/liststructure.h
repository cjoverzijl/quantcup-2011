#ifndef BUYLIST_H_
#define BUYLIST_H_
#include <stdlib.h>
#include "limits.h"
#include "types.h"

#define MAX_RECENTS 10

struct node_struct { // order speeds up dereferences, TODO: apply refactor elsewhere
  struct node_struct * next; //necessary to implement list, this is nearest lower price for buy and nearest higher price for sell
  struct node_struct * prev;
  t_order order;
  t_orderid id;
};


typedef struct node_struct node;

typedef struct ll {
  node * head;
  node * limits[MAX_PRICE];
  node * idtable[MAX_LIVE_ORDERS];
  t_price recents[MAX_RECENTS];
  int side;
} list;

list * init_list(list * ll);

node * find_by_id(t_orderid id, list * ll) ;

node * quickfind1(t_price limit, node * addresses[] );
node * quickfind2(t_price limit, list * ll);
node * quickfind3(t_price limit, list * ll );

node * better_recent_price(t_price limit, list * ll);
void add_recent_price(t_price limit, list * ll);

void add_node(t_orderid id, t_order * order, list * ll, pool_t * nodespool );

void remove_node(node * rv, list * ll, pool_t * nodespool) ;

void delete_list(list * ll, pool_t * nodespool);


#endif /* BUYLIST_H_ */
