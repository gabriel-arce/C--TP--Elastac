/*
 * elestac_comunicaciones.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef ELESTAC_COMUNICACIONES_H_
#define ELESTAC_COMUNICACIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      //perror
#include <arpa/inet.h>  //INADDR_ANY
#include <unistd.h>     //close  usleep
#include <netdb.h> 		//gethostbyname
#include <netinet/in.h>
#include <fcntl.h> 		//fcntl
#include <commons/string.h>
#include <commons/log.h>

typedef struct {
	uint8_t identificador;
	uint32_t tamanio;
}__attribute__((packed)) t_header;

void * serializar_header(uint8_t id, uint32_t size);
t_header * deserializar_header(void * buffer);

#endif /* ELESTAC_COMUNICACIONES_H_ */
