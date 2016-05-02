/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include <elestac_sockets.h>

#include "Nucleo.h"
#define MAX_CLIENTES 10

int main(void) {

/*	// Obtener el directorio actual - TEST
	char *directorio = getcwd(NULL, 0);
	printf("El directorio actual es: %s\n", directorio);
	free(directorio);*/

	//Cargar configuracion
	cargar_conf();

	//Escuchar procesos consolas o cpus
	escuchar_procesos();

	//Mientras haya al menos haya un cpu, planifica..

	//while procesos_cpu > 1

	planificar_procesos();

	return EXIT_SUCCESS;
}

