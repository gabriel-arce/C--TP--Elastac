/*
 * ProcesarConsola.c
 *
 *  Created on: 27 de abr. de 2016
 *      Author: gabriel
 */

#include "Consola.h"
#include <elestac_sockets.h>


int main(int argc, char **argv){

	int socketConsola;									//Descriptor de consola

	//Cargar configuracion
	printf("Inicializando Consola..\n");
	printf("please wait..\n");
	printf("\n");

	cargar_config();

	printf("IP Nucleo: %s\n", consola->ip_nucleo);
	printf("Puerto Nucleo: %d\n", consola->puerto_nucleo);


	//Luego de enviar, queda a la espera de la respuesta del nucleo


	// Verifica sin argumentos
	if (argc == 1){
		MostrarAyuda();
		return EXIT_SUCCESS;
		}

	// Verifica cantidad de argumentos
	if (argc != 2){
		MostrarMensajeDeError(CantidadArgumentosIncorrecta);
		return EXIT_FAILURE;
		}


	// Abrir Script
	if ( (in = fopen(argv[1],"rb")) == NULL ) {
		MostrarMensajeDeError(NoSePudoAbrirIn);
		return EXIT_FAILURE;
		}


	//Leer un archivo que contiene el programa ansisop y envia al nucleo

	//Conectar al nucleo
	if((socketConsola = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1){
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	proceso = malloc(sizeof(t_proceso));


	if ((enviarPorSocket(socketConsola, "Hola! Soy una consola!..")) == -1){
		MostrarMensajeDeError(NoSePudoEnviarSocket);
	};




	return EXIT_SUCCESS;
}



