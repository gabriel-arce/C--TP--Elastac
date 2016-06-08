/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(void){


	cargarConfiguracion();		//Cargar configuracion


	conectarConNucleo();		//Conectar al nucleo


	conectarConUMC();			//Conectar al UMC

	while(1){


		escucharAlNucleo();			//Escuchar al nucleo a la espera de nuevos PCBs

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

	}



 	return EXIT_SUCCESS;
}
