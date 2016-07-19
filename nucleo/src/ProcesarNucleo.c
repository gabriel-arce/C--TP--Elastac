/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include "Nucleo.h"


int main(int argc, char * argv[]) {

	//Chequear argumentos
	if (chequear_argumentos(argc, 2))
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













/*
	t_header * handshake = malloc(sizeof(t_header));
	void * buffer_out = malloc(5);

	handshake->identificador = 2;
	handshake->tamanio = 0;

	memcpy(buffer_out, &handshake->identificador, sizeof(uint8_t));
	memcpy(buffer_out + sizeof(uint8_t) , &handshake->tamanio, sizeof(uint32_t));

	if (send(socketNucleo, buffer_out, 5, 0) == -1) {
		printf("Error en el send\n");
		exit(EXIT_FAILURE);
	}
	printf("Conexion con umc\n");

	free(buffer_out);
	free(handshake);

	void * buffer_in = malloc(5);
	t_header * head_in = malloc(sizeof(t_header));

	if (recv(socketNucleo, buffer_in, 5, 0) == -1) {
		printf("Error en el recv\n");
	}

	memcpy(&head_in->identificador, buffer_in, sizeof(uint8_t));
	memcpy(&head_in->tamanio, buffer_in + sizeof(uint8_t), sizeof(uint32_t));

	if (head_in->identificador == Tamanio_pagina) {
		tamanio_pagina = head_in->tamanio;
		printf("Tamaño de pagina: %d\n", tamanio_pagina);
	}*/

	return EXIT_SUCCESS;
}

