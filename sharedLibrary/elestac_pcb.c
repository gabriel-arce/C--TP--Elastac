/*
 * elestac_pcb.c
 *
 *  Created on: 31 de may. de 2016
 *      Author: gabriel
 */

#include "elestac_pcb.h"
t_pcb *crearPCB(char *programa, int fd, uint8_t stack_size, t_queue *cola_pcb){

    char * prog1 = "begin \n variables a, b, c \n a = b + 12 \n print b \n textPrint foo\n end\"";
    char * prog2 = "begin \n print b \n end";
    char * prog3 = "begin\nend";
    char * prog4 = "end";
    char * prog5 = "begin";
    t_metadata_program * meta1 = metadata_desde_literal(prog1);
    t_metadata_program * meta2 = metadata_desde_literal(prog2);
    t_metadata_program * meta3 = metadata_desde_literal(prog3);
    t_metadata_program * meta4 = metadata_desde_literal(prog4);
    t_metadata_program * meta5 = metadata_desde_literal(prog5);

    t_metadata_program *meta = metadata_desde_literal(programa);

    printf("instrucciones_size prog1 :%d\n", meta1->instrucciones_size);
    printf("instrucciones_size prog2 :%d\n", meta2->instrucciones_size);
    printf("instrucciones_size prog3 :%d\n", meta3->instrucciones_size);
    printf("instrucciones_size prog4 :%d\n", meta4->instrucciones_size);
    printf("instrucciones_size prog5 :%d\n", meta5->instrucciones_size);
    printf("instrucciones_size prog5 :%d\n", meta->instrucciones_size);

	t_pcb *pcb;
	t_sp *sp;

	if ((pcb = malloc(sizeof(t_pcb))) == NULL)
		puts("No se pudo alocar para el pcb");

	if ((sp = malloc(sizeof(t_sp))) == NULL)
		puts("No se pudo alocar indice de codigo en PCB");

	pcb->pcb_pid 					= 1;
	pcb->pcb_pc					= meta->instruccion_inicio;
	pcb->paginas_codigo	= stack_size;

	list_create(pcb->indice_codigo);

	sp->pagina 	= meta->instrucciones_serializado[pcb->pcb_pc].start;
	sp->offset	= meta->instrucciones_serializado[pcb->pcb_pc].offset;

	pcb->pcb_sp = sp;
	pcb->cantidad_de_etiquetas	= meta->cantidad_de_etiquetas;
	pcb->estado = Listo;
	pcb->consola	= fd;
	pcb->quantum_actual	= 0;


	pcb->indice_etiquetas = meta->etiquetas;

	list_create(pcb->indice_stack);

	return pcb;

}


void destruirPCB(t_pcb *pcb){
	free(pcb->indice_etiquetas);
//	free(pcb->indice_stack.args);
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
//	string_append(&serial, string_itoa(pcb->indice_codigo.posicion));
	string_append(&serial, SERIALIZADOR);
//	string_append(&serial, string_itoa(pcb->indice_codigo.tamanio));

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

