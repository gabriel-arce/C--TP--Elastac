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

	memcpy(&(unHeader->identificador), unStream->data + offset, tmp_size =
			sizeof(unHeader->identificador));
	offset += tmp_size;

	memcpy(&(unHeader->tamanio), unStream->data + offset, tmp_size =
			sizeof(unHeader->tamanio));
	return unHeader;
}

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

int recibir_handshake(int sock_fd, t_header * handshake_in) {

	void * buffer_in = malloc(5);

	int recibido = recv(sock_fd, buffer_in, 5, MSG_WAITALL);

	if (recibido == -1) {
		printf("Error en el recv.\n");
		return -1;
	}

	memcpy(&handshake_in->identificador, buffer_in, 1);
	memcpy(&handshake_in->tamanio, buffer_in + 1, 4);

	free(buffer_in);

	return recibido;

}
