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
#include "time.h"


//note: les exercices inginious on été réalisé avec l'aide de Guillaume Bellon
//et Vahid Beyraghi



int premierMessage=1;
int wmin=0;//la fenetre du sender est caractérisee par un numéro de début et de fin
int wmax=31;
int notSendOnes=1;
node_t *current;
node_t *toSend;
node_t *finWind;
node_t* runner;
int numeroDeSequence=0;


struct timeval retransmissionTimer;
struct timeval premierTimer; // retransmission timer pour le premier paquet



/*
la fonction réenvoi le paquet dont le numéro de paquet est numseq. Le paquet doit
se trouver dans le buffer. sfd est le file descriptor du socket.
*/
    int resend(int numseq,int sfd){

      node_t* noeudAenvoyer=find_node(current,wmin,wmax,numseq);
      pkt_t* paquetAEnvoyer=node_get_data(noeudAenvoyer);
        if(pkt_get_length(paquetAEnvoyer)==0){
          char charAEnvoyer[12];
          size_t l=12*sizeof(char);
          pkt_status_code codeRetour=pkt_encode(paquetAEnvoyer,charAEnvoyer,&l);
          if(codeRetour!=PKT_OK){
            printf("====erreure lors de l'encodage du paquet\n");
          }

          int *nbrIteration=malloc(sizeof(int));
          difference(wmin,wmax,numseq,nbrIteration);
          node_t* NoeudTimeReset=current;
          for(int i=0;i<*nbrIteration;i++){
            current=node_get_next(current);
          }


          chrono_set_time(node_get_chrono(current), retransmissionTimer);//réinitialise le chrono
          current=NoeudTimeReset;
          free(nbrIteration);




          if(write(sfd,charAEnvoyer,sizeof(charAEnvoyer))!=sizeof(charAEnvoyer))
          {
            destroy_list(current);
            printf("ERROR: %s\n", strerror(errno));
            printf("Erreur write sfd(s1)\n");
            return 0;
       }
     memset(charAEnvoyer,0,12);

     return 1;



      }
      else{
        char charAEnvoyer[pkt_get_length(paquetAEnvoyer)+16];
        size_t l=(pkt_get_length(paquetAEnvoyer)+16)*sizeof(char);


       pkt_status_code codeRetour=pkt_encode(paquetAEnvoyer,charAEnvoyer,&l);
       if(codeRetour!=PKT_OK){
         printf("====erreure lors de l'encodage du paquet\n");
       }

       if(premierMessage){
         int *nbrIteration=malloc(sizeof(int));
         difference(wmin,wmax,numseq,nbrIteration);
         node_t* NoeudTimeReset=current;
         for(int i=0;i<*nbrIteration;i++){
           current=node_get_next(current);
         }
        chrono_set_time(node_get_chrono(current), premierTimer); // réinitialise le chrono si c'est le premier message
        current=NoeudTimeReset;
        free(nbrIteration);
       }
       else{
         int *nbrIteration=malloc(sizeof(int));
         difference(wmin,wmax,numseq,nbrIteration);
         node_t* NoeudTimeReset=current;
         for(int i=0;i<*nbrIteration;i++){
           current=node_get_next(current);
         }


        chrono_set_time(node_get_chrono(current), retransmissionTimer);//réinitialise le chrono
        current=NoeudTimeReset;
        free(nbrIteration);
       }



       if(write(sfd,charAEnvoyer,sizeof(charAEnvoyer))!=(int)sizeof(charAEnvoyer))
       {
          destroy_list(current);
           printf("ERROR: %s\n", strerror(errno));
           printf("Erreur write sfd(s2)\n");
           return 0;
       }
     memset(charAEnvoyer,0,(pkt_get_length(paquetAEnvoyer)+16));

     return 1;
      }
   }



/*
la fonction lit depuis un fichier ou l'entrée standard. 
Elle transforme ce qu'elle lit en paquet, les encode et les écrit sur le sfd
Elle lit ensuite sur le sfd les accusé de réception et réenvoi les paquets si besoin
sfd est le file descriptor du socket, fd est le file descriptor du fichier à lire
Au préalable sfd et fd doivent avoir été ouvert
*/
void read_write_loop(int sfd,int fdEntree) {
    int ret=-1;
    current=create_empty_list(61);
    toSend=current;
    finWind=find_node(current,0,31,31);
    premierTimer.tv_sec = 5;
    premierTimer.tv_usec = 0;
    int deconnection=0;
    int countDeconnection=0;
    //int finLecture = 0;
    int seqnumDeconnection=-2;

    runner = current;


    //int totalLengthr=0;
    //int totalLengthwSfd=0;
    //int totalLengthw=0;
    char reader[512];
    char writer[12];//12 octets nn? pas 528 vu que ps de payload ni crc2
    memset(reader,0,512);
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
            printf("select error\n");
            printf("ERROR: %s\n", strerror(errno));
            return;
        }
        if ((fds[0].revents & POLLIN))
        {
          //printf("===l131\n" );
          if(premierMessage && notSendOnes){//si c'est le premier message, on ne doit pas tout envoyer d'un coup
            notSendOnes=0;

            int length=read(fdEntree,reader,512);
            if(length==0)
            {
              //fin du fichier

              
              return;
            }
            pkt_t *un=pkt_new();
   


            pkt_status_code status= create_packet(reader,length,31,numeroDeSequence,0,un);
            //printf("length=%d\n",length );

            if(status!=PKT_OK){

              printf("====erreure lors du create_packet\n" );


              return;
            }
            node_set_data(toSend,un);


            numeroDeSequence++;


            char charAEnvoyer[16+length];
            size_t l=(16+length)*sizeof(char);

            pkt_status_code codeRetour=pkt_encode(node_get_data(toSend),charAEnvoyer,&l);
            if(codeRetour!=PKT_OK){
              printf("====erreure lors de l'encodage du paquet\n");
            }
            pkt_t* pktforLen=node_get_data(toSend);

            chrono_set_time(node_get_chrono(toSend), premierTimer); // démarre le chrono



            if(write(sfd,charAEnvoyer,16+pkt_get_length(pktforLen))!=16+pkt_get_length(pktforLen))
            {
                destroy_list(current);
                printf("Erreur write sfd(s3)\n");
                return;
            }
            //totalLengthwSfd+=length;

            memset(reader,0,512);
            memset(charAEnvoyer,0,(16+length));
            length=0;
            toSend=node_get_next(toSend);

          }



          else{
            if(!premierMessage){

             

              while(toSend!=finWind && !deconnection){

                int length=read(fdEntree,reader,512);
                if(length==0)
                {
                  //fin du fichier
                  
                  deconnection=1;
                  seqnumDeconnection = numeroDeSequence;
                  pkt_t* pkt_disconnect = pkt_new();
                  if(create_packet(NULL, 0, 1, numeroDeSequence, 0, pkt_disconnect) != PKT_OK){
                      printf("erreur dans la création du paquet de déconnection\n");
                      return;
                  }
                  pkt_del(node_get_data(toSend));
                  node_set_data(toSend, pkt_disconnect);
                  char pktChar[12];
                  size_t len2 = 12*sizeof(char);
                  if(pkt_encode(pkt_disconnect, pktChar, &len2) != PKT_OK){
                    printf("erreur dans l'encodage du paquet de déconnection\n");
                    return;
                  }
                  if(write(sfd, pktChar, len2)!=12*sizeof(char)){
                     destroy_list(current);
                     printf("Erreur write sfd(s4)\n");
                     return;
                  }
                  chrono_set_time(node_get_chrono(toSend), retransmissionTimer);
                  printf("sender===retransmissionTimer=%ld\n", retransmissionTimer.tv_sec);
                  int r=node_get_data(runner)==NULL;
                  printf("sender===node_get_data(runner)==NULL ===%d\n",r);
                  printf("numero de sequence de toSend: %d\n", pkt_get_seqnum(node_get_data(toSend)));
                  printf("numero de sequence de runner: %d\n", pkt_get_seqnum(node_get_data(runner)));
                  numeroDeSequence++;
                  toSend=node_get_next(toSend);

                }

                else{   	
                	pkt_del(node_get_data(toSend));
                  node_set_data(toSend,pkt_new());//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                  if(create_packet(reader,length,31,numeroDeSequence,0,node_get_data(toSend))!=PKT_OK){

                    printf("====erreure lors du create_packet" );
                    return;
                  }
                  numeroDeSequence++;
                  if(numeroDeSequence==256){
                    numeroDeSequence=0;
                  }


                  char charAEnvoyer[16+length];
                  size_t len=(16+length)*sizeof(char);

                  pkt_status_code codeRetour=pkt_encode(node_get_data(toSend),charAEnvoyer,&len);


                  if(codeRetour!=PKT_OK){
                    printf("====erreure lors de l'encodage du paquet\n");
                  }


                  chrono_set_time(node_get_chrono(toSend), retransmissionTimer); // démarre le chrono
                  if(write(sfd,charAEnvoyer,sizeof(charAEnvoyer))!=(int)sizeof(charAEnvoyer))
                  {
                      destroy_list(current);
                      printf("Erreur write sfd(s5)\n");
                      return;
                  }
                  memset(reader,0,512);
                  memset(charAEnvoyer,0,(16+length));
                  length=0;
                  toSend=node_get_next(toSend);
                }




              }//fin du while
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
                printf("Erreur read socket\n");
                return;
            }

            pkt_t* paquetDecode=pkt_new();
            memcpy(paquetDecode,writer,2);
            if(pkt_get_type(paquetDecode)!=PTYPE_ACK){//le paquet recu n'est pas un ack
            resend(pkt_get_seqnum(paquetDecode),sfd);
            pkt_del(paquetDecode);
            }
            else{//paquest recu est de type ack
                if(premierMessage){//si la lecture était le premier message
                pkt_del(paquetDecode);
                premierMessage=0;
                retransmissionTimer = chrono_get_currentTime(node_get_chrono(current)); //calcul du retransmission timer
                retransmissionTimer.tv_sec = retransmissionTimer.tv_sec+2;
                current=node_get_next(current);
                finWind=node_get_next(finWind);
                wmin++;
                wmax++;




              }
              else{//on cheke si le numéro de séquence recu est celui attendu
                if(pkt_get_seqnum(paquetDecode)==seqnumDeconnection+1){
                  destroy_list(current);
                  pkt_del(paquetDecode);
                
                  return;
                }//fin du if envoi de la demande de deconnection

                if(pkt_get_seqnum(paquetDecode)==wmin){//le seqnum n'est pas celui attendu
                  resend(pkt_get_seqnum(paquetDecode),sfd);

                }

                else{//grace aux acquis cumulatifs, on sait que tous les paquets ont bien été recu
// jusqu'au numéro de séquence recu
                    int* nbrAdecaler=malloc(sizeof(int));
                    difference(wmin,wmax,pkt_get_seqnum(paquetDecode),nbrAdecaler);
                    for(int i=0;i<*nbrAdecaler;i++){



                      current=node_get_next(current);
                      finWind=node_get_next(finWind);
                      wmin++;
                      wmax++;
                      if(wmin==256){
                        wmin=0;
                      }
                      if(wmax==256){
                        wmax=0;
                      }
                    }
                    free(nbrAdecaler);
                }
                pkt_del(paquetDecode);

              }//fin du else qui regarde si le numéro de seq est celui attendu
            }//fin du else qui regarde si le le paquet est de type ack

        }//fin de la lecture du sfd


        pkt_t* renvoi;

        if(premierMessage){
          if(!chrono_is_ok(node_get_chrono(current))){
            renvoi = node_get_data(current);
            resend(pkt_get_seqnum(renvoi), sfd);
          }
        }
        else{

          runner = current;
          for(int i = wmin; i!=wmax; i++){
            if(!chrono_is_ok(node_get_chrono(runner))&& node_get_data(runner)!=NULL){
              if(pkt_get_length(node_get_data(runner))==0){
                countDeconnection++;
              }
              if(countDeconnection==4){

                destroy_list(current);
                return;
              }
              renvoi = node_get_data(runner);
              resend(pkt_get_seqnum(renvoi), sfd); //si le chrono est dépassé le paquet est réenvoyé
            }
            runner = node_get_next(runner);
            if(i == 255){
              i = -1;
            }
          }


          runner = current;
        }


    }//fin du while
}



/*
la fonction ouvre le fichier dans lequel il lit ce qu'il écrira sur le sfd
elle appelle ensuite read_write_loop et ferme le fichier lorsque la lecture est terminée
sfd est le file descriptor du socket et nomFichier le nom du fichier de sortie
*/

void sender2(int sfd, char* nomFichier){
  int fd;
  if(nomFichier==NULL){
    fd=0;
  }
  else{
    fd=open(nomFichier,O_RDONLY);

  }
  if(fd == -1){
		printf( "erreur dans l'ouverture du fichier d'entree\n");
		exit(EXIT_FAILURE); // il faut voir les consignes
	}

  read_write_loop(sfd, fd);
  int f = close(fd);
	if(f == -1){
		printf( "erreur dans la fermeture du fichier d'entrée\n");
	}
	return;

}



int main(int argc, char *argv[]){
  
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
     printf("option inconnue");
     return -1;
   }
 }
 
 hostName=argv[optind];
 port=atoi(argv[optind+1]);
 printf( "numéro de port: %d\n", port);


struct sockaddr_in6 addr;
const char *err=real_address(hostName,&addr);
if(err){
  printf( "Could not resolve hostname %s: %s\n", hostName, err);
  return EXIT_FAILURE;
}

int sfd=create_socket(NULL,-1,&addr,port);/* Connected */
if(sfd<0){
  printf( "Failed to create the socket!\n");
  return EXIT_FAILURE;
}

sender2(sfd,nomFichier);
close(sfd);
return EXIT_SUCCESS;

}
