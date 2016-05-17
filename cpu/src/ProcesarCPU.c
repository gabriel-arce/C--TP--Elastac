/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(void){

	t_CPU_config *cpu = NULL;		//Configuracion
	int socketCPU;				    //Descriptor de CPU

	printf("PROCESO CPU \n");

	//cargar configuracion
	cpu = cargar_config();
	printf("IP Nucleo: %s\n", cpu->ip_nucleo);
	printf("Puerto Nucleo: %d\n", cpu->puerto_nucleo);
	printf("IP UMC: %s\n", cpu->ip_UMC);
	printf("Puerto UMC: %d\n", cpu->puerto_UMC);
	printf("Quantum: %d\n", cpu->quantum);


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
	} else {
		return EXIT_FAILURE;
	}



	return EXIT_SUCCESS;
}
