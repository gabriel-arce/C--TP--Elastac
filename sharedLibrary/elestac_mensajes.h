/*
 * elestac_mensajes.h
 *
 *  Created on: 10/5/2016
 *      Author: utnso
 */

#ifndef ELESTAC_MENSAJES_H_
#define ELESTAC_MENSAJES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#define ID_CONSOLA 1
#define ID_NUCLEO 2
#define ID_UMC 3
#define ID_SWAP 4
#define ID_CPU 5

typedef struct {
	int8_t identificador;
	int32_t tamanio;
}__attribute__((packed)) t_header;

typedef struct {
	char * data;
	int size;
}__attribute__((packed)) t_stream;


t_stream * serializar_header(t_header * unHeader);
t_header * deserializar_header(t_stream * unStream);

#endif /* ELESTAC_MENSAJES_H_ */
