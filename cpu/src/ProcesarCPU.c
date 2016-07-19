/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(void){

	signal(SIGUSR1,rutina);		//HotPlug
	signal(SIGINT, rutina);

	cargarConfiguracion();		//Cargar configuracion


	conectarConNucleo();		//Conectar al nucleo


	conectarConUMC();			//Conectar al UMC


	while(!hotPlugActivado){

		escucharAlNucleo();								//Escuchar al nucleo a la espera de nuevos PCBs
		cambiar_proceso_activo(pcbActual->pcb_pid);		//envio a umc nuevo PID

		while( (getQuantumPcb() <= getQuantum())  &&  (pcbCorriendo)){

			quantumSleep();
			ejecutarProximaInstruccion();
			actualizarQuantum();

		}

		if(getQuantumPcb() == getQuantum()){

			restaurarQuantum();
		}

		enviarPCB();

		borrarPCBActual();

	}

	desconectarCPU();



 	return EXIT_SUCCESS;
}
