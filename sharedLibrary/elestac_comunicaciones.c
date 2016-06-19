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

void * serializar_iniciar_prog(int pid, int paginas, char * codigo) {

	int n = 0;
	n = 12 + strlen(codigo);

	void * buffer = malloc(n);

	t_paquete_inicializar_programa * paquete = malloc(sizeof(t_paquete_inicializar_programa));
	paquete->pid = pid;
	paquete->paginas_requeridas = paginas;
	paquete->programa_length = strlen(codigo);

	memcpy(buffer, &(paquete->pid), 4);
	memcpy(buffer + 4, &(paquete->paginas_requeridas), 4);
	memcpy(buffer + 8, &(paquete->programa_length), 4);
	memcpy(buffer + 12, codigo, strlen(codigo));

	free(paquete);

	return buffer;
}

t_paquete_inicializar_programa * deserializar_iniciar_prog(void * buffer) {
	t_paquete_inicializar_programa * paquete = malloc(sizeof(t_paquete_inicializar_programa));

	memcpy(&(paquete->pid), buffer, 4);
	memcpy(&(paquete->paginas_requeridas), buffer + 4, 4);
	memcpy(&(paquete->programa_length), buffer + 8, 4);
	paquete->codigo_programa = malloc(paquete->programa_length);
	memcpy(paquete->codigo_programa, buffer + 12, paquete->programa_length);

	free(buffer);

	return paquete;
}

void * serializar_leer_pagina(int pagina, int offset, int bytes) {
	t_paquete_solicitar_pagina * paquete = malloc(sizeof(t_paquete_solicitar_pagina));
	void * buffer = malloc(12);

	paquete->nro_pagina = pagina;
	paquete->offset = offset;
	paquete->bytes = bytes;

	memcpy(buffer, &(paquete->nro_pagina), 4);
	memcpy(buffer + 4, &(paquete->offset), 4);
	memcpy(buffer + 8, &(paquete->bytes), 4);

	free(paquete);
	return buffer;
}

t_paquete_solicitar_pagina * deserializar_leer_pagina(void * buffer) {
	t_paquete_solicitar_pagina * paquete = malloc(sizeof(t_paquete_solicitar_pagina));

	memcpy(&(paquete->nro_pagina), buffer, 4);
	memcpy(&(paquete->offset), buffer + 4, 4);
	memcpy(&(paquete->bytes), buffer + 8, 4);

	free(buffer);
	return paquete;
}

void * serializar_almacenar_pagina(int pagina, int offset, int bytes, void * buffer) {
	t_paquete_almacenar_pagina * paquete = malloc(sizeof(t_paquete_almacenar_pagina));
	void * ret_buffer = malloc(12 + bytes);

	paquete->nro_pagina = pagina;
	paquete->offset = offset;
	paquete->bytes = bytes;

	memcpy(ret_buffer, &(paquete->nro_pagina), 4);
	memcpy(ret_buffer + 4, &(paquete->offset), 4);
	memcpy(ret_buffer + 8, &(paquete->bytes), 4);
	memcpy(ret_buffer + 12, buffer, bytes);

	free(paquete);
	return ret_buffer;
}

t_paquete_almacenar_pagina * deserializar_almacenar_pagina(void * buffer) {
	t_paquete_almacenar_pagina * paquete = malloc(sizeof(t_paquete_almacenar_pagina));

	memcpy(&(paquete->nro_pagina), buffer, 4);
	memcpy(&(paquete->offset), buffer + 4, 4);
	memcpy(&(paquete->bytes), buffer + 8, 4);
	paquete->buffer = malloc(paquete->bytes);
	memcpy(paquete->buffer, buffer + 12, paquete->bytes);

	free(buffer);
	return paquete;
}

void * serializar_fin_prog(int pid) {
	return serializar_header((uint8_t) 37, (uint32_t) pid);
}

t_header * deserializar_fin_prog(void * buffer) {
	return deserializar_header(buffer);
}
