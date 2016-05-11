/*
 * elestac_paquetes.h
 *
 *  Created on: 10/5/2016
 *      Author: utnso
 */

#ifndef ELESTAC_PAQUETES_H_
#define ELESTAC_PAQUETES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

typedef enum {
	CONSOLA,
	NUCLEO,
	UMC,
	SWAP,
	CPU
} t_PID;

typedef struct {
	t_PID identificador;
	uint32_t tamanio;
}__attribute__((packed)) t_header;

typedef struct {
	char * data;
	int size;
}__attribute__((packed)) t_stream;


t_stream * serializar_header(t_header * unHeader);
t_header * deserializar_header(t_stream * unStream);


#endif /* ELESTAC_PAQUETES_H_ */
