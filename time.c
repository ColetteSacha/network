#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include "time.h"


/*
cree un chrono en lui allouant de la mémoire
*/
chrono_t* chrono_new(){
  chrono_t* ret=malloc(sizeof(chrono_t));
  if(ret==NULL){
  	return NULL;
  	}
  return ret;

}

/*
supprime un chrono en désallouant la mémoire qui lui avait été attribuée
chrono est le chrono qu'on veut supprimer
*/
void chrono_del(chrono_t *chrono) {
  free(chrono);
}

struct timeval chrono_get_temps(chrono_t *chrono){
  return chrono->temps;
}


/*
démarre le chorno. Temps max est la durée maximale avant une retransmission du paquet
*/
void chrono_set_time(chrono_t *chrono, struct timeval max){
  gettimeofday(&(chrono->temps),NULL);
  chrono->tempsMax = max;
}


/*
vérifie si la limite maximale du chrono n'est pas dépassée
return 1 si tout va bien
return 0 si la limite est dépassée
*/
int chrono_is_ok(chrono_t *chrono){
	return chrono_get_currentTime(chrono).tv_sec <= (chrono->tempsMax).tv_sec;//!!!!!!!!!!
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