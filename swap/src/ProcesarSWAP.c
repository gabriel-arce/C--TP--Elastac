/*
 * ProcesarSWAP.c
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#include <swap.h>

int main(int argc, char ** argv) {
	printf("***Proceso SWAP***\n");
	printf("\n");

	signal(SIGUSR1, signal_handler);

	if ((chequear_argumentos(argc, 2)) == -1)
		return EXIT_FAILURE;

	cargar_archivo_config(argv, (void *) cargar_config);
	imprimir_config();

	inicializar_espacio_swap();
	inicializar_bitmap();

	inicializar_semaforos();

	iniciar_servidor_UMC();
	escuchar_solicitudes_UMC();

	//release de estructuras y memoria
	free(swap_config);

	return EXIT_SUCCESS;
}
