/*
 * ProcesarConsola.c
 *
 *  Created on: 27 de abr. de 2016
 *      Author: gabriel
 */

#include "Consola.h"
#include <elestac_sockets.h>


int main(void){

	int socketConsola = 0;		//Descriptor de consola
	int socketEscucha = 0;		//Descriptor de escucha

	//Cargar configuracion
	printf("Inicializando Consola..\n");
	printf("please wait..\n");
	printf("\n");

	cargar_config();

	printf("IP Nucleo: %s\n", consola->ip_nucleo);
	printf("Puerto Nucleo: %d\n", consola->puerto_nucleo);
	printf("Programa ANSISOP: %s\n", consola->programa_ansisop);


	//Luego de enviar, queda a la espera de la respuesta del nucleo
	//Leer un archivo que contiene el programa ansisop y envia al nucleo

	//Crear socket al nucleo
	if((socketConsola = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1){
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	if ((proceso = malloc(sizeof(t_proceso))) == NULL){
		return EXIT_FAILURE;
	}

	strcpy(proceso->tipoProceso, "CON");
	strcpy(proceso->contenido, consola->programa_ansisop);

	printf("%s\n", proceso->tipoProceso);
	printf("%s\n", proceso->contenido);


	if ((enviarPorSocket(socketConsola, "Hola! Soy una consola!..")) == -1){
		MostrarMensajeDeError(NoSePudoEnviarSocket);
		return EXIT_FAILURE;
	};


	if ((enviarPorSocket(socketConsola, proceso)) == -1){
		MostrarMensajeDeError(NoSePudoEnviarSocket);
		return EXIT_FAILURE;
	};

	/*
	//Crear socket desde el nucleo
	if((socketEscucha = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1){
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	escucharEn(socketEscucha);
*/

	return EXIT_SUCCESS;
}



