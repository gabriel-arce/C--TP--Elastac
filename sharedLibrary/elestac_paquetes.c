/*
 * elestac_paquetes.c
 *
 *  Created on: 10/5/2016
 *      Author: utnso
 */

#include "elestac_paquetes.h"

t_stream * serializar_header(t_header * unHeader) {
	t_stream * unStream = NULL;
	unStream = malloc(sizeof(t_stream));
	int32_t offset = 0, tmp_size = 0;

	unStream->data = malloc(
			sizeof(unHeader->identificador) + sizeof(unHeader->tamanio));

	memcpy(unStream->data + offset, &unHeader->identificador, tmp_size =
			sizeof(unHeader->identificador));
	offset += tmp_size;
	memcpy(unStream->data + offset, &unHeader->tamanio, tmp_size =
			sizeof(unHeader->tamanio));
	unStream->size = offset + tmp_size;
	return unStream;
}

t_header * deserializar_header(t_stream * unStream) {
	t_header * unHeader = NULL;
	unHeader = malloc(sizeof(t_header));
	int32_t offset = 0, tmp_size = 0;

	memcpy(&unHeader->identificador, unStream->data + offset, tmp_size =
			sizeof(unHeader->identificador));
	offset += tmp_size;

	memcpy(&unHeader->tamanio, unStream->data + offset, tmp_size =
			sizeof(unHeader->tamanio));
	return unHeader;
}
