/*
 * ProcesarUMC.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <UMC.h>

int main() {

	socket_nucleo = -1;
	socket_cliente = -1;
	socket_servidor = -1;

	printf("***Proceso UMC***\n");
	new_line();

	//carga archivo de configuracion y sus parametros
	cargar_config();
	imprimir_config();

	//creo estructuras
	//crear_archivo_log();
	//inicializar_memoria();
	//crear_archivo_reporte();

	cpu_conectadas = list_create();
	marcos_memoria = list_create();
	tabla_de_paginas = list_create();

	if (tlb_habilitada())
		tlb = list_create();

	//inicializo semaforos
	pthread_mutex_init(&mutex_servidor, 0);
	pthread_mutex_init(&mutex_hilos, 0);
	pthread_mutex_init(&mutex_lista_cpu, 0);
	pthread_mutex_init(&mutex_nucleo, 0);
	pthread_mutex_init(&mutex_memoria, 0);

	//pthread_create(&hilo_cliente, NULL, conecta_swap,NULL);
	//pthread_create(&hiloConsola, NULL, lanzar_consola, NULL);
	pthread_create(&hilo_server, NULL, escucha_conexiones, NULL);

	//pthread_join(hilo_cliente, NULL);
	//pthread_detach(hilo_cliente, NULL);
	//pthread_join(hiloConsola, NULL);
	//pthread_detach(hiloConsola);
	pthread_join(hilo_server, NULL);
	pthread_detach(hilo_server);

	list_destroy(cpu_conectadas);
	//free(memoria_principal);
	free(umc_config);

	return EXIT_SUCCESS;
}
