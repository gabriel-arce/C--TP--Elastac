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
#include <elestac_comunicaciones.h>
#include <pthread.h>
#include <signal.h>

//#define CONFIG_PATH "consola.conf"
#define CONFIG_PATH "../consola/src/consola.conf"
#define Iniciar_ansisop 10
#define Imprimir_valor 11
#define Imprimir_texto 12
#define Fin_ansisop 13
#define ABORTAR_PROGRAMA 33

typedef struct {
	int puerto_nucleo;
	char * ip_nucleo;
	char * programa_ansisop;
} t_consola_config;

typedef enum{
	CantidadArgumentosIncorrecta,
	NoSePudoAbrirIn,
	NoSePudoCrearSocket,
	NoSePudoEnviarSocket,
	OtroError,
} Error;

t_consola_config *consola;
pthread_mutex_t mutex_nucleo;
int socketConsola;

void cargar_config(char ** config_path);
void MostrarAyuda();
void MostrarError( Error );
void rutina(int n);

#endif /* CONSOLA_H_ */
