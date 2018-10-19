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

/*
la fonction place dans renvoi le paquet ACK ou NACK qui devra être renvoyé. 
Si la fonction renvoit autre chose que PKT_OK le paquet doit être ignoré
recu est le paquet recu du socket et auquel il faut répondre, window est 
la taille de la fenetre actuel du receveur. timestamp n'est pas encore défini
*/



pkt_status_code reponse (pkt_t* recu, pkt_t *renvoi, uint8_t window, uint32_t timestamp){
	int res = 1;

	if(pkt_get_type(recu) != PTYPE_DATA){
		return E_TR;
	}
	if(pkt_get_tr(recu) != 0){
		return E_TR
	}

	pkt_status_code statEnvoi = PKT_OK;
	pkt_set_tr(renvoi, 0);
	pkt_set_window(renvoi, window);
	pkt_set_length(renvoi, 0);
	pkt_set_timestamp(renvoi, timestamp);


	uint16_t sequnum;
	if(statRecu != PKT_OK){// renvoi un nack
		pkt_set_type(renvoi, PTYPE_NACK);
		sequnum = pkt_get_sequnum(recu);
		if(sequnum >512 || 11 sequnum<0){//seqnum n'est pas acceptable -> ignoré
			return E_SEQNUM;
		}
		pkt_set_sequnum(renvoi, sequnum);
		return PKT_OK;
	}

	if(statRecu == PKT_OK){
		pkt_get_type(renvoi, PTYPE_ACK);
		sequnum = (pkt_get_sequnum(recu)+1)%2;
		pkt_set_sequnum(renvoi, sequnum);
		return PKT_OK;
	}
}



//Avec l'aide de Louis Colin et Jonathan Thibaut
/* 
la fonction lit sur le socket et calcul le paquet qu'il faut répondre
ACK ou NACK. Si la fonction renvoie autre chose que PKT_OK, le paquet recu
doit être ignoré. 
sdf est le file descripteur du socket lu. pkt est le paquet de réponse 
*/
            

            
pkt_status_code read_write_loop(int sfd, int fd) {
    char reader[528];

    while(1)
    {
        struct pollfd fds;
        pkt_t* renvoi;
        renvoi = pkt_new(); // ne pas oublier de free
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

/*
lit le socket, renvoie un ACK ou NACK, écrit sur l'entrée standard et dans le fichier
*/
void recever(int sfd, char* nomFichier){
	int fd = open(nomFichier, O_WRONLY);
	if(fd == -1){
		fprintf(stderr, "erreur dans l'ouverture du fichier de sortie\n");
		exit(EXIT_FAILURE); // il faut voir les consignes
	}

	pkt_status_code status;
	status = read_write_loop(sfd, fd);
	
	int f = close(fd);
	if(fd == -1){
		fprintf(stderr, "erreur dans la fermeture du fichier de sortie\n");
	}
	return;

}

