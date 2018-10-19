#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

//Fait avec Louis Colin
/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){

struct sockaddr_in6 src_addr;
socklen_t addrlen=sizeof(struct sockaddr_in6);
ssize_t nread;
nread=recvfrom(sfd,NULL,0,MSG_PEEK,(struct sockaddr *) &src_addr,&addrlen);
if(nread==-1){
    fprintf(stderr,"Erreur recvfrom wait_for_client\n");
    return -1;
}
if(connect(sfd,(struct sockaddr *) &src_addr,addrlen)==-1){
    fprintf(stderr,"Erreur connect wait_for_client\n");
    return -1;
}
    //fprintf(stderr,"===Connected after first message - wait_for_client\n");
return 0;
}
