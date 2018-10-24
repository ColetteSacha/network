#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <time.h>

#include "packet_interface.h"
#include "node.h"
#include "time.h"








node_t* new_node(){
  node_t *ret=(node_t*)calloc(1,sizeof(node_t));
  if(ret==NULL){
      return NULL;
  }
  return ret;
}


void node_del(node_t* n){
  if(n!=NULL){
    if(n->data!=NULL){
      pkt_del(n->data);
    }
    if(n->chrono!=NULL){
      chrono_del(n->chrono);
    }
    free(n);
    n=NULL;
  }
}

void node_set_data(node_t *node,pkt_t *new){
  node->data=new;
}

void node_set_next(node_t *node,node_t* newNext){
  node->next=newNext;
}

void node_set_chrono(node_t *node){//réinitialise le temps d'un chrono
  chrono_set_time(node->chrono);
}

pkt_t* node_get_data(node_t *node){
  return node->data;
}

node_t* node_get_next(node_t *node){
  return node->next;
}

chrono_t* node_get_chrono(node_t *node){
  return node->chrono;
}

void destroy_list(node_t* current){
  node_t* runner = current;
  while(current != node_get_next(current)){
    runner = node_get_next(current);
    node_set_next(current, node_get_next(runner));
    node_del(runner);
    runner = current;
  }
  node_del(current);
}






node_t* create_empty_list(int number)
{

  node_t* ret=new_node();

  if(number==1){
    return ret;
  }
  node_t*current=ret;
  for(int i=1;i<number;i++){
    node_t* new=new_node();
    current->next=new;
    current=current->next;
  }
  current->next=ret;
  return ret;
}







pkt_status_code difference(int seqnumDebut, int seqnumFin, int seqnum, int* decalage){




  if(seqnumDebut%256<seqnumFin%256){

        if(!(seqnum>=seqnumDebut && seqnum<=seqnumFin)){// !!!! avant seqnum>seqnumDebut
            return E_SEQNUM;
        }
        *decalage = (seqnum%256) - (seqnumDebut%256);
    }
    if(seqnumDebut%256>seqnumFin%256){
        if(!(seqnum<seqnumDebut || seqnum>seqnumFin)){

            return E_SEQNUM;
        }
        if((seqnum%256)>(seqnumDebut%256)){
            *decalage = (seqnum%256)-(seqnumDebut%256);
        }
        else{
            *decalage = (seqnum%256) + (255 - (seqnumDebut%256));
        }
    }
    return PKT_OK;
}

node_t* find_node(node_t* current,int wmin,int seqnumFin, int place){
  int *nbrIteration=malloc(sizeof(int));
  difference(wmin,seqnumFin,place,nbrIteration);
  for(int i=0;i<*nbrIteration;i++){
    current=current->next;
  }
  free(nbrIteration);
  return current;
}
