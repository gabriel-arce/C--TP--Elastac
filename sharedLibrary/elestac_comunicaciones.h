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

typedef struct {
	uint32_t programa_length;
	char * codigo_programa;
} t_paquete_programa;

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

void * serializar_ansisop(char * programa);
t_paquete_programa * deserializar_ansisop(void * buffer);

void * serializar_imprimir_texto(char * texto);
char * deserializar_imprimir_texto(void * buffer);

void * serializar_imprimir_valor(int valor);
int deserializar_imprimir_valor(void * buffer);

void * serializar_identificador_semaforo(char* id);
char * deserializar_identificador_semaforo(void * buffer);

int enviar_handshake(int socket, int id);
int recibir_handshake(int socket);

int enviar_header(int id, int tamanio, int socket);
t_header * recibir_header(int socket);

int enviar_solicitud_lectura(int pagina, int offset, int bytes, int socket);
t_paquete_solicitar_pagina * recibir_solicitud_lectura(int bytes_to_recv, int socket);

int enviar_solicitud_escritura(int pagina, int offset, int bytes, void * buffer, int socket);
t_paquete_almacenar_pagina * recibir_solicitud_escritura(int bytes_to_recv, int socket);

//respuesta: si -> 1  / no -> 0
int enviar_respuesta_inicio(int socket, int respuesta);
int recibir_respuesta_inicio(int socket);

int enviar_inicializar_programa(int pid, int paginas, char * codigo, int socket);
t_paquete_inicializar_programa * recibir_inicializar_programa(int bytes_a_recibir, int socket);

int enviar_texto(char * texto, int socket);
char* recibir_texto(int buffer_size, int socket);

int enviar_valor_de_variable(uint32_t valor, int socket);
int recibir_valor_de_variable(int socket);

int enviar_signal_identificador_semaforo(char* identificador_semaforo, int socket);


int enviar_wait_identificador_semaforo(char* identificador_semaforo, int socket);


#endif /* ELESTAC_COMUNICACIONES_H_ */
