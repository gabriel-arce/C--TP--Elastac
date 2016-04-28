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

	//Conectar al nucleo
	if((socketCPU = clienteDelServidor(cpu->ip_nucleo, cpu->puerto_nucleo)) == -1)
		exit(1);

}
