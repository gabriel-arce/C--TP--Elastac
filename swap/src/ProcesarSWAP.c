/*
 * ProcesarSWAP.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <swap.h>

int main(void) {

	printf("***Proceso SWAP***\n");
	printf("\n");

	cargar_config();

	int socket_swap = crearSocket();

	if (bindearSocket(socket_swap, swap_config->puerto_escucha) == -1)
		exit(EXIT_FAILURE);

	if (escucharEn(swap_config->puerto_escucha) == -1)
		exit(EXIT_FAILURE);

	printf("En escucha de conexiones....\n");
	printf("\n");

	if (aceptarEntrantes(socket_swap) == -1)
		exit(EXIT_FAILURE);

	char * buffer_in = malloc(1000);
	char * paquete = malloc(1000);
	paquete = "hola, soy swap";

	if (enviarPorSocket(socket_swap, paquete) == -1) {
		perror("No se pudo mandar el paquete");
		exit(EXIT_FAILURE);
	}

	while (1) {
		int bytes_recibidos = recv(socket_swap, buffer_in, 1000, 0);

		if (bytes_recibidos == -1) {
			perror("Se desconecto UMC");
			exit(EXIT_FAILURE);
		}

		buffer_in[bytes_recibidos] = "\0";
		printf("%s", buffer_in);
	}

	free(buffer_in);
	free(paquete);

	close(socket_swap);
	return EXIT_SUCCESS;
}
