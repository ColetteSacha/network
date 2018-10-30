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
#include <sys/stat.h>
#include <fcntl.h>

#include "packet_interface.h"
#include "node.h"
#include "wait_for_client.h"
#include "create_socket.h"
#include "real_address.h"

//note: les exercices inginious on été réalisé avec l'aide de Guillaume Bellon
//et Vahid Beyraghi


//optind





/*
la fonction place dans renvoi le paquet ACK ou NACK qui devra être renvoyé.
Si la fonction renvoit autre chose que PKT_OK le paquet doit être ignoré
recu est le paquet recu du socket et auquel il faut répondre, window est
la taille de la fenetre actuel du receveur. timestamp n'est pas encore défini
*/






pkt_status_code reponse (pkt_t* recu, pkt_t *renvoi, uint8_t window, uint32_t timestamp, int seqnumDebut, int seqnumFin, int* decalage){

	uint8_t seqnum = pkt_get_seqnum(recu);

	// if(seqnum >512){//seqnum n'est pas acceptable -> ignoré
		// return E_SEQNUM;
	// }




    pkt_status_code stat = difference(seqnumDebut, seqnumFin, seqnum, decalage);




    //if(stat != PKT_OK){

    	//return stat;
    //}

    int tr = pkt_get_tr(recu);
    pkt_set_tr(renvoi, 0);
		pkt_set_window(renvoi, window);
		pkt_set_length(renvoi, 0);
		pkt_set_timestamp(renvoi, timestamp);
		if(stat==E_WINDOW){
			pkt_set_seqnum(renvoi,seqnumDebut);
			pkt_set_type(renvoi,PTYPE_ACK);
			return E_WINDOW;
		}


    if(*decalage != 0){//*decalage =! 0
    	if(tr){//signal tronqué et mauvais seqnum, on renvoi un nack avec le seqnum recu
    		pkt_set_type(renvoi, PTYPE_NACK);
    		pkt_set_seqnum(renvoi, seqnum);
    		return PKT_OK;
    	}
    	else{//mauvais seqnum, on renvoi un ack avec le seqnum attendu mais il faut enregistrer paquet dans le buffer
    		pkt_set_type(renvoi, PTYPE_ACK);
    		pkt_set_seqnum(renvoi, seqnumDebut);
    		return PKT_OK;
    	}
    }
    if(*decalage == 0){//decalge

    	if(tr){//bon seqnum mais signal tronqué, on renvoi un nack avec le num de sequence recu

    		pkt_set_type(renvoi, PTYPE_NACK);
    		pkt_set_seqnum(renvoi, seqnum);
    		return PKT_OK;
    	}
    	else{

				//signal correct et bon num de séquence, on renvoi un ack avec le prochain seqnum attendu
    		pkt_set_type(renvoi, PTYPE_ACK);
    		if(seqnum == 255){
    			seqnum = -1;
    		}
    		pkt_set_seqnum(renvoi, seqnum+1);
    		return PKT_OK;

    	}
    }
		return PKT_OK;
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
    int seqnumDebut = 0;
    int seqnumFin = 31;
    int* decalage;
		int deconnection = 0;
		int tailleBuffer = 0;
    decalage = malloc(sizeof(int));
	node_t* current;
	node_t* runner;
	current = create_empty_list(62); //2 fois la taille max de la window
	runner = current;
	size_t taille = 12;

    while(!(deconnection && (tailleBuffer==0)))
    {
			printf("===l150\n" );
        struct pollfd fds;
        pkt_t* renvoi;
        renvoi = pkt_new(); // ne pas oublier de free
        fds.fd = sfd;
        fds.events = POLLIN;
        int ret = poll(&fds, 1, -1 );
        if (ret<0) {
            fprintf(stderr,"select error\n");
            fprintf(stderr,"ERROR: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (fds.revents & POLLIN){
					printf("receiver recoit qq ch\n");
            memset(reader,0,528);
            int length=read(sfd, reader, 528); // 528 est la taille totale du payload(512%s) + header(16)
						printf("===length=%d\n", length);
            if(length<12)//la taille est plus petite que le header
            {
							  printf("length<12\n" );
                fprintf(stderr,"ERROR: %s\n", strerror(errno)); // il faut ignorer le fichier
                fprintf(stderr,"Erreur read socket\n");
                //return E_UNCONSISTENT;
            }

            if(length == 12){//fin du programme
							printf("normalement, fin du while receiver\n" );
							deconnection = 1;
            }

            pkt_t* recu = pkt_new();
            pkt_status_code stat = pkt_decode(reader, length, recu);
            if(stat != PKT_OK){
                fprintf(stderr, "erreur décode \n"); // ne pas sortir de la boucle ??
            }

            char renvoiChar[12];
            stat = reponse(recu, renvoi, 31, pkt_get_timestamp(recu), seqnumDebut, seqnumFin, decalage);



            if(stat == PKT_OK){


            	if(pkt_get_type(renvoi) == PTYPE_NACK){//le message recu était tronqué

            		pkt_encode(renvoi, renvoiChar, &taille);//12
            		if(write(sfd, renvoiChar, 12)!=12){
									pkt_del(renvoi);
									pkt_del(recu);
									destroy_list(runner);
									free(decalage);
									printf("Erreur write sfd\n");
	                exit(EXIT_FAILURE);
								}
								printf("le message recu était tronqué\n" );
								pkt_del(renvoi);
								pkt_del(recu);
            	}
            	if(pkt_get_type(renvoi) == PTYPE_ACK){

            		if(*decalage == 0){//le message recu n'était pas tronqué et le seqnum est correct, on écrit sur le fichier
            			seqnumDebut = seqnumDebut+1;
            			seqnumFin = seqnumFin+1;
            			if(seqnumDebut == 256){
            				seqnumDebut = 0;
            			}
            			if(seqnumFin == 256){
            				seqnumFin = 0;
            			}

            			if(write(fd, pkt_get_payload(recu), pkt_get_length(recu))!=pkt_get_length(recu)){
										pkt_del(renvoi);
										pkt_del(recu);
										destroy_list(runner);
										free(decalage);
										printf("Erreur write sfd\n");
		                exit(EXIT_FAILURE);
									}

            			pkt_encode(renvoi, renvoiChar, &taille);//12
									printf("le message est de type Ack et bon seqnum. Seqnum=%d\n", pkt_get_seqnum(renvoi));
									if(write(sfd, renvoiChar, 12)!=12){
										pkt_del(renvoi);
										pkt_del(recu);
										destroy_list(runner);
										free(decalage);
										printf("Erreur write sfd\n");
		                exit(EXIT_FAILURE);
									}
									pkt_del(renvoi);
									pkt_del(recu);
            			while(node_get_data(current) != NULL){//vide buf
            				pkt_t* videBuffer = node_get_data(current);
            				//write(1, pkt_get_payload(videBuffer), pkt_get_length(videBuffer));
            				if(write(fd, pkt_get_payload(videBuffer), pkt_get_length(videBuffer))!=pkt_get_length(videBuffer)){
											pkt_del(renvoi);
											pkt_del(recu);
											destroy_list(runner);
											free(decalage);
											printf("Erreur write sfd\n");
			                exit(EXIT_FAILURE);
										}
            				seqnumDebut = seqnumDebut + 1;
            				seqnumFin = seqnumFin + 1;
            				if(seqnumDebut == 256){
            					seqnumDebut = 0;
            				}
            				if(seqnumFin == 256){
            					seqnumFin = 0;
            				}

            				node_set_data(current, NULL);
            				current = node_get_next(current);
            				runner = current;
            				pkt_del(videBuffer);
										tailleBuffer--;
            			}
  	         		}
  	         		else{// pas tronqué mais le seqnum n'est pas correct
  	         			for(int i = 0; i<*decalage-1; i++){
  	         				runner = node_get_next(runner);
  	         			}
  	         			node_set_data(runner, recu);
  	         			runner = current;
  	         			pkt_encode(renvoi, renvoiChar, &taille); //12
									printf("pas tronque mais seqnum pas correct\n");
									if(write(sfd, renvoiChar, 12)!=12){
										pkt_del(renvoi);
										pkt_del(recu);
										destroy_list(runner);
										free(decalage);
										printf("Erreur write sfd\n");
		                exit(EXIT_FAILURE);
									}
									pkt_del(renvoi);
									pkt_del(recu);
									tailleBuffer++;
  	         		}
            	}
            }
						else{
							pkt_encode(renvoi, renvoiChar, &taille);//12
							printf("le message est de type E_WINDOW Seqnum=%d\n", pkt_get_seqnum(renvoi));
							if(write(sfd, renvoiChar, 12)!=12){
								pkt_del(renvoi);
								pkt_del(recu);
								destroy_list(runner);
								free(decalage);
								printf("Erreur write sfd\n");
								exit(EXIT_FAILURE);
							}
							pkt_del(recu);
							pkt_del(renvoi);
						}

        }
    }
		free(decalage);
destroy_list(runner);
return (PKT_OK);
}







/*
lit le socket, renvoie un ACK ou NACK, écrit sur l'entrée standard et dans le fichier
*/
void receiver2(int sfd, char* nomFichier){ //si il n'y a pas de fichier ??
 int fd;
 if(nomFichier==NULL){
	 fd=1;
 }
 else{
	 fd = open(nomFichier, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
 }

	if(fd == -1){
		fprintf(stderr, "erreur dans l'ouverture du fichier de sortie\n");
		exit(EXIT_FAILURE); // il faut voir les consignes
	}

	pkt_status_code status;
	status = read_write_loop(sfd, fd);
	if(status!=PKT_OK){
		exit(EXIT_FAILURE);
	}

	if(close(fd)<0){
		fprintf(stderr, "erreur dans la fermeture du fichier de sortie\n");
	}
	return;

}

int receiver(int argc, char *argv[]){
int opt;
int port;
char* nomFichier=NULL;
char* hostName;
	while((opt = getopt(argc, argv, "f:")) != -1)
	{
		switch(opt)
			case 'f':
			nomFichier = optarg;
			break;
	}

	port = atoi(argv[optind + 1]);
	hostName = argv[optind];

	struct sockaddr_in6 addr;
	int sfd;
	real_address(hostName, &addr);
	sfd = create_socket(&addr, port, NULL, -1); /* Bound */
		if (sfd > 0 && wait_for_client(sfd) < 0) { /* Connected */
			fprintf(stderr,
					"Could not connect the socket after the first message.\n");
			close(sfd);
			return EXIT_FAILURE;
		}

	if (sfd < 0) {
		fprintf(stderr, "Failed to create the socket!\n");
		return EXIT_FAILURE;
	}
	/* Process I/O */
	receiver2(sfd, nomFichier);

	close(sfd);
	return EXIT_SUCCESS;
}
