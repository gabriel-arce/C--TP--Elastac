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

	cpu_conectadas = list_create();
	lista_frames = list_create();
	tabla_de_paginas = list_create();

	if (tlb_habilitada())
		tlb = list_create();

	//inicializo semaforos
	pthread_mutex_init(&mutex_lista_cpu, 0);


	//carga archivo de configuracion y sus parametros
	cargar_config();
	imprimir_config();

	//creo estructuras
	memoria_size = umc_config->frames_size * umc_config->cant_frames;
	memoria_principal = malloc(memoria_size);
	memset(memoria_principal, 0, memoria_size);

	//crea hilo de conexion con swap
	pthread_create(&hilo_cliente, NULL, conecta_swap, NULL);

	//crea hilo de conexion servidor con nucleo y las x cpu
	pthread_create(&hilo_server, NULL, escucha_conexiones, NULL);

	//crea hilo de consola
	pthread_create(&hiloConsola, NULL, lanzar_consola, NULL);

	//joineo los hilos
	pthread_join(hiloConsola, NULL);
	pthread_join(hilo_cliente, NULL);
	pthread_join(hilo_server, NULL);

	pthread_detach(hiloConsola);
	pthread_detach(hilo_cliente);
	pthread_detach(hilo_server);


	list_destroy(cpu_conectadas);
	free(memoria_principal);
	free(umc_config);

	return EXIT_SUCCESS;
}
