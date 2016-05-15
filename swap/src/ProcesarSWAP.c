/*
 * ProcesarSWAP.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <swap.h>

int main() {
	printf("***Proceso SWAP***\n");
	printf("\n");

	cargar_config();
	imprimir_config();

	int socket_swap = crearSocket();
	int socket_umc = -1;

	int optval = 1;

	setsockopt(socket_swap, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	if (bindearSocket(socket_swap, swap_config->puerto_escucha) == -1)
		exit(EXIT_FAILURE);

	if (listen(socket_swap, 10) == -1) {
		printf("Error en el listen\n");
		exit(EXIT_FAILURE);
	}

	printf("En escucha de conexiones....\n");
	printf("\n");

	if ((socket_umc = aceptarEntrantes(socket_swap)) == -1)
		exit(EXIT_FAILURE);

	void * buffer_in = malloc(5);
	t_header * handshake_in = malloc(sizeof(t_header));

	if (recv(socket_umc, buffer_in, 5, 0) == -1) {
		perror("Error en el recv\n");
		exit(EXIT_FAILURE);
	}

	memcpy(&handshake_in->identificador, buffer_in, sizeof(uint8_t));
	memcpy(&handshake_in->tamanio, buffer_in + sizeof(uint8_t), sizeof(uint32_t));

	if (handshake_in->identificador == ID_UMC) {
		printf("Se conecto UMC\n");
	}

	free(buffer_in);
	free(handshake_in);
	close(socket_swap);

	return EXIT_SUCCESS;
}
