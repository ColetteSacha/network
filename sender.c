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





    int resend(int numseq,int sfd){
      printf("===resend\n" );

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
     printf("===resend fin\n" );

     return 1;



      }
      else{
        char charAEnvoyer[(int)pkt_get_length(paquetAEnvoyer)+16];
        size_t l=((int)pkt_get_length(paquetAEnvoyer)+16)*sizeof(char);


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



       if(write(sfd,charAEnvoyer,l)!=(int)l)
       {
          destroy_list(current);
           printf("ERROR: %s\n", strerror(errno));
           printf("Erreur write sfd(s2)\n");
           return 0;
       }
     memset(charAEnvoyer,0,528);
     printf("===resend fin\n" );

     return 1;
      }



   }





void read_write_loop(int sfd,int fdEntree) {
    int ret=-1;
    current=create_empty_list(62);
    toSend=current;
    finWind=find_node(current,0,31,31);
    premierTimer.tv_sec = 5;
    premierTimer.tv_usec = 0;
    int deconnection=0;
    int countDeconnection=0;
    //int finLecture = 0;
    int seqnumDeconnection=-1;

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
            printf("lecture premier message : bytes lu = %d\n", length);
            if(length==0)
            {
              //fin du fichier

              printf("======== erreur ligne162" );
              return;
            }
            //totalLengthr+=length;

            //todo: METTRE LE CHAR DS UN pkt(node_get_data(current[0]))
            pkt_t *un=pkt_new();
            printf("===l70\n" );


            pkt_status_code status= create_packet(reader,length,31,numeroDeSequence,0,un);
            //printf("length=%d\n",length );

            if(status!=PKT_OK){

              printf("====erreure lors du create_packet\n" );


              return;
            }
            node_set_data(toSend,un);


            numeroDeSequence++;


            char charAEnvoyer[length+16];
            size_t l=(length+16)*sizeof(char);

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
            memset(charAEnvoyer,0,528);
            length=0;
            toSend=node_get_next(toSend);

          }



          else{
            //printf("l198 debut du else\n");
            if(!premierMessage){


              //pkt_t* chekeWindow=node_get_data(toSend);//!!!!

              while(toSend!=finWind && !deconnection){

                int length=read(fdEntree,reader,512);
                printf("lecture : bytes lu = %d\n", length);
                printf("deconnection = %d\n", deconnection);

                if(length>0){
                  node_set_data(toSend,pkt_new());//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                  if(create_packet(reader,length,31,numeroDeSequence,0,node_get_data(toSend))!=PKT_OK){

                    printf("====erreure lors du create_packet" );
                    return;
                  }
                  numeroDeSequence++;
                  if(numeroDeSequence==256){
                    numeroDeSequence=0;
                  }


                  char charAEnvoyer[length+16];
                  size_t len=(length+16)*sizeof(char);

                  pkt_status_code codeRetour=pkt_encode(node_get_data(toSend),charAEnvoyer,&len);
                  //!!!!!!!!!!!!!pas paquet AEnvoyer ms packet ds current!!!!!!!!!!!!!!!!!!!


                  if(codeRetour!=PKT_OK){
                    printf("====erreure lors de l'encodage du paquet\n");
                  }


                  chrono_set_time(node_get_chrono(toSend), retransmissionTimer); // démarre le chrono

                  if(write(sfd,charAEnvoyer,len)!=(int)len)
                  {
                      destroy_list(current);
                      printf("Erreur write sfd(s5)\n");
                      return;
                  }
                  //totalLengthwSfd+=length;

                  memset(reader,0,512);
                  memset(charAEnvoyer,0,528);
                  //length=0;
                  toSend=node_get_next(toSend);
                }

                if(length<512)
                {
                  //fin du fichier
                  printf("fin du fichier - sender\n");
                  deconnection=1;
                  //finLecture = 1;
                  seqnumDeconnection = numeroDeSequence;
                  printf("seqnumDeconnection=%d\n", seqnumDeconnection);
                  pkt_t* pkt_disconnect = pkt_new();
                  if(create_packet(NULL, 0, 1, numeroDeSequence, 0, pkt_disconnect) != PKT_OK){
                      printf("erreur dans la création du paquet de déconnection\n");
                      return;
                  }//vérifier le numéro de séquencd
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
              }//fin du while
          }
        }//fin de l'envoi ds la possibilité de la window
      }//fin du fait qu'on a la possibilité de lecture du stdin ou fichier
        //reste todo




        if (fds[1].revents & POLLIN){
            int length=read(sfd, writer, 12);
            if(length<=0)
            {
              printf("fin de la lecture l274 - sender\n");
                destroy_list(current);
                fprintf(stderr,"ERROR: %s\n", strerror(errno));
                printf("Erreur read socket\n");
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
            printf("le paquet recu n'est ps un ACK donc resend\n" );
            resend(pkt_get_seqnum(paquetDecode),sfd);
            pkt_del(paquetDecode);
            }
            else{//paquest recu est de type ack. Acheke:numéro de séquence
              printf("sender: recoit un ACK seqnum = %d\n", pkt_get_seqnum(paquetDecode));
              printf("sender: sequnumDeconnection = %d\n", seqnumDeconnection);
              if(premierMessage){//si la lecture était le premier message
                pkt_del(paquetDecode);
                pkt_del(node_get_data(current));
                premierMessage=0;
                printf("premier message recu\n" );
                retransmissionTimer = chrono_get_currentTime(node_get_chrono(current)); //calcul du retransmission timer
                printf("sender===retransmissionTimer sans l'addition=%ld\n", retransmissionTimer.tv_sec);
                retransmissionTimer.tv_sec = retransmissionTimer.tv_sec+2;
                current=node_get_next(current);
                finWind=node_get_next(finWind);
                wmin++;
                wmax++;
                printf("fin de la reception du premier message l416\n");




              }
              else{//on cheke si le numéro de séquence recu est celui attendu
                if(pkt_get_seqnum(paquetDecode)==seqnumDeconnection+1){
                  printf("sortir de la boucle sender\n" );
                  pkt_del(paquetDecode);
                  destroy_list(current);
                  return;
                }//fin du if envoi de la demande de deconnection

                if(pkt_get_seqnum(paquetDecode)==wmin){//le seqnum n'est pas celui attendu
                  printf("seqnum n'est pas celui attendu donc rese,d\n" );
                  resend(pkt_get_seqnum(paquetDecode),sfd);

                }

                if(pkt_get_seqnum(paquetDecode) == seqnumDeconnection+1){//la demande de deconnection a été recue
                  destroy_list(current);
                  pkt_del(paquetDecode);
                  return;
                }

                else{//grace aux acquis cumulatifs, on sait que tous les paquets ont bien été recu
// jusqu'au numéro de séquence recu
                    int* nbrAdecaler=malloc(sizeof(int));
                    difference(wmin,wmax,pkt_get_seqnum(paquetDecode),nbrAdecaler);
                    for(int i=0;i<*nbrAdecaler;i++){


                      pkt_del(node_get_data(current));
                      current=node_get_next(current);
                      finWind=node_get_next(finWind);
                      wmin++;
                      wmax++;
                      if(wmin==255){
                        wmin=0;
                      }
                      if(wmax==255){
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
            printf("chrono du premier foire donc resend\n" );
            resend(pkt_get_seqnum(renvoi), sfd);
          }
        }
        else{

          runner = current;
          for(int i = wmin; i!=wmax; i++){
            if(!chrono_is_ok(node_get_chrono(runner))&& node_get_data(runner)!=NULL){
              printf("sender===countDeconnection=%d\n",countDeconnection );
              if(pkt_get_length(node_get_data(runner))==0){
                countDeconnection++;
              }
              if(countDeconnection==4){

                destroy_list(current);
                return;
              }
              renvoi = node_get_data(runner);
              printf("===renvoi du à la clock\n" );
              printf("===seqnum de renvoi=%d\n", pkt_get_seqnum(renvoi));
              resend(pkt_get_seqnum(renvoi), sfd); //si le chrono est dépassé le paquet est réenvoyé
              printf("===renvoi sender l362\n" );
            }
            runner = node_get_next(runner);
            if(i == 256){
              i = 0;
            }
          }


          runner = current;
        }


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
		printf( "erreur dans l'ouverture du fichier d'entree\n");
		exit(EXIT_FAILURE); // il faut voir les consignes
	}
  printf( "debut du read_write_loop\n" );

  read_write_loop(sfd, fd);
  int f = close(fd);
	if(f == -1){
		printf( "erreur dans la fermeture du fichier d'entrée\n");
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
     printf("option inconnue");
     return -1;
   }
 }
 printf("ligne425:ok\n" );

 hostName=argv[optind];
 port=atoi(argv[optind+1]);
 printf( "numéro de port: %d\n", port);


struct sockaddr_in6 addr;
const char *err=real_address(hostName,&addr);
if(err){
  printf( "Could not resolve hostname %s: %s\n", hostName, err);
  return EXIT_FAILURE;
}

printf("ligne439:ok\n" );


int sfd=create_socket(NULL,-1,&addr,port);/* Connected */
if(sfd<0){
  printf( "Failed to create the socket!\n");
  return EXIT_FAILURE;
}
printf("ligne447:ok\n" );

printf( "debut du sender\n" );
sender(sfd,nomFichier);
close(sfd);
return EXIT_SUCCESS;

}
