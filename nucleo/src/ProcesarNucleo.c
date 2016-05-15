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

	t_header * handshake = malloc(sizeof(t_header));
	void * buffer_out = malloc(5);

	handshake->identificador = 2;
	handshake->tamanio = 0;

	memcpy(buffer_out, &handshake->identificador, sizeof(uint8_t));
	memcpy(buffer_out + sizeof(uint8_t) , &handshake->tamanio, sizeof(uint32_t));

	if (send(socketNucleo, buffer_out, 5, 0) == -1) {
		printf("Error en el send\n");
		exit(EXIT_FAILURE);
	}
	printf("Conexion con umc\n");

	free(buffer_out);
	free(handshake);

	void * buffer_in = malloc(5);
	t_header * head_in = malloc(sizeof(t_header));

	if (recv(socketNucleo, buffer_in, 5, 0) == -1) {
		printf("Error en el recv\n");
	}

	memcpy(&head_in->identificador, buffer_in, sizeof(uint8_t));
	memcpy(&head_in->tamanio, buffer_in + sizeof(uint8_t), sizeof(uint32_t));

	if (head_in->identificador == Tamanio_pagina) {
		tamanio_pagina = head_in->tamanio;
		printf("Tamaño de pagina: %d\n", tamanio_pagina);
	}

	return EXIT_SUCCESS;
}

