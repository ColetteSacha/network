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
#include <time.h>
#include <errno.h>
#include <sys/poll.h>

//Avec l'aide de Louis Colin et Jonathan Thibaut
/* Loop reading a socket and printing to stdout,
   * while reading stdin and writing to the socket
   * @sfd: The socket file descriptor. It is both bound and connected.
   * @return: as soon as stdin signals EOF
   */
void read_write_loop(int sfd) {
    int ret=-1;
    int totalLengthr=0;
    int totalLengthwSfd=0;
    int totalLengthw=0;
    char reader[1024];
    char writer[1024];
    memset(reader,0,1024);
    memset(writer,0,1024);
    while(1)
    {
        struct pollfd fds[2];

        fds[0].fd=0;
        fds[0].events=POLLIN;
        fds[1].fd = sfd;
        fds[1].events = POLLIN;
        ret = poll(fds, 2, -1 );
        if (ret<0) {
            fprintf(stderr,"select error\n");
            fprintf(stderr,"ERROR: %s\n", strerror(errno));
            return;
        }
        if (fds[0].revents & POLLIN)
        {
            int length=read(0,reader,1024);   
            if(length==0)
            {
                //printf("Fin du programme!\n");
                return;
            }  
            totalLengthr+=length;
            //fprintf(stderr, "read from stdin length = %d\n", length);
            //fprintf(stderr, "total length read from stdin: %d\n",totalLengthr);
            if(write(sfd,reader,length)!=length)
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur write sfd\n");
                return;
            }
            totalLengthwSfd+=length;
            //fprintf(stderr, "write on sfd length = %d\n", length);
            //fprintf(stderr, "total length write on sfd: %d\n",totalLengthwSfd);
            memset(reader,0,1024);
            length=0;
        }

        if (fds[1].revents & POLLIN){
            int length=read(sfd, writer, 1024);
            if(length<0)
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur read socket\n");
                return;
            }
            totalLengthw+=length;
            if(length==0)
            {
                //fprintf(stderr,"Fin du programme");
                return;
            }
            if(write(1,writer,length)!=length)
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr, "Erreur write stdout\n");
                return;
            }
            //fprintf(stderr, "write on stdout length = %d\n",length);
            //fprintf(stderr, "total length written on stdout: %d\n",totalLengthw);
            memset(writer,0,1024);                
        }
    }
}
