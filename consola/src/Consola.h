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
#include <elestac_config.h>

#define CONFIG_PATH "consola.conf"
//#define CONFIG_PATH "../consola/src/consola.conf"

typedef struct {
	int puerto_nucleo;
	char * ip_nucleo;
	char * programa_ansisop;
} t_consola_config;

typedef struct {
	char tipoProceso[3];
	char *contenido;
} t_proceso;

typedef enum{
	CantidadArgumentosIncorrecta,
	NoSePudoAbrirIn,
	NoSePudoCrearSocket,
	NoSePudoEnviarSocket,
	OtroError,
} Error;

t_consola_config *consola;
t_proceso *proceso;
FILE *in;

void cargar_config();
void MostrarAyuda();
void MostrarError( Error );

#endif /* CONSOLA_H_ */
