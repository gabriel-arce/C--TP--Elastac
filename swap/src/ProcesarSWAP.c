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

	if (!chequear_argumentos(argc, 2))
		return EXIT_FAILURE;

	cargar_archivo_config(argv, (void *) cargar_config);
	imprimir_config();

	inicializar_semaforos();
	//inicializar_espcacio_swap

	iniciar_servidor_UMC();
	escuchar_solicitudes_UMC();

	//release de estructuras y memoria
	free(swap_config);

	return EXIT_SUCCESS;
}
