

/*
Renvoie 1 si le paquet renvoi doit être renvoyé. Renvoie 0 si le paquet doit être ignoré.
data est le char* recu du réseau et non décodé. Renvoi est le paquet de réponse à data. recu
est un paquet vide qui va être rempli pour être lu par la suite.
window est la taille de la fenêtre actuel du receveur. timestamp n'est pas encore défini
si la fonction renvoie 1, le paquet recu est valide et doit être lu.
*/



int reponse (const char* data, pkt_t *renvoi, pkt_t *recu, uint8_t window, uint32_t timestamp){
	int res = 1;
	//pkt_t* recu = pkt_new(); il faut faire cette ligne en dehors de la fonction
	pkt_status_code statRecu = pkt_decode(data, XXXXXXXX, recu);

	if(pkt_get_type != PTYPE_DATA){
		return 0;
	}
	if(pkt_get_tr != 0){
		if(pkt_get_tr == 1){
			statRecu = E_TR;
		}
		else{
			return 0;
		}
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
		if(sequnum >512 11 sequnum<0){//seqnum n'est pas acceptable -> ignoré
			return 0;
		}
		pkt_set_sequnum(renvoi, sequnum);
		return 1;
	}

	if(statRecu == PKT_OK){
		pkt_get_type(renvoi, PTYPE_ACK);
		sequnum = (pkt_get_sequnum(recu)+1)%2;
		pkt_set_sequnum(renvoi, sequnum);
	}
}
