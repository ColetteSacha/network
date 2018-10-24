#ifndef TIME
#define TIME

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

typedef struct chrono chrono_t;

struct chrono{
struct timeval temps;
struct timeval tempsMax;

};


chrono_t* chrono_new();

void chrono_del(chrono_t *chrono);

struct timeval chrono_get_temps(chrono_t *chrono);

//void chrono_set_temps(chrono_t *chrono);


void chrono_set_time(chrono_t *chrono,struct timeval max);

int chrono_is_ok(chrono_t *chrono);

struct timeval chrono_get_currentTime(chrono_t* chrono);

//chrono_t** create_tab_chrono(int numb);

//void destroy_tab_chrono(chrono_t** tab,int taille);

#endif
