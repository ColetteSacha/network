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

/*
modification : lit sur le socket et renvoie des ack et nack 
*/
            
            pkt_t* renvoi = pkt_new();

            
pkt_status_code read_write_loop(int sfd, pkt* renvoi, int fd) {
    char reader[528];

    while(1)
    {
        struct pollfd fds;
        fds.fd = sfd;
        fds.events = POLLIN;
        ret = poll(fds, 1, -1 );
        if (ret<0) {
            fprintf(stderr,"select error\n");
            fprintf(stderr,"ERROR: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (fds.revents & POLLIN){
            memset(reader,0,528);
            int length=read(sfd, reader, 528); // 528 est la taille totale du payload(512) + header(16)
            if(length<12)//la taille est plus petite que le header
            {
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur read socket\n");
                return E_UNCONSISTENT;
            }

            if(length == 0){//fin du programme
                return PKT_OK;
            }

            pkt_t* recu = pkt_new();
            pkt_status_code stat = pkt_decode(reader, length, recu);
            if(stat != PKT_OK){
                return stat;
            }
            int rep;
            stat = reponse(recu, renvoi, 31, 0)// taille max du window, timestamp à voir ??
            if(stat != PKT_OK){ // le paquet doit être ignoré
                return stat;
            }

            int w = write(fd, recu->payload, recu->length);
            if(w != -1){ 
                fprintf(stderr, "erreur dans l'écriture du recever \n");
                exit(EXIT_FAILURE);
            }
            if(w != recu->length){
                return E_LENGTH;
            }
            
        }
    }
}
