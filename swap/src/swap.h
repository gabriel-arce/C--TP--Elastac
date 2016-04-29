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
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <elestac_config.h>
#include <elestac_sockets.h>
#include <elestac_semaforos.h>

#define CONFIG_PATH "../swap/src/swap.conf"

typedef struct {
	int puerto_escucha;
	char * espacio_swap;
	int cant_paginas;
	int pagina_size;
	int retardo_compactacion;
} t_swap_config;

t_swap_config * swap_config;

void cargar_config();


#endif /* SWAP_H_ */
