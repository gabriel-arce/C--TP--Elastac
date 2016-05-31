/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(void){

	//Cargar configuracion
	cargarConfiguracion();

	//Conectar al nucleo
	conectarConNucleo();

	//Conectar al UMC
	conectarConUMC();



/*
	if (enviar_handshake(socketCPU, 5, 0) == -1) {
		printf("No se pudo enviar el handshake a umc. \n");
		return EXIT_FAILURE;
	}

	t_header * handshake_in = malloc(sizeof(t_header));

	recibir_handshake(socketCPU, handshake_in);

	if (handshake_in->identificador == 3) {
		printf("Se conecto umc\n");
		printf("Tamanio de pagina: %d", handshake_in->tamanio);

		tamanio_paginas = handshake_in->tamanio;    		 			        //Asignar el tamaño de paginas

	} else {
		return EXIT_FAILURE;
	}
*/

	//Escuchar al nucleo a la espera de nuevos PCBs
	escucharAlNucleo();

 	return EXIT_SUCCESS;
}
