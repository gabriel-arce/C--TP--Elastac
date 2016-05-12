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
	//escuchar_procesos();

	//Mientras haya al menos haya un cpu, planifica..

	//while procesos_cpu > 1

	//planificar_procesos();

	//Crear socket al nucleo
	if((socketNucleo = clienteDelServidor(nucleo->ip_umc, nucleo->puerto_umc)) == -1){
//		MostrarMensajeDeError(ETIQUETA_NUCLEO, NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	if (enviar_handshake(socketNucleo, 2, 0) == -1) {
		printf("No se pudo enviar el handshake a umc. \n");
	}

	t_header * handshake_in = malloc(sizeof(t_header));

	recibir_handshake(socketNucleo, handshake_in);

	if (handshake_in->identificador == 3) {
		//creo el hilo para atender a UMC
		printf("Se conecto umc\n");
	} else {
		return EXIT_FAILURE;
	}

	free(handshake_in);

	return EXIT_SUCCESS;
}

