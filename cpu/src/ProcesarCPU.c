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

	//Escuchar al nucleo a la espera de nuevos PCBs
	escucharAlNucleo();

	while(pcbCorriendo()){

		ejecutarProximaInstruccion();
		actualizarQuantum();

		if(getQuantumPcb() == getQuantum()){

			restaurarQuantum();
			cambiarEstadoAFinQuantum();
		}
	}

	enviarPCB();

	borrarPCBActual();





 	return EXIT_SUCCESS;
}
