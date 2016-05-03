/*
 * ProcesarUMC.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <UMC.h>

int main() {

	printf("***Proceso UMC***\n");
	new_line();

	cargar_config();

	//pthread_create(&hiloConsola, NULL, (void *) lanzar_consola, NULL);
	//pthread_create(&hilo_cliente, NULL, conecta_swap, NULL);
	//pthread_create(&hilo_server, NULL, atiende_server, NULL);



	return EXIT_SUCCESS;
}
