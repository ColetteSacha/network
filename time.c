#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include "time.h"



chrono_t* chrono_new(int seq){
  chrono_t* ret=malloc(sizeof(chrono_t));
  if(ret==NULL){
  	return NULL;
  	}
    ret->seqnum=seq;
  return ret;

}

void chrono_del(chrono_t *chrono) {
  free(chrono);
}

struct timeval chrono_get_temps(chrono_t *chrono){
  return chrono->temps;
}
int chrono_get_seqnum(chrono_t *chrono){
  return chrono->seqnum;
}

void chrono_set_seqnum(chrono_t *chrono,int seq){
  chrono->seqnum=seq;
}

void chrono_set_time(chrono_t *chrono){
  gettimeofday(&(chrono->temps),NULL);
}

struct timeval chrono_get_currentTime(chrono_t* chrono){
  struct timeval celuici=chrono->temps;
  struct timeval maintenant;
  gettimeofday(&maintenant,NULL);
  struct timeval ret;
  ret.tv_sec=maintenant.tv_sec-celuici.tv_sec;
  ret.tv_usec=maintenant.tv_usec-celuici.tv_usec;
  return ret;
}

/*chrono_t** create_tab_chrono(int numb){
  chrono_t* ret[numb];
  for(int i=0;i<numb;i++){
    chrono_t *new=chrono_new(i);
    ret[i]=new;
  }
  return ret;
}

void destroy_tab_chrono(chrono_t** tab,int taille){
  for(int i=0;i<taille;i++){
    chrono_del(tab[i]);
  }
}
*/
