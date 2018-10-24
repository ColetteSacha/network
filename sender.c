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
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>



#include "create_socket.h"
#include "packet_interface.h"
#include "node.h"
#include "real_address.h"

int premierMessage=1;
int wmin=0;//la fenetre du sender est caractérisee par un numéro de début et de fin
int wmax=31;
int notSendOnes=1;
node_t *current;
node_t *toSend;
node_t *finWind;
int numeroDeSequence=0;


struct timeval retransmissionTimer; 
struct timeval premierTimer; // retransmission timer pour le premier paquet
premierMessage.tv_sec = 5;
premierMessage.tv_usec = 0;




   int resend(int numseq,int sfd){

     node_t* noeudAenvoyer=find_node(current,wmin,wmax,numseq);
     pkt_t* paquetAEnvoyer=noeudAenvoyer->data;

     char charAEnvoyer[528];
     size_t l=528*sizeof(char);

     pkt_status_code codeRetour=pkt_encode(paquetAEnvoyer,charAEnvoyer,&l);
     if(codeRetour!=PKT_OK){
       fprintf(stderr, "====erreure lors de l'encodage du paquet\n");
     }

     if(premierMessage){
      chrono_set_time(node_get_chrono(noeudAenvoyer), premierTimer); // réinitialise le chrono si c'est le premier message
     }
     else{
      chrono_set_time(node_get_chrono(noeudAenvoyer), retransmissionTimer);//réinitialise le chrono 
     }



     if(write(sfd,charAEnvoyer,sizeof(charAEnvoyer))!=sizeof(charAEnvoyer))
     {
        destroy_list(current);
         fprintf(stderr,"ERROR: %s\n", strerror(errno));
         fprintf(stderr,"Erreur write sfd\n");
         return 0;
     }
   memset(charAEnvoyer,0,528);
   return 1;


   }





void read_write_loop(int sfd,int fdEntree) {
    int ret=-1;
    current=create_empty_list(62);
    toSend=current;
    finWind=find_node(current,0,31,31);
    node_t runner = current;

    //int totalLengthr=0;
    //int totalLengthwSfd=0;
    //int totalLengthw=0;
    char reader[512];
    char writer[12];//12 octets nn? pas 528 vu que ps de payload ni crc2
    memset(reader,0,528);
    memset(writer,0,12);



    while(1)
    {
        struct pollfd fds[2];

        fds[0].fd=fdEntree;
        fds[0].events=POLLIN;
        fds[1].fd = sfd;
        fds[1].events = POLLIN;
        ret = poll(fds, 2, -1 );
        if (ret<0) {
            destroy_list(current);
            fprintf(stderr,"select error\n");
            fprintf(stderr,"ERROR: %s\n", strerror(errno));
            return;
        }
        if ((fds[0].revents & POLLIN))
        {
          if(premierMessage && notSendOnes){//si c'est le premier message, on ne doit pas tout envoyer d'un coup
            notSendOnes=0;

            int length=read(fdEntree,reader,512);
            if(length==0)
            {
              //fin du fichier
              destroy_list(current);
              fprintf(stderr, "======== erreur ligne116" );

                return;
            }
            //totalLengthr+=length;

            //todo: METTRE LE CHAR DS UN pkt(node_get_data(current[0]))
            pkt_t *un=pkt_new();


            pkt_status_code status= create_packet(reader,length,31,numeroDeSequence,0,un);
            fprintf(stderr, "length=%d\n",length );

            if(status!=PKT_OK){
              fprintf(stderr, "first\n" );
              fprintf(stderr, "%d\n", status);
              fprintf(stderr, "E_LENGTH=%d\n", E_LENGTH );

              fprintf(stderr, "====erreure lors du create_packet\n" );


              return;
            }
            node_set_data(toSend,un);


            numeroDeSequence++;


            char charAEnvoyer[528];
            size_t l=528*sizeof(char);

            pkt_status_code codeRetour=pkt_encode(node_get_data(toSend),charAEnvoyer,&l);
            if(codeRetour!=PKT_OK){
              fprintf(stderr, "first\n" );
              fprintf(stderr, "====erreure lors de l'encodage du paquet\n");
            }
            pkt_t* pktforLen=node_get_data(toSend);

            chrono_set_time(node_get_chrono(toSend), premierTimer); // démarre le chrono



            if(write(sfd,charAEnvoyer,16+pkt_get_length(pktforLen))!=16+pkt_get_length(pktforLen))
            {
                destroy_list(current);
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur write sfd\n");
                return;
            }
            //totalLengthwSfd+=length;

            memset(reader,0,512);
            memset(charAEnvoyer,0,528);
            length=0;
            toSend=toSend->next;
          }



          else{

            if(!premierMessage){

              //pkt_t* chekeWindow=node_get_data(toSend);//!!!!

              while(toSend!=finWind){

                int length=read(fdEntree,reader,512);
                if(length==0)
                {
                  destroy_list(current);
                  //fin du fichier
                    return;
                }
                //totalLengthr+=length;

                node_set_data(toSend,pkt_new());//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                if(create_packet(reader,length,31,numeroDeSequence,0,node_get_data(toSend))!=PKT_OK){
                  fprintf(stderr, "second\n" );

                  fprintf(stderr, "====erreure lors du create_packet" );
                  return;
                }
                numeroDeSequence++;
                if(numeroDeSequence==256){
                  numeroDeSequence=0;
                }


                char charAEnvoyer[528];
                size_t len=528*sizeof(char);

                pkt_status_code codeRetour=pkt_encode(node_get_data(toSend),charAEnvoyer,&len);
                //!!!!!!!!!!!!!pas paquet AEnvoyer ms packet ds current!!!!!!!!!!!!!!!!!!!


                if(codeRetour!=PKT_OK){
                  fprintf(stderr, "====erreure lors de l'encodage du paquet\n");
                }


                chrono_set_time(node_get_chrono(toSend), retransmissionTimer); // démarre le chrono

                if(write(sfd,charAEnvoyer,sizeof(charAEnvoyer))!=sizeof(charAEnvoyer))
                {
                   destroy_list(current);
                    fprintf(stderr,"ERROR: %s\n", strerror(errno));
                    fprintf(stderr,"Erreur write sfd\n");
                    return;
                }
                //totalLengthwSfd+=length;

                memset(reader,0,512);
                memset(charAEnvoyer,0,528);
                length=0;
                toSend=toSend->next;

              }






          }
        }//fin de l'envoi ds la possibilité de la window
      }//fin du fait qu'on a la possibilité de lecture du stdin ou fichier
        //reste todo




        if (fds[1].revents & POLLIN){
            int length=read(sfd, writer, 12);
            if(length<=0)
            {
                destroy_list(current);
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                fprintf(stderr,"Erreur read socket\n");
                return;
            }
            //totalLengthw+=length;
          /*  if(length==0)//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            {
                destroy_list(current);
                //fprintf(stderr,"Fin du programme");
                return;
            }

            */

            pkt_t* paquetDecode=pkt_new();//attention, a pkt_del() a la fin du programme
            memcpy(paquetDecode,writer,2);
            if(pkt_get_type(paquetDecode)!=PTYPE_ACK){//le paquet recu n'est pas un ack
            resend(pkt_get_seqnum(paquetDecode),sfd);
            pkt_del(paquetDecode);
            }
            else{//paquest recu est de type ack. Acheke:numéro de séquence
              if(premierMessage){//si la lecture était le premier message
                pkt_del(paquetDecode);
                pkt_del(node_get_data(current));
                premierMessage=0;
                current=current->next;
                finWind=finWind->next;
                wmin++;
                wmax++;

                retransmissionTimer = chrono_get_currentTime(current); //calcul du retransmission timer
                retransmissionTimer.tv_sec = retransmissionTimer.tv_sec+2;


              }
              else{//on cheke si le numéro de séquence recu est celui attendu
                if(pkt_get_seqnum(paquetDecode)==wmin){//!!!!!!!!!!!!!!!!!!!!!!!!!!
                  resend(pkt_get_seqnum(paquetDecode),sfd);
                  pkt_del(paquetDecode);
                }
                else{//grace aux acquis cumulatifs, on sait que tous les paquets ont bien été recu
// jusqu'au numéro de séquence recu
                    int* nbrAdecaler=malloc(sizeof(int));
                    difference(wmin,wmax,pkt_get_seqnum(paquetDecode),nbrAdecaler);
                    for(int i=0;i<*nbrAdecaler;i++){


                      pkt_del(node_get_data(current));
                      current=current->next;
                      finWind=finWind->next;
                      wmin++;
                      wmax++;
                      if(wmin==255){
                        wmin=0;
                      }
                      if(wmax==255){
                        wmax=0;
                      }
                    }
                    
                    pkt_del(paquetDecode);
                    free(nbrAdecaler);
                }
              }//fin du else qui regarde si le numéro de seq est celui attendu
            }//fin du else qui regarde si le le paquet est de type ack

        }//fin de la lecture du sfd


        pkt_t* renvoi;

        if(premier){
          if(!chorno_is_ok(node_get_chrono(current))){
            renvoi = node_get_data(current);
            resend(pkt_get_seqnum(renvoi), sdf);
          }
        }

        for(int i = wmin; i!=wmax, i++){
          if(!chorno_is_ok(node_get_chrono(runner))){
            renvoi = node_get_data(runner);
            resend(pkt_get_seqnum(renvoi), sfd); //si le chrono est dépassé le paquet est réenvoyé
            runner = node_get_next(runner);
            if(wmin == 256){
              wmin = 0;
            }
            if(wmax == 256){
              wmax = 0;
            }
          }
        }
        runner = current;
    }//fin du while
}






void sender(int sfd, char* nomFichier){
  int fd;
  if(nomFichier==NULL){
    fd=0;
  }
  else{
    fd=open(nomFichier,O_RDONLY);

  }
  if(fd == -1){
		fprintf(stderr, "erreur dans l'ouverture du fichier d'entree\n");
		exit(EXIT_FAILURE); // il faut voir les consignes
	}
  read_write_loop(sfd, fd);
  int f = close(fd);
	if(fd == -1){
		fprintf(stderr, "erreur dans la fermeture du fichier d'entrée\n");
	}
	return;

}



int main(int argc, char *argv[]){
  printf("ligne326:ok\n" );

 int opt;
 int port;
 char* nomFichier=NULL;
 char* hostName;
 while((opt = getopt(argc, argv, "f:")) != -1){
   switch(opt){
     case 'f':
     nomFichier = optarg;
     break;

     default:
     fprintf(stderr,"option inconnue");
     return -1;
   }
 }
 hostName=argv[optind];
 port=atoi(argv[optind+1]);
 fprintf(stderr, "numéro de port: %d\n", port);


struct sockaddr_in6 addr;
const char *err=real_address(hostName,&addr);
if(err){
  fprintf(stderr, "Could not resolve hostname %s: %s\n", hostName, err);
  return EXIT_FAILURE;
}

int sfd=create_socket(NULL,-1,&addr,port);/* Connected */
if(sfd<0){
  fprintf(stderr, "Failed to create the socket!\n");
  return EXIT_FAILURE;
}

sender(sfd,nomFichier);
close(sfd);



}

