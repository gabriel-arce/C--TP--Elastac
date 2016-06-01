/*
 * elestac_paquetes.c
 *
 *  Created on: 10/5/2016
 *      Author: utnso
 */

#include "elestac_paquetes.h"

int enviar_handshake(int sock_fd, uint8_t id_proc, uint32_t size) {

	t_header * handshake = malloc(sizeof(t_header));
	handshake->identificador = id_proc;
	handshake->tamanio = size;
	void * buffer_out = malloc(5);

	memcpy(buffer_out, &handshake->identificador, 1);
	memcpy(buffer_out + 1, &handshake->tamanio, 4);

	send(sock_fd, buffer_out, 5, 0);

	int resultado_send = send(sock_fd, buffer_out, 5, 0);

	free(handshake);
	free(buffer_out);

	return resultado_send;
}

t_header * recibir_handshake(int sock_fd, t_header * handshake_in) {

	void * buffer_in = malloc(5);

	int recibido = recv(sock_fd, buffer_in, 5, MSG_WAITALL);

	if (recibido == -1) {
		printf("Error en el recv.\n");
		return NULL;
	}

	memcpy(&handshake_in->identificador, buffer_in, 1);
	memcpy(&handshake_in->tamanio, buffer_in + 1, 4);

	free(buffer_in);

	return handshake_in;

}

void * serializar_header(t_header * header) {
	void * buffer = malloc(5);

	memcpy(buffer, &header->identificador, 1);
	memcpy(buffer + 1, &header->tamanio, 4);

	return buffer;
}

t_header * deserializar_header(void * buffer) {
	t_header * header = malloc(sizeof(t_header));

	memcpy(&header->identificador, buffer, 1);
	memcpy(&header->tamanio, buffer + 1, 4);

	return header;
}




