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

enum interfaces {
	Solicitar_pagina = 35,
	Almacenar_pagina = 36,
} t_interfaz;

typedef struct {
	uint8_t identificador;
	uint32_t tamanio;
}__attribute__((packed)) t_header;

typedef struct {
	int pid;
	int paginas_requeridas;
	int programa_length;
	char * codigo_programa;
} t_paquete_inicializar_programa;

typedef struct {
	int nro_pagina;
	int offset;
	int bytes;
} t_paquete_solicitar_pagina;

typedef struct {
	int nro_pagina;
	int offset;
	int bytes;
	void * buffer;
} t_paquete_almacenar_pagina;

void * serializar_header(uint8_t id, uint32_t size);
t_header * deserializar_header(void * buffer);

void * serializar_iniciar_prog(int pid, int paginas, char * codigo);
t_paquete_inicializar_programa * deserializar_iniciar_prog(void * buffer);

void * serializar_leer_pagina(int pagina, int offset, int bytes);
t_paquete_solicitar_pagina * deserializar_leer_pagina(void * buffer);

void * serializar_almacenar_pagina(int pagina, int offset, int bytes, void * buffer);
t_paquete_almacenar_pagina * deserializar_almacenar_pagina(void * buffer);

void * serializar_fin_prog(int pid);
int deserializar_fin_prog(void * buffer);

void * serializar_cambio_proceso(int pid);
int deserializar_cambio_proceso(void * buffer);

void * serializar_tamanio_pagina(int page_size);
int deserializar_tamanio_pagina(void * buffer);

void * serializar_respuesta_inicio(int response);
int deserializar_respuesta_inicio(void * buffer);

#endif /* ELESTAC_COMUNICACIONES_H_ */
