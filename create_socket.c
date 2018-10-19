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


/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port){
  
    //fprintf(stderr,"===CREATION SOCKET===\n");    
    int sfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if(sfd==-1)
    {
        fprintf(stderr,"Erreur création socket\n");
        return -1;
    }
    //fprintf(stderr,"===CREATION SOCKET OK===\n");
        
    size_t addrlen = sizeof(struct sockaddr_in6);   
    
    if(src_port>0)
    {
        source_addr->sin6_port=htons(src_port);
        //fprintf(stderr,"initialisation source OK\n");
    }
    
    
    if(dst_port>0)
    {
        dest_addr->sin6_port=htons(dst_port);
        //fprintf(stderr,"initialisation dest OK\n");
    }
    
    if(source_addr!=NULL)
    {
        if(bind(sfd,(struct sockaddr*) source_addr, addrlen)!=0)
        {
            fprintf(stderr,"error bind\n");
            return -1;
        }   
        //fprintf(stderr,"bind OK\n");
    }
   
    if(dest_addr!=NULL)
    {
        if(connect(sfd,(struct sockaddr*) dest_addr, addrlen)!=0)
        {
            fprintf(stderr,"error connect\n");
            return -1;
        }
        //fprintf(stderr,"connect OK\n");
    }
    
    return sfd;
}
