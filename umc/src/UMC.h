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

#define CONFIG_PATH "../umc/src/umc.conf"
#define MSJ_ERROR1 "Error en sintaxis del comando\n"
#define MSJ_ERROR2 "Error en # de variable"
#define PROMPT "#>"
#define LENGTH_MAX_COMANDO 7

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

t_umc_config * umc_config;
pthread_t hiloConsola, hilo_server, hilo_cliente;

void new_line();
void cargar_config();
void lanzar_consola();
void atiende_server();
void conecta_swap();
int no_es_comando();
void modificar_retardo(int ret);
void reporte_estructuras();
void reporte_contenido();
void limpiar_tlb();
void marcar_paginas();

#endif /* UMC_H_ */
