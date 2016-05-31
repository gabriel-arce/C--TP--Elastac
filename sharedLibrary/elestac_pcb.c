/*
 * elestac_pcb.c
 *
 *  Created on: 31 de may. de 2016
 *      Author: gabriel
 */

#include "elestac_pcb.h"

t_pcb *crearPCB(char *programa, int fd, uint8_t stack_size, t_queue *cola_pcb){
	t_pcb *pcb = malloc(sizeof(t_pcb));

	const char* PROGRAMA = "#!/usr/bin/ansisop \n begin \n variables a, b, c \n  a = b + 12 \n print b \n textPrint foo\n end";

	//Obtener metadata del programa
	t_metadata_program* metadata = malloc(sizeof(t_metadata_program));
	metadata = metadata_desde_literal(PROGRAMA);

	pcb->pcb_pid	= crearPCBID(cola_pcb);
	pcb->pcb_pc	= metadata->instruccion_inicio;
	pcb->pcb_sp	= 0;
	pcb->indice_etiquetas = metadata->etiquetas;
	pcb->paginas_codigo = stack_size;
	pcb->indice_codigo.posicion	= metadata->instrucciones_serializado[0].start;
	pcb->indice_codigo.tamanio	= metadata->instrucciones_serializado[0].offset ;
	pcb->indice_stack.args = NULL;
	pcb->indice_stack.retPos = 0;
	pcb->indice_stack.retVar = 0;
	pcb->quantum = 0;
	pcb->estado = Listo;
	pcb->consola = fd;

	printf("PCB creado..\n");
	printf("PID: %d\n", pcb->pcb_pid);
	printf("PC: %d\n", pcb->pcb_pc);
	printf("SP: %d\n", pcb->pcb_sp);
	printf("Indice Etiquetas: %s\n", pcb->indice_etiquetas);
	printf("Paginas de Codigo: %d\n", pcb->paginas_codigo);
	printf("Indice de Codigo: %d, %d\n", pcb->indice_codigo.posicion, pcb->indice_codigo.tamanio);

	return pcb;
}

void destruirPCB(t_pcb *pcb){
	free(pcb->indice_etiquetas);
	free(pcb->indice_stack.args);
	free(pcb);
}

char* serializarPCB (t_pcb* pcb)
{
	char* serial = string_new();
	string_append(&serial,"0");											//Tipo de Proceso
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_pid));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_pc));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_sp));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->paginas_codigo));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->indice_etiquetas));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->indice_codigo.posicion));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->indice_codigo.tamanio));

	return serial;
}

t_pcb *convertirPCB(char *mensaje){
	t_pcb *pcb;

	char** componentes = string_split(mensaje,SERIALIZADOR);
	pcb->pcb_pid								= atoi(componentes[0]);
	pcb->pcb_pc								= atoi(componentes[1]);
	pcb->pcb_sp								= atoi(componentes[2]);
	pcb->indice_etiquetas 				= componentes[3];
	pcb->paginas_codigo 				= atoi(componentes[4]);
	pcb->indice_codigo.posicion	= atoi(componentes[5]);
	pcb->indice_codigo.tamanio	= atoi(componentes[6]);
	pcb->indice_stack.args 			= componentes[7];
	pcb->indice_stack.retPos 		= componentes[8];
	pcb->indice_stack.retVar 		= componentes[9];
	pcb->quantum 							= atoi(componentes[10]);
	pcb->estado								= atoi(componentes[11]);
	pcb->consola								= atoi(componentes[12]);

	return pcb;
}

int crearPCBID(t_queue *cola){
	if(queue_is_empty(cola))
		return 1;
	return queue_size(cola) + 1;
}

void salirPor(const char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}
