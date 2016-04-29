/*
 * ProcesarUMC.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <UMC.h>

int main() {
	int socket_cliente, socket_servidor;

	printf("***Proceso UMC***\n");
	new_line();

	cargar_config();

	pthread_create(&hiloConsola, NULL, lanzar_consola, NULL);

	socket_servidor = crearSocket();
	if (socket_servidor == -1)
		exit(EXIT_FAILURE);
	new_line();

	if (bindearSocket(socket_servidor, umc_config->puerto_escucha) == -1)
			exit(EXIT_FAILURE);
	new_line();

	if (escucharEn(socket_servidor) == -1)
			exit(EXIT_FAILURE);
	new_line();



	//Conectar con swap
	if((socket_cliente = clienteDelServidor(umc_config->ip_swap, umc_config->puerto_swap)) == -1)
		exit(EXIT_FAILURE);

	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		if (enviarPorSocket(socket_cliente, mensaje) == -1) {
			perror("No se pudo enviar el mensaje");
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}
