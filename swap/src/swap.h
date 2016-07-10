/*
 * swap.h
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <elestac_config.h>
#include <elestac_sockets.h>
#include <elestac_semaforos.h>
#include <elestac_comunicaciones.h>
#include <pthread.h>

#define CONFIG_PATH "swap.conf"
//#define CONFIG_PATH "../swap/src/swap.conf"
#define Respuesta_inicio_SI 1
#define Respuesta_inicio_NO 0

static const int EXIT_ERROR = -1;

//~~~Interfaces~~~
#define ID_UMC 3
#define Inicializar_programa 14
#define Respuesta_inicio 17
#define Finalizar_programa 13
#define Solicitar_pagina 15
#define Almacenar_pagina 16
//~~~__________~~~

typedef struct {
	int puerto_escucha;
	char * espacio_swap;
	int cant_paginas;
	int pagina_size;
	int retardo_compactacion;
} t_swap_config;

t_swap_config * swap_config;
int socket_SWAP;
int socket_UMC;
pthread_mutex_t mutex_umc_recv;

void cargar_config(char ** config_path);
void imprimir_config();
void iniciar_servidor_UMC();
void escuchar_solicitudes_UMC();
void inicializar_semaforos();

//*****OPERACIONES CON UMC*****
int inicializar_programa(int buffer_init_size);
int finalizar_programa(int pid);
int leer_pagina(int buffer_read_size);
int almacenar_pagina(int buffer_write_size);
//*****-------------------*****

#endif /* SWAP_H_ */
