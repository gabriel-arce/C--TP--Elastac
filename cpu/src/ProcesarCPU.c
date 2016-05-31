/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(void){

	int socketCPU;				    //Descriptor de CPU


	printf("PROCESO CPU \n");


	//Cargar configuracion
	cargarConfiguracion();


	//Conectar al nucleo
	if((socketCPU = clienteDelServidor(cpu->ip_nucleo, cpu->puerto_nucleo)) == -1)
		exit(1);


	//conexion con umc
	if((socketCPU = clienteDelServidor(cpu->ip_UMC, cpu->puerto_UMC)) == -1)
			exit(1);

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


 int i = 0;
 while(i < quantum){

	//me fijo el pc
	//pido instruccion a umc
	//analizarLinea()
i+=1;
}
	//devolver el pcb al nucleo


	return EXIT_SUCCESS;
}
