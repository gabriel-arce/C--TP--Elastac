/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include "Nucleo.h"


int main(void) {

/*	// Obtener el directorio actual - TEST
	char *directorio = getcwd(NULL, 0);
	printf("El directorio actual es: %s\n", directorio);
	free(directorio);*/
	pid_global = 0;
	//Cargar configuracion
	cargarConfiguracion();

	//Crear las listas
	crearListasYColas();

	//Crear cliente para UMC
	crearClienteUMC();

	//Crear semaforos
	crearSemaforos();

	//Crear servidor de consolas
	pthread_create(&pIDServerConsola, NULL, (void *)crearServerConsola, NULL);

	//Crear servidor de cpus
	pthread_create(&pIDServerCPU, NULL, (void *)crearServerCPU, NULL);

	//Planificar consolas
	pthread_create(&pIDPlanificador, NULL, (void *)planificar_consolas, NULL);

	pthread_join(pIDServerConsola, NULL);
	pthread_join(pIDServerCPU, NULL);
	pthread_join(pIDPlanificador, NULL);

	//Liberar
	pthread_detach(pIDServerConsola);
	pthread_detach(pIDServerCPU);
	pthread_detach(pIDPlanificador);

	//Destruir semaforos
	destruirSemaforos();
}

