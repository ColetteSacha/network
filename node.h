#ifndef __NODE_H_
#define __NODE_H_

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "time.h"
#include "packet_interface.h"

//ATEENTION, INCLUDE LE TRUC DES PKT





typedef struct node {
pkt_t *data;
struct node *next;
chrono_t* chrono;
}node_t;



node_t* new_node();
void node_del(node_t* n);
void node_set_data(node_t *node,pkt_t *new);
void node_set_next(node_t *node,node_t* newNext);
void node_set_chrono(node_t *node,struct timeval max);
pkt_t* node_get_data(node_t *node);
node_t* node_get_next(node_t *node);
chrono_t* node_get_chrono(node_t *node);
node_t* create_empty_list(int number);
void destroy_list(node_t* current);
pkt_status_code difference(int seqnumDebut, int seqnumFin, int seqnum, int* decalage);
node_t* find_node(node_t* current,int wmin,int seqnumFin, int place);

#endif
