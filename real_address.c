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

//Avec l'aide de Louis Colin

/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval)
{  
    struct addrinfo hints;
    struct addrinfo* res;
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags=AI_CANONNAME;
    
    if(getaddrinfo(address, NULL, &hints, &res)!=0)
    {
        fprintf(stderr,"getaddrinfo fail\n");
        return "getaddrinfo fail\n";
    }
    
    struct sockaddr_in6* tmp = (struct sockaddr_in6*) res->ai_addr;
    memcpy(rval,tmp,sizeof(*tmp));
    fprintf(stderr,"%lu\n",sizeof(rval));
    
    freeaddrinfo(res);
    
    return NULL;
}
