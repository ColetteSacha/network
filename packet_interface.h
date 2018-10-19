	
	/*
	 * Encode une struct pkt dans un buffer, prÃªt a Ãªtre envoye sur le reseau
	 * (c-a-d en network byte-order), incluant le CRC32 du header et
	 * eventuellement le CRC32 du payload si celui-ci est non nul.
	 *
	 * @pkt: La structure a encoder
	 * @buf: Le buffer dans lequel la structure sera encodee
	 * @len: La taille disponible dans le buffer
	 * @len-POST: Le nombre de d'octets ecrit dans le buffer
	 * @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
	 *         le buffer est trop petit.
	 */
	pkt_status_code pkt_encode(const pkt_t*, char *buf, size_t *len);

	/* Accesseurs pour les champs toujours presents du paquet.
	 * Les valeurs renvoyees sont toutes dans l'endianness native
	 * de la machine!
	 */
	ptypes_t pkt_get_type     (const pkt_t*);
	uint8_t  pkt_get_tr       (const pkt_t*);
	uint8_t  pkt_get_window   (const pkt_t*);
	uint8_t  pkt_get_seqnum   (const pkt_t*);
	uint16_t pkt_get_length   (const pkt_t*);
	uint32_t pkt_get_timestamp(const pkt_t*);
	uint32_t pkt_get_crc1     (const pkt_t*);
	/* Renvoie un pointeur vers le payload du paquet, ou NULL s'il n'y
	 * en a pas.
	 */
	const char* pkt_get_payload(const pkt_t*);
	/* Renvoie le CRC2 dans l'endianness native de la machine. Si
	 * ce field n'est pas present, retourne 0.
	 */
	uint32_t pkt_get_crc2(const pkt_t*);

	/* Setters pour les champs obligatoires du paquet. Si les valeurs
	 * fournies ne sont pas dans les limites acceptables, les fonctions
	 * doivent renvoyer un code d'erreur adapte.
	 * Les valeurs fournies sont dans l'endianness native de la machine!
	 */
	pkt_status_code pkt_set_type     (pkt_t*, const ptypes_t type);
	pkt_status_code pkt_set_tr       (pkt_t*, const uint8_t tr);
	pkt_status_code pkt_set_window   (pkt_t*, const uint8_t window);
	pkt_status_code pkt_set_seqnum   (pkt_t*, const uint8_t seqnum);
	pkt_status_code pkt_set_length   (pkt_t*, const uint16_t length);
	pkt_status_code pkt_set_timestamp(pkt_t*, const uint32_t timestamp);
	pkt_status_code pkt_set_crc1     (pkt_t*, const uint32_t crc1);
	/* Defini la valeur du champs payload du paquet.
	 * @data: Une succession d'octets representants le payload
	 * @length: Le nombre d'octets composant le payload
	 * @POST: pkt_get_length(pkt) == length */
	pkt_status_code pkt_set_payload(pkt_t*,
		                        const char *data,
		                        const uint16_t length);
	/* Setter pour CRC2. Les valeurs fournies sont dans l'endianness
	 * native de la machine!
	 */
	pkt_status_code pkt_set_crc2(pkt_t*, const uint32_t crc2);

	#endif  /* __PACKET_INTERFACE_H_ */
