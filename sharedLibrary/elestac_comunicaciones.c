/*
 * elestac_comunicaciones.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "elestac_comunicaciones.h"

void * serializar_header(uint8_t id, uint32_t size) {
	t_header * header = malloc(sizeof(t_header));
	header->identificador = id;
	header->tamanio = size;

	void * buffer = malloc(5);
	memcpy(buffer, &(header->identificador), 1);
	memcpy(buffer + 1, &(header->tamanio), 4);

	free(header);

	return buffer;
}

t_header * deserializar_header(void * buffer) {
	t_header * header = malloc(sizeof(t_header));

	memcpy(&(header->identificador), buffer, 1);
	memcpy(&(header->tamanio), buffer + 1, 4);

	free(buffer);

	return header;
}
