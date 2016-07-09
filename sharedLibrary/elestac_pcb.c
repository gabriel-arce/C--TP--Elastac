/*
 * elestac_pcb.c
 *
 *  Created on: 31 de may. de 2016
 *      Author: gabriel
 */

#include "elestac_pcb.h"
t_pcb *crearPCB(char *programa, int fd, uint8_t stack_size, t_queue *cola_pcb){

/*    char * prog1 = "begin \n variables a, b, c \n a = b + 12 \n print b \n textPrint foo\n end\"";
    char * prog2 = "begin \n print b \n end";
    char * prog3 = "begin\nend";
    char * prog4 = "end";
    char * prog5 = "begin";
    t_metadata_program * meta1 = metadata_desde_literal(prog1);
    t_metadata_program * meta2 = metadata_desde_literal(prog2);
    t_metadata_program * meta3 = metadata_desde_literal(prog3);
    t_metadata_program * meta4 = metadata_desde_literal(prog4);
    t_metadata_program * meta5 = metadata_desde_literal(prog5);*/

    t_metadata_program *meta = metadata_desde_literal(programa);

/*    printf("instrucciones_size prog1 :%d\n", meta1->instrucciones_size);
    printf("instrucciones_size prog2 :%d\n", meta2->instrucciones_size);
    printf("instrucciones_size prog3 :%d\n", meta3->instrucciones_size);
    printf("instrucciones_size prog4 :%d\n", meta4->instrucciones_size);
    printf("instrucciones_size prog5 :%d\n", meta5->instrucciones_size);
    printf("instrucciones_size prog5 :%d\n", meta->instrucciones_size);*/

	t_pcb *pcb;
	t_sp *sp;
	t_indice_de_codigo *indiceCodigo;

	if ((pcb = malloc(sizeof(t_pcb))) == NULL)
		puts("No se pudo alocar para el pcb");

	if ((sp = malloc(sizeof(t_sp))) == NULL)
		puts("No se pudo alocar indice de codigo en PCB");

	if ((indiceCodigo = malloc(sizeof(t_indice_de_codigo))) == NULL)
		puts("No se pudo alocar para indice de codigo");

	pcb->pcb_pid 					= crearPCBID(cola_pcb);
	pcb->pcb_pc					= meta->instruccion_inicio;
	pcb->paginas_codigo	= stack_size;
	sp->offset	= 0;
	sp->pagina	= 0;

	pcb->indice_codigo = list_create();

	for(int i = 0; i <= meta->instrucciones_size; i++){
		indiceCodigo->posicion	= meta->instrucciones_serializado[i].start;
		indiceCodigo->tamanio	= meta->instrucciones_serializado[i].offset;
		list_add(pcb->indice_codigo, indiceCodigo);
	}

/*	sp->pagina 	= meta->instrucciones_serializado[pcb->pcb_pc].start;
	sp->offset	= meta->instrucciones_serializado[pcb->pcb_pc].offset;*/

	pcb->pcb_sp = sp;
	pcb->cantidad_de_etiquetas	= meta->cantidad_de_etiquetas;
	pcb->estado = Listo;
	pcb->consola	= fd;
	pcb->quantum_actual	= 0;


	pcb->indice_etiquetas = meta->etiquetas;

	list_create(pcb->indice_stack);
	if ((pcb->indice_stack = malloc(sizeof(t_list))) == NULL)
		puts("No se pudo alocar para indice de stack");

	return pcb;

}


void destruirPCB(t_pcb *pcb){
	free(pcb->indice_etiquetas);
//	free(pcb->indice_stack.args);
	free(pcb);
}

char* serializarPCB (t_pcb* pcb)
{
	t_indice_de_codigo *indiceCodigo = malloc(sizeof(t_indice_de_codigo));
	t_stack *indiceStack = malloc(sizeof(t_stack));
	t_posicion *args = malloc(sizeof(t_posicion));
	t_variable_stack *variableStack = malloc(sizeof(t_variable_stack));
	char* serial = string_new();

	string_append(&serial,"0");											//Tipo de Proceso
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_pid));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_pc));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_sp->offset));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->pcb_sp->pagina));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->paginas_codigo));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(list_size(pcb->indice_codigo)));


	for(int i = 0; i < list_size(pcb->indice_codigo); i++){
		indiceCodigo = list_get(pcb->indice_codigo,i);
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(indiceCodigo->posicion));
		string_append(&serial, string_itoa(indiceCodigo->tamanio));
	}

	string_append(&serial, SERIALIZADOR);
	string_append(&serial, &pcb->indice_etiquetas);
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->cantidad_de_etiquetas));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(list_size(pcb->indice_stack)));

	for(int i = 0; i < list_size(pcb->indice_stack); i++){
		indiceStack = list_get(pcb->indice_stack,i);
		string_append(&serial, SERIALIZADOR);
		for(int j = 0; j < list_size(indiceStack->args); j++){
			args = list_get(indiceStack->args, j);
			string_append(&serial, string_itoa(args->offset));
			string_append(&serial, SERIALIZADOR);
			string_append(&serial, string_itoa(args->pagina));
			string_append(&serial, SERIALIZADOR);
			string_append(&serial, string_itoa(args->size));
			string_append(&serial, SERIALIZADOR);
		}
		for(int k =1; k < list_size(indiceStack->vars); k++){
			variableStack = list_get(indiceStack->vars, k);
			string_append(&serial, string_itoa(variableStack->posicion->offset));
			string_append(&serial, SERIALIZADOR);
			string_append(&serial, string_itoa(variableStack->posicion->pagina));
			string_append(&serial, SERIALIZADOR);
			string_append(&serial, string_itoa(variableStack->posicion->size));
			string_append(&serial, SERIALIZADOR);
			string_append(&serial, variableStack->id);
		}
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(indiceStack->retPos));
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(indiceStack->retVar->offset));
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(indiceStack->retVar->pagina));
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(indiceStack->retVar->size));
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(indiceStack->stackActivo));
	}

	string_append(&serial, string_itoa(pcb->estado));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->consola));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->quantum_actual));

// TODO Me falta agregar las listas del PCB para serializar..

	free(indiceCodigo);
	return serial;
}

t_pcb *convertirPCB(char *mensaje){
	t_pcb *pcb;

	char** componentes = string_split(mensaje,SERIALIZADOR);
	pcb->pcb_pid								= atoi(componentes[0]);
	pcb->pcb_pc								= atoi(componentes[1]);
	pcb->pcb_sp								= atoi(componentes[2]);
	//pcb->indice_etiquetas 				= componentes[3];
	pcb->paginas_codigo 				= atoi(componentes[4]);
	/*
	pcb->indice_codigo.posicion	= atoi(componentes[5]);
	pcb->indice_codigo.tamanio	= atoi(componentes[6]);
	pcb->indice_stack.args 			= componentes[7];
	pcb->indice_stack.retPos 		= componentes[8];
	pcb->indice_stack.retVar 		= componentes[9];
	*/
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

