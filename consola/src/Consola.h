/*
 * Consola.h
 *
 *  Created on: 22/4/2016
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define CONFIG_PATH "../consola/src/consola.conf"

typedef struct {
	int puerto_nucleo;
	char* ip_nucleo;
	char* programa_ansisop;
} t_consola_config;

t_consola_config *cargar_config();

#endif /* CONSOLA_H_ */
