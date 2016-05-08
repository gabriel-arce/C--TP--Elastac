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

	char * buffer_in = malloc(1000);

	int bytes_recibidos = recv(socket_umc, buffer_in, 1000, 0);

	if (bytes_recibidos == -1) {
		perror("Se desconecto UMC\n");
		exit(EXIT_FAILURE);
	}

	int id_umc = atoi(buffer_in);

	if (id_umc == ID_UMC) {
		printf("Se conecto UMC\n");
	}

	free(buffer_in);

	close(socket_swap);

	return EXIT_SUCCESS;
}
