/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(void){

	signal(SIGUSR1,rutina);


	cargarConfiguracion();		//Cargar configuracion


	conectarConNucleo();		//Conectar al nucleo


	conectarConUMC();			//Conectar al UMC


	while(!hotPlugActivado){

		escucharAlNucleo();			//Escuchar al nucleo a la espera de nuevos PCBs (cuando recibe aviso al UMC cambio de PID)


		while(pcbCorriendo()){

			ejecutarProximaInstruccion();
			//nanosleep()
			actualizarQuantum();

			if(getQuantumPcb() == getQuantum()){								//hay que ver si la ultima sentencia puede ser una entrada salida ( cambiaria a bloqueado y despues a findequantum)

				restaurarQuantum();
				cambiarEstadoAFinQuantum();
			}
		}

		enviarPCB();

		borrarPCBActual();

	}

	desconectarCPU();



 	return EXIT_SUCCESS;
}
