/*
 * ProcesarCPU.c
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#include "CPU.h"


int main(int argc, char * argv[]){

	signal(SIGUSR1,rutina);		//HotPlug
	signal(SIGINT, rutina);

	if (chequear_argumentos(argc, 2) == -1)
			return EXIT_FAILURE;

	cargar_archivo_config(argv, (void *) cargarConfiguracion);


	conectarConNucleo();		//Conectar al nucleo


	conectarConUMC();			//Conectar al UMC


	while(!hotPlugActivado){

		escucharAlNucleo();								//Escuchar al nucleo a la espera de nuevos PCBs
		//imprimir_pcb(pcbActual);
		cambiar_proceso_activo(pcbActual->pcb_pid);		//envio a umc nuevo PID

/*		//Modifico finalizacion programa
		enviar_header(21,sizeof(pcbActual),socketNucleo);
		enviar_pcb(pcbActual, socketNucleo);*/		/* Anduvo OK */

		//Modificar wait
		/*enviar_wait_identificador_semaforo("b", socketNucleo);
		enviar_pcb(pcbActual, socketNucleo);	/* TODO Preguntar como enviar desde aca un wait a nucleo.. */

/*		//Modificar obtener valor compartida
		t_nombre_compartida nombre = "colas";
		enviar_obtener_valor_compartido(nombre, socketNucleo);*/		/* TODO Preguntar como enviar desde aca a nucleo.. */

		while( (getQuantumPcb() <= getQuantum())  &&  (pcbCorriendo)){

			quantumSleep();
			ejecutarProximaInstruccion();
			actualizarQuantum();

		}

		if((getQuantumPcb() == (getQuantum() + 1)) && (!pcbFinalizado)){

			restaurarQuantum();
		}

		enviarPCB();
		//imprimir_pcb(pcbActual);
		borrarPCBActual();

	}

	desconectarCPU();



 	return EXIT_SUCCESS;
}
