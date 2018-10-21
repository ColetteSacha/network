#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "packet_interface.h"
#include "node.h"



int main(int argc, char *argv[]){
  node_t*first=create_empty_list(3);
  node_t *toSend=first;
  pkt_t*premierPack=pkt_new();
  pkt_set_length(premierPack,1);
  pkt_t*deuxPack=pkt_new();
  pkt_set_length(deuxPack,2);
  pkt_t*troisPack=pkt_new();
  pkt_set_length(troisPack,3);
  node_set_data(first,premierPack);
  toSend=toSend->next;
  node_set_data(toSend,deuxPack);
  toSend=toSend->next;
  node_set_data(toSend,troisPack);
  toSend=toSend->next;
  toSend=find_node(toSend,37,35);
  pkt_t* atest=node_get_data(first);
  printf("%d\n", pkt_get_length(atest));


return 1;
  /*
 node_t*first=create_empty_list(3);
 pkt_t*premierPack=pkt_new();
 pkt_set_length(premierPack,1);
 pkt_t*deuxPack=pkt_new();
 pkt_set_length(deuxPack,2);
 pkt_t*troisPack=pkt_new();
 pkt_set_length(troisPack,3);
 node_set_data(first,premierPack);
 first=first->next;
 node_set_data(first,deuxPack);
 first=first->next;
 node_set_data(first,troisPack);
 first=first->next;
 first=find_node(first,37,35);
 pkt_t* atest=node_get_data(first);
 printf("%d\n", pkt_get_length(atest));

 */


 /*
 pkt_t* atest=node_get_data(first);
 printf("%d\n", pkt_get_length(atest));
 first=first->next;
  atest=node_get_data(first);
 printf("%d\n", pkt_get_length(atest));
 first=first->next;
  atest=node_get_data(first);
 printf("%d\n", pkt_get_length(atest));

 node_t*second=first->next;

 */







/*

  pkt_t*premierPack=pkt_new();
  pkt_set_length(premierPack,45);
  uint16_t lengthPremierPack=pkt_get_length(premierPack);
  printf("%d\n", lengthPremierPack);
  node_t*first=new_node();
  node_set_data(first,premierPack);
  pkt_t*aTest=node_get_data(first);
  printf("%d\n",pkt_get_length(aTest));
  return 1;

  */

}
