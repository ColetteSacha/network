#ifndef TIME
#define TIME

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

typedef struct chrono chrono_t;

struct chrono{
struct timeval temps;
int seqnum;

};


chrono_t* chrono_new();

void chrono_del(chrono_t *chrono);

struct timeval chrono_get_temps(chrono_t *chrono);
int chrono_get_seqnum(chrono_t *chrono);

void chrono_set_seqnum(chrono_t *chrono,int seq);

void chrono_set_time(chrono_t *chrono);

struct timeval chrono_get_currentTime(chrono_t* chrono);

#endif
