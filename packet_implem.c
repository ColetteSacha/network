#include "packet_interface.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <zlib.h>
//avec l'aide de Sebastien Kalbusch et Martin Willame


struct __attribute__((__packed__)) pkt {
    uint8_t window:5;
    uint8_t tr:1;
    uint8_t type:2;
    uint8_t seqnum;
    uint16_t length;
    uint32_t timestamp;
    uint32_t crc1;
    char* payload;
    uint32_t crc2;
};

/*
uint16_t transformTo16 (char* str){

    uint8_t tab[2];
    tab[0]=(uint8_t) str[0];
    tab[1]=(uint8_t) str[1];
    uint16_t* u;
    u=(uint16_t*) tab;
    uint16_t ret=*u;
    return ret;
}
uint32_t transformTo32 (char* str){
    uint8_t tab[4];
    tab[0]=(uint8_t) str[0];
    tab[1]=(uint8_t) str[1];
    tab[2]=(uint8_t) str[2];
    tab[3]=(uint8_t) str[3];
    uint32_t* u;
    u=(uint32_t*) tab;
    uint32_t ret=*u;
    return ret;

}

char* u16toStr(uint16_t n) {
    uint16_t* np=&n;
    uint8_t* nh=(uint8_t*) np;
    return (char*) nh;
}
char* u32toStr(uint32_t n) {
    uint32_t* np=&n;
    uint8_t* nh=(uint8_t*) np;
    return (char*) nh;
}
*/

pkt_t* pkt_new(){

    pkt_t* ret=(pkt_t *)calloc(1,sizeof(pkt_t));
    if(ret==NULL){
        return NULL;
    }

    return ret;
}

void pkt_del(pkt_t *pkt)
{
    if(pkt!=NULL){
        if(pkt->payload!=NULL){
            free(pkt->payload);
            pkt->payload=NULL;
        }
        free(pkt);
        pkt=NULL;
    }
}



pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt){

    //if(len < 4) return E_NOHEADER;
    //if(len < 12) return E_UNCONSISTENT;

    memcpy(pkt, data, 2);
    if((pkt_get_type(pkt) != PTYPE_DATA)&&(pkt_get_type(pkt) != PTYPE_ACK)&&(pkt_get_type(pkt) != PTYPE_NACK)){
        return E_TYPE;
    }
    uint16_t length;
    memcpy(&length,data+2,2);
    length=ntohs(length);

    pkt_status_code code=pkt_set_length(pkt,length);
    if(code!=PKT_OK)
    {
        return code;
    }
    uint32_t timestamp;
    memcpy(&timestamp,data+4,4);
    code=pkt_set_timestamp(pkt,timestamp);
    if(code!=PKT_OK){
      return code;
    }

    /*if((length != len-4*sizeof(uint32_t))&&(length != len-3*sizeof(uint32_t))){
        return E_LENGTH;
    }
    */
    if((pkt->tr == 1)&&(length!=0)){
        return E_TR;
    }
    uint32_t crcfirst;
    memcpy(&crcfirst,data+8,4);
    crcfirst=ntohl(crcfirst);
    code=pkt_set_crc1(pkt,crcfirst);
    if(code!=PKT_OK){
      return code;
    }
    uint32_t crcfirstToCheke=crc32(0,(const Bytef *) data, 8);
    if(crcfirst!=crcfirstToCheke){
      return E_CRC;
    }



    pkt_set_payload(pkt, data+12, length); //COPIE DU PAYLOAD CAR ERREUR SINON


    if(length!=0){
      uint32_t crcsecond;
      memcpy(&crcsecond,data+12+length,4);
      crcsecond=ntohl(crcsecond);
      code=pkt_set_crc2(pkt,crcsecond);
      uint32_t crcsecondToChecke=crc32(0,(const Bytef *) pkt_get_payload(pkt), length);
      if(crcsecond!=crcsecondToChecke){
        return E_CRC;
      }
    }
    if(len < sizeof(char)*12 + length) return E_UNCONSISTENT;
    return PKT_OK;


}



pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    
    uint16_t length=pkt_get_length(pkt);
    
    length=htons(length);
    memcpy(buf,pkt,2);
    memcpy(buf+2,&length,2);
    length=pkt_get_length(pkt);
    uint32_t timestamp=pkt_get_timestamp(pkt);
    memcpy(buf+4,&timestamp,4);

    uint32_t crcfirst=crc32(0,(const Bytef *) buf, 8*sizeof(char));
    crcfirst=htonl(crcfirst);
    memcpy(buf+8,&crcfirst,4);

    if(pkt_get_payload(pkt)==NULL){
      *len=12;
      return PKT_OK;
    }

    memcpy(buf+12,pkt_get_payload(pkt),length);
  

    uint32_t crcsecond=crc32(0,(const Bytef *) pkt_get_payload(pkt), length);
    crcsecond=htonl(crcsecond);
    memcpy(buf+12+length,&crcsecond,4);
    *len=16+length;
    return PKT_OK;







    /*
    memcpy(buf, pkt, sizeof(*pkt));
    pkt_t* temp=(pkt_t*) buf;
    temp->length=htons(temp->length);

    //calc CRC1
    uint32_t test_crc1=0;
    char crc1buf[8];
    memcuint32_t crc2;

    if(pkt_get_length(pkt)>0){

        crc2=ntohl(pkt_get_crc2(pkt));
        code=pkt_set_crc2(pkt,crc2);
        if(code!=PKT_OK)
            return code;
    }
    uint32_t crc1_test=0;
    crc1_test = crc32(crc1_test, (const Bytef *) data, 8);
    if(crc1_test != pkt_get_crc1(pkt))
        return E_CRC;
    if(length>0)
    {
        uint32_t crc2_test=0;
        crc2_test = crc32(crc2_test, (const Bytef *) data+12, length);
        if(crc2_test != crc2)
            return E_CRC;
    }py(crc1buf,buf,8);
    test_crc1=crc32(test_crc1,(Bytef *) crc1buf, 8);
    temp->crc1=htonl(test_crc1);
    //memcpy(temp->payload, pkt_get_payload(pkt),length); //COPIE DU PAYLOAD CAR ERREUR SI ON NE LE FAIT PAS
    if(length>0)
    {
        uint32_t test_crc2=0;
        test_crc2 = crc32(test_crc2, (Bytef *)(temp->payload), length);
        temp->crc2=htonl(test_crc2);
    }
    buf=(char*) temp;
    return PKT_OK;



    return PKT_OK;

    */

}








ptypes_t pkt_get_type  (const pkt_t*pkt)
{
    return pkt->type;
}

uint8_t  pkt_get_tr(const pkt_t*pkt)
{
    return pkt->tr;
}

uint8_t  pkt_get_window(const pkt_t*pkt)
{
    return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t*pkt)
{
    return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t*pkt)
{
    return pkt->length;
}

uint32_t pkt_get_timestamp   (const pkt_t*pkt)
{
    return pkt->timestamp;
}

uint32_t pkt_get_crc1   (const pkt_t*pkt)
{
    return pkt->crc1;
}

uint32_t pkt_get_crc2   (const pkt_t*pkt)
{
    return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t*pkt)
{
    return pkt->payload;
}


pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
    if(type==PTYPE_DATA){
        pkt->type=1;
        return PKT_OK;
    }
    if(type==PTYPE_ACK){
        pkt->type=2;
        return PKT_OK;
    }
    if(type==PTYPE_NACK){
        pkt->type=3;
        return PKT_OK;
    }
    return E_TYPE;
}
//faire attention si pkt n'est ps compatible avec le type!
pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
    if(tr==0||tr==1){
        pkt->tr=tr;
        return PKT_OK;
    }
    return E_TR;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
    if(window>31){
        return E_WINDOW;
    }
    pkt->window=window;
    return PKT_OK;
}

//attention aux condtions sur les PTYPE
pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
    //if(seqnum>255){
    //    return E_SEQNUM;
    //}
    pkt->seqnum=seqnum;
    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
    if(length>512){
        return E_LENGTH;
    }
    pkt->length=length;
    return PKT_OK;
}
//attention aux return
pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
    if(timestamp>512){
        return E_UNCONSISTENT;
    }
    pkt->timestamp=timestamp;
    return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
    pkt->crc1=crc1;
    return PKT_OK;

}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
    pkt->crc2=crc2;
    return PKT_OK;

}

pkt_status_code pkt_set_payload(pkt_t *pkt,
                                const char *data,
                                const uint16_t length)
{

    if(length>MAX_PAYLOAD_SIZE){//defini ds PACKET_INTERFACE.H
        return E_NOMEM;
    }
    if(pkt->payload!=NULL){
        free(pkt->payload);
        pkt->payload=NULL;
        pkt->length=0;
    }
    if((data==NULL)||(length == 0)){
        return PKT_OK;
    }
    pkt->payload=malloc(length);
    if(pkt->payload==NULL)
    {
        return E_NOMEM;
    }
    memcpy(pkt->payload,data,length);
    pkt->length = length;
    return PKT_OK;

}
