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

	if ((enviarPorSocket(socketNucleo, "Hola! Soy nucleo!..")) == -1){
//		MostrarMensajeDeError(ETIQUETA_NUCLEO, NoSePudoEnviarSocket);
		return EXIT_FAILURE;
	};


	return EXIT_SUCCESS;
}

