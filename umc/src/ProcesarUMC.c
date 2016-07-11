/*
 * ProcesarUMC.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <UMC.h>

int main(int argc, char * argv[]) {

	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	if (chequear_argumentos(argc, 2))
		return EXIT_FAILURE;

	cargar_archivo_config(argv, (void *) cargar_config);
	imprimir_config();

	socket_nucleo = -1;
	socket_swap = -1;
	socket_servidor = -1;

	printf("***Proceso UMC***\n");
	new_line();

	//----creo estructuras
	//crear_archivo_log();
	//inicializar_memoria();
	//crear_archivo_reporte();

	cpu_conectadas = list_create();
	lista_procesos = list_create();

	tlb_on = tlb_habilitada();

	//inicializo semaforos
	inicializar_semaforos();

	//pthread_create(&hilo_cliente, NULL, conecta_swap, NULL);
	pthread_create(&hilo_server, NULL, escucha_conexiones, NULL);

	//pthread_join(hilo_cliente, NULL);
	//pthread_detach(hilo_cliente, NULL);
	pthread_join(hilo_server, NULL);
	pthread_detach(hilo_server);

	list_destroy(cpu_conectadas);
	//free(memoria_principal);
	free(umc_config);

	return EXIT_SUCCESS;
}
