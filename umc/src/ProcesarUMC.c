/*
 * ProcesarUMC.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <UMC.h>

int main() {
	int socket_umc;

	printf("***Proceso UMC***\n");
	printf("\n");

	//cargar_config();
	lanzar_consola();

	//Conectar con swap
	if((socket_umc = clienteDelServidor(umc_config->ip_swap, umc_config->puerto_swap)) == -1)
		exit(EXIT_FAILURE);

	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		if (enviarPorSocket(socket_umc, mensaje) == -1) {
			perror("No se pudo enviar el mensaje");
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}
