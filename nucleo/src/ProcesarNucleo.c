/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include "Nucleo.h"


int main(int argc, char * argv[]) {

	//Chequear argumentos
	if ((chequear_argumentos(argc, 2)) == -1)
		return EXIT_FAILURE;

	//Cargar configuracion
	cargar_archivo_config(argv, (void *) cargarConfiguracion);

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

