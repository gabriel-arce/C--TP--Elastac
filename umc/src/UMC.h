/*
 * UMC.h
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#ifndef UMC_H_
#define UMC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <elestac_config.h>
#include <elestac_sockets.h>
#include <elestac_semaforos.h>

//#define CONFIG_PATH "umc.conf" //para runear en terminal
#define CONFIG_PATH "../umc/src/umc.conf"  //para runear en eclipse
#define MSJ_ERROR1 "Error en sintaxis del comando\n"
#define MSJ_ERROR2 "Error en # de variable"
#define PROMPT "#>"
#define LENGTH_MAX_COMANDO 7
#define MENSAJE_HANDSHAKE "Hola soy umc"
#define ID_NUCLEO 2
#define ID_UMC 3
#define ID_SWAP 4
#define ID_CPU 5

typedef struct {
	int puerto_escucha;
	int puerto_swap;
	char * ip_swap;
	int cant_frames;
	int frames_size;
	int frame_x_prog;
	int entradas_tlb;
	int retardo;
} t_umc_config;

typedef struct {
	int identificador;
	char * mensaje;
} t_handshake;

typedef struct {
	int socket_nucleo;
} t_sesion_nucleo;

typedef struct {
	int socket_cpu;
	int id_cpu;
} t_sesion_cpu;

t_umc_config * umc_config;
int socket_cliente, socket_servidor;
pthread_t hiloConsola, hilo_server, hilo_cliente;
int id_cpu;
t_list * cpu_conectadas;
pthread_mutex_t mutex_lista_cpu;
unsigned long memoria_size;
char * memoria_principal;
t_list * tlb;
t_list * lista_frames;
t_list * tabla_de_paginas;

void new_line();
void cargar_config();
void imprimir_config();
int tlb_habilitada();
void * lanzar_consola();
void * escucha_conexiones();
void * conecta_swap();
int no_es_comando();
void modificar_retardo(int ret);
void reporte_estructuras();
void reporte_contenido();
void limpiar_tlb();
void marcar_paginas();

#endif /* UMC_H_ */
