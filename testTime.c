#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>

#include "time.h"


int main(int argc, char *argv[]){
chrono_t** test=create_tab_chrono(3);
chrono_t* zero=chrono_new(0);
test[0]=zero;
chrono_t* first=chrono_new(1);
test[1]=first;

printf("%d\n", chrono_get_seqnum(test[1]));

  return 1;
}
