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

	//Cargar configuracion
	cargar_conf();

	//Escuchar procesos consolas o cpus
	escuchar_procesos();

	//Mientras haya al menos haya un cpu, planifica..

	//while procesos_cpu > 1

	//planificar_procesos();

	//Crear socket al nucleo
	if((socketNucleo = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc)) == -1){
//		MostrarMensajeDeError(ETIQUETA_NUCLEO, NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	t_header * handshake = malloc(sizeof(t_header));
	handshake->identificador = NUCLEO;

	if (send(socketNucleo, handshake, sizeof(t_header), 0)
			== -1) {
		printf("No se pudo enviar el handshake a UMC.\n");
		return EXIT_FAILURE;
	}

	free(handshake);

	t_header * buffer_in = malloc(sizeof(t_header));

	if (recv(socketNucleo, buffer_in, sizeof(t_header), MSG_WAITALL) == -1) {
		printf("No se pudo recibir el handshake de UMC.\n");
		return EXIT_FAILURE;
	}

	printf("%d\n", buffer_in->identificador);

	if (buffer_in->identificador == UMC) {
		//creo el hilo para atender a UMC
		printf("Se conecto umc.\n");
	} else {
		return EXIT_FAILURE;
	}

	free(buffer_in);

	return EXIT_SUCCESS;
}

