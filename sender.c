<<<<<<< HEAD
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

//Avec l'aide de Vahid Beyraghi et Jonathan Thibaut
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
    char reader[528];
    char writer[8];//8 octets nn? pas 528 vu que ps de payload ni crc2
    memset(reader,0,528);
    memset(writer,0,8);
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
            int length=read(0,reader,528);
            if(length==0)
            {
              //fin du fichier
                return;
            }
            //totalLengthr+=length;

            //todo: METTRE LE CHAR DS UN pkt

            pkt_t* paquetAEnvoyer;
            char charAEnvoyer[528];

            pkt_status_code codeRetour=pkt_encode(paquetAEnvoyer,charAEnvoyer,sizeof(charAEnvoyer));
            if(codeRetour!=PKT_OK){
              fprintf(stderr, "====erreure lors de l'encodage du paquet\n");
            }


            if(write(sfd,charAEnvoyer,sizeof(charAEnvoyer))!=sizeof(charAEnvoyer))
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur write sfd\n");
                return;
            }
            //totalLengthwSfd+=length;

            memset(reader,0,528);
            memset(charAEnvoyer,0,528);
            length=0;
        }
        //reste todo

        if (fds[1].revents & POLLIN){
            int length=read(sfd, writer, 8);
            if(length<0)
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur read socket\n");
                return;
            }
            //totalLengthw+=length;
            if(length==0)
            {
                //fprintf(stderr,"Fin du programme");
                return;
            }

            //pkt_t* paquetDecode=pkt_new();//attention, a free a la fin du programme
            //avoir une fonction pour décoder les ACK



            if(write(1,writer,length)!=length)
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr, "Erreur write stdout\n");
                return;
            }
            //fprintf(stderr, "write on stdout length = %d\n",length);
            //fprintf(stderr, "total length written on stdout: %d\n",totalLengthw);
            memset(writer,0,528);
        }
    }
}
=======





>>>>>>> 29f380014d43c9327d438e0c4ac76946f46f8a5d





