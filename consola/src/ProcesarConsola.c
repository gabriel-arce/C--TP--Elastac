/*
 * ProcesarConsola.c
 *
 *  Created on: 27 de abr. de 2016
 *      Author: gabriel
 */

#include "Consola.h"
#include <elestac_sockets.h>


int main(void){

	int socketConsola;									//Descriptor de consola

	//Cargar configuracion
	printf("Inicializando Consola..\n");
	printf("please wait..\n");
	printf("\n");

	cargar_config();

	printf("IP Nucleo: %s\n", consola->ip_nucleo);
	printf("Puerto Nucleo: %d\n", consola->puerto_nucleo);

	//Conectar al nucleo
	if((socketConsola = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1)
		exit(1);

	enviarPorSocket(socketConsola, "Hola! Soy una consola!..");

	return EXIT_SUCCESS;
}



