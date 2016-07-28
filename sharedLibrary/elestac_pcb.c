/*
 * elestac_pcb.c
 *
 *  Created on: 31 de may. de 2016
 *      Author: gabriel
 */

#include "elestac_pcb.h"

t_pcb *crearPCB(char *programa, int pid, int fd_consola, int tamanioPaginas) {

	int programa_length = strlen(programa);
	t_metadata_program *meta = metadata_desde_literal(programa);

	t_pcb *pcb = NULL;

	if ((pcb = malloc(sizeof(t_pcb))) == NULL) {
		puts("No se pudo alocar para el pcb");
		return NULL;
	}

	//***PID
	pcb->pcb_pid = pid;

	//***PC
	pcb->pcb_pc = meta->instruccion_inicio;

	//***CANTIDAD DE PAGINAS DEL CODIGO
	div_t output = div(programa_length, tamanioPaginas); //me fijo cuantas paginas ocupa el codigo y redondeo para arriba
	pcb->paginas_codigo = output.quot;
	if (output.rem > 0) {
		pcb->paginas_codigo += 1;
	}

	//***SP
	pcb->pcb_sp = malloc(sizeof(t_sp));
	if ((pcb->pcb_sp) == NULL) {
		puts("No se pudo alocar para el sp");
		free(pcb);
		return pcb;
	}
	pcb->pcb_sp->pagina = pcb->paginas_codigo + 1;
	pcb->pcb_sp->offset = 0;

	//***INDICE DE CODIGO
	pcb->indice_codigo = list_create();

	int i;
	for (i = 0; i <= meta->instrucciones_size; i++)
		list_add(pcb->indice_codigo,
				crearIndiceCodigo(meta->instrucciones_serializado[i].start,
						meta->instrucciones_serializado[i].offset));

	//***INDICE DE ETIQUETAS
	if(meta->etiquetas == NULL)
		pcb->indice_etiquetas = string_new();
	else
	pcb->indice_etiquetas = meta->etiquetas;
	//***CANTIDAD DE ETIQUETAS
	pcb->cantidad_de_etiquetas = meta->cantidad_de_etiquetas;

	//***INDICE DE STACK
	pcb->indice_stack = list_create();

	//***SOCKET CONSOLA
	pcb->consola = fd_consola;

	//***QUANTUM ACTUAL
	pcb->quantum_actual = 0;

	return pcb;
}


void destruirPCB(t_pcb *pcb){

	if ((pcb->pcb_sp) != NULL)
		free(pcb->pcb_sp);

	if (((pcb->indice_codigo) != NULL)&&((list_size(pcb->indice_codigo)) > 0))
		list_destroy_and_destroy_elements(pcb->indice_codigo, (void *) free);

	if ((pcb->indice_etiquetas) != NULL)
		free(pcb->indice_etiquetas);

	void destroyer(t_stack * stack_n) {
		if ((stack_n->args) != NULL)
			list_destroy_and_destroy_elements(stack_n->args, (void *) free);

		void inner_destroyer(t_variable_stack * var_stack) {
			free(var_stack->posicion);
			free(var_stack);
		}
		if ((stack_n->vars) != NULL)
			list_destroy_and_destroy_elements(stack_n->vars, (void *) inner_destroyer);

		if ((stack_n->retVar) != NULL)
			free(stack_n->retVar);

		free(stack_n);
	}
	if (((pcb->indice_stack) != NULL)&&((list_size(pcb->indice_stack)) > 0))
		list_destroy_and_destroy_elements(pcb->indice_stack, (void *) destroyer);

	free(pcb);
}

void salirPor(const char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

t_indice_de_codigo *crearIndiceCodigo(int start, int offset){
	t_indice_de_codigo *indiceCodigo =  malloc(sizeof(t_indice_de_codigo));
	indiceCodigo->posicion	= start;
	indiceCodigo->tamanio	= offset;
	return indiceCodigo;
}

//*****SERIALIZACION DEL PCB*****
void imprimir_pcb(t_pcb * pcb) {
	puts("\n**** PCB ****\n");
	printf("pid: %d\n", pcb->pcb_pid);
	printf("pc: %d\n", pcb->pcb_pc);
	printf("\n");
	printf("sp: \n");
	printf("   pagina: %d\n", pcb->pcb_sp->pagina);
	printf("   offset: %d\n", pcb->pcb_sp->offset);
	printf("\n");
	printf("paginas codigo: %d\n", pcb->paginas_codigo);
	printf("\n");
	printf("indice de codigo: \n");
	int i_c_length = list_size(pcb->indice_codigo);
	int n;
	printf("| P | T | \n");
	for (n = 0; n < i_c_length; n++) {
		t_indice_de_codigo * indice_n = list_get(pcb->indice_codigo, n);
		printf("| %d | %d | \n", indice_n->posicion, indice_n->tamanio);
	}
	printf("\n");
	printf("indice de etiquetas: \n");
	printf("%s\n", pcb->indice_etiquetas);
	printf("\n");
	printf("cantidad de etiquetas: %d\n", pcb->cantidad_de_etiquetas);
	printf("\n");
	printf("indice de stack: \n");
	int i_s_length = list_size(pcb->indice_stack);
	for (n = 0; n < i_s_length; n++) {
		printf("\n");
		puts("-------------------------------");
		printf("registro %d del stack\n", n);
		t_stack * stack_n = list_get(pcb->indice_stack, n);
		printf("\n");
		//MUESTRO LOS ARGS
		printf("args: \n");
		int args_length = list_size(stack_n->args);
		int a;
		printf("| P | O | S | \n");
		for (a = 0; a < args_length; a++) {
			t_posicion * arg_n = list_get(stack_n->args, a);
			printf("| %d | %d | %d | \n", arg_n->pagina, arg_n->offset,
					arg_n->size);
		}
		printf("\n");
		//MUESTRO LAS VARS
		printf("vars: \n");
		int vars_length = list_size(stack_n->vars);
		int v;
		printf("| id | P | O | S | \n");
		for (v = 0; v < vars_length; v++) {
			t_variable_stack * var_n = list_get(stack_n->vars, v);
			printf("| %c | %d | %d | %d | \n", var_n->id,
					var_n->posicion->pagina, var_n->posicion->offset,
					var_n->posicion->size);
		}
		printf("\n");
		printf("retPos: %d\n", stack_n->retPos);
		printf("\n");
		printf("retVar: \n");
		if (stack_n->retVar) {
			printf("P | O | S\n");
			printf("%d | %d | %d\n", stack_n->retVar->pagina,
					stack_n->retVar->offset, stack_n->retVar->size);
		} else {
			printf("    empty\n");
		}
		puts("-------------------------------");
	}
	printf("\n");
	printf("consola: %d\n", pcb->consola);
	printf("quantum actual: %d\n", pcb->quantum_actual);
}

int calcular_size_pcb(t_pcb * pcb) {
	int tamanio = 0;
	int i;

	//pid + pc + sp + paginas_codigo + cantidad_etiqs + consola + quantum
	tamanio += 32;

	int n_indice_cod = list_size(pcb->indice_codigo);
	//le tengo que agregar la cantidad al buffer
	tamanio += 4;
	//como el cada entrada del indice de codigo pesa 8 bytes entonces -> 8 * cantidad de indices
	tamanio += (n_indice_cod * sizeof(t_indice_de_codigo));

	int etiquet_length = string_length(pcb->indice_etiquetas);
	//le tengo que agregar el length del indice de etiquetas al buffer
	tamanio += 4;
	//y luego el indice de etiquetas
	tamanio += etiquet_length;

	int n_indice_stk = list_size(pcb->indice_stack);
	//le tengo que agregar la cantidad de elementos que va a tener el indice de stack
	tamanio += 4;
	for (i = 0; i < n_indice_stk; i++) {

		int args_size = 0;
		int vars_size = 0;

		t_stack * stack = list_get(pcb->indice_stack, i);

		//primero me fijo la cantidad de entradas que tiene args y lo agrego al buffer
		args_size = list_size(stack->args);
		tamanio += 4;
		tamanio += (args_size * sizeof(t_posicion));

		//luego las vars...
		vars_size = list_size(stack->vars);
		tamanio += 4;
		tamanio +=
				(vars_size * (sizeof(t_posicion) + sizeof(t_nombre_variable)));

		//el retPos
		tamanio += 4;

		//el retVar
		tamanio += sizeof(bool); //aca para ver si hay o no retVar
		if (stack->retVar) {
			tamanio += sizeof(t_posicion);
		}

		//el stack activo
		tamanio += sizeof(bool);
	}

	return tamanio;
}

void * serializar_pcb(t_pcb * pcb) {

	int pointer = 0;

	int buffer_size = calcular_size_pcb(pcb);
	void * buffer = malloc(buffer_size);
	memset(buffer, 0, buffer_size);

	//PID
	memcpy(buffer + pointer, &(pcb->pcb_pid), 4);
	pointer += 4;
	//PC
	memcpy(buffer + pointer, &(pcb->pcb_pc), 4);
	pointer += 4;
	//SP (pagina, offset)
	memcpy(buffer + pointer, &(pcb->pcb_sp->pagina), 4);
	pointer += 4;
	memcpy(buffer + pointer, &(pcb->pcb_sp->offset), 4);
	pointer += 4;
	//PAGINAS CODIGO
	memcpy(buffer + pointer, &(pcb->paginas_codigo), 4);
	pointer += 4;
	//----cantidad de elementos del indice de codigo
	int size_i_c = list_size(pcb->indice_codigo);
	memcpy(buffer + pointer, &(size_i_c), 4);
	pointer += 4;
	//INDICE DE CODIGO
	int k;
	for (k = 0; k < size_i_c; k++) {
		t_indice_de_codigo * indice = list_get(pcb->indice_codigo, k);
		memcpy(buffer + pointer, &(indice->posicion), 4);
		pointer += 4;
		memcpy(buffer + pointer, &(indice->tamanio), 4);
		pointer += 4;
	}
	//----length del indice de etiquetas
	int etiq_length = string_length(pcb->indice_etiquetas);
	memcpy(buffer + pointer, &(etiq_length), 4);
	pointer += 4;
	//INDICE DE ETIQUETAS
	memcpy(buffer + pointer, pcb->indice_etiquetas, etiq_length);
	pointer += etiq_length;
	//CANTIDAD DE ETIQUETAS
	memcpy(buffer + pointer, &(pcb->cantidad_de_etiquetas), 4);
	pointer += 4;
	//----cantidad de elementos del indice de stack
	int size_i_s = 0;
	size_i_s = list_size(pcb->indice_stack);
	memcpy(buffer + pointer, &(size_i_s), 4);
	pointer += 4;
	//INDICE DE STACK
	for (k = 0; k < size_i_s; k++) {
		//aca itero cada elemento del stack
		t_stack * stack = list_get(pcb->indice_stack, k);
		//++++primero las args
		int size_args = 0;
		size_args = list_size(stack->args);
		memcpy(buffer + pointer, &(size_args), 4);
		pointer += 4;
		int j;
		for (j = 0; j < size_args; j++) {
			t_posicion * arg_n = list_get(stack->args, j);
			memcpy(buffer + pointer, &(arg_n->pagina), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(arg_n->offset), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(arg_n->size), 4);
			pointer += 4;
		}
		//++++despues las vars
		int size_vars = 0;
		size_vars = list_size(stack->vars);
		memcpy(buffer + pointer, &(size_vars), 4);
		pointer += 4;
		for (j = 0; j < size_vars; j++) {
			t_variable_stack * var_n = list_get(stack->vars, j);
			memcpy(buffer + pointer, &(var_n->posicion->pagina), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(var_n->posicion->offset), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(var_n->posicion->size), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(var_n->id), 1);
			pointer += 1;
		}
		//RETPOS
		memcpy(buffer + pointer, &(stack->retPos), 4);
		pointer += 4;
		//hay_retvar + RETVAR
		bool retvar = true;
		if (stack->retVar) {
			memcpy(buffer + pointer, &(retvar), sizeof(bool));
			pointer += sizeof(bool);
			memcpy(buffer + pointer, &(stack->retVar->pagina), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(stack->retVar->offset), 4);
			pointer += 4;
			memcpy(buffer + pointer, &(stack->retVar->size), 4);
			pointer += 4;
		} else {
			retvar = false;
			memcpy(buffer + pointer, &(retvar), sizeof(bool));
			pointer += sizeof(bool);
		}
		//STACK ACTIVO
		memcpy(buffer + pointer, &(stack->stackActivo), sizeof(bool));
		pointer += sizeof(bool);
	}
	//CONSOLA
	memcpy(buffer + pointer, &(pcb->consola), 4);
	pointer += 4;
	//QUANTUM
	memcpy(buffer + pointer, &(pcb->quantum_actual), 4);
	pointer += 4;

	return buffer;
}

t_pcb * deserializar_pcb(void * buffer) {
	t_pcb * pcb = malloc(sizeof(t_pcb));
	pcb->indice_codigo = list_create();
	pcb->indice_stack = list_create();
	int pointer = 0;

	memcpy(&(pcb->pcb_pid), buffer + pointer, 4);
	pointer += 4;
	memcpy(&(pcb->pcb_pc), buffer + pointer, 4);
	pointer += 4;
	pcb->pcb_sp = malloc(sizeof(t_sp));
	memcpy(&(pcb->pcb_sp->pagina), buffer + pointer, 4);
	pointer += 4;
	memcpy(&(pcb->pcb_sp->offset), buffer + pointer, 4);
	pointer += 4;
	memcpy(&(pcb->paginas_codigo), buffer + pointer, 4);
	pointer += 4;
	//+++INDICE DE CODIGO
	int size_i_c = 0;
	memcpy(&(size_i_c), buffer + pointer, 4);
	pointer += 4;
	int k;
	for (k = 0; k < size_i_c; k++) {
		t_indice_de_codigo * indice = malloc(sizeof(t_indice_de_codigo));
		memcpy(&(indice->posicion), buffer + pointer, 4);
		pointer += 4;
		memcpy(&(indice->tamanio), buffer + pointer, 4);
		pointer += 4;
		list_add(pcb->indice_codigo, indice);
	}
	//+++INDICE DE ETIQUETAS
	int etiq_length = 0;
	memcpy(&(etiq_length), buffer + pointer, 4);
	pointer += 4;
	pcb->indice_etiquetas = malloc(etiq_length * sizeof(char));
	memcpy(pcb->indice_etiquetas, buffer + pointer, etiq_length);
	pointer += etiq_length;
	memcpy(&(pcb->cantidad_de_etiquetas), buffer + pointer, 4);
	pointer += 4;
	//+++INDICE DE STACK
	int size_i_s = 0;
	memcpy(&(size_i_s), buffer + pointer, 4);
	pointer += 4;
	for (k = 0; k < size_i_s; k++) {
		t_stack * stack = malloc(sizeof(t_stack));
		stack->args = list_create();
		stack->vars = list_create();
		//---ARGS
		int size_args = 0;
		memcpy(&(size_args), buffer + pointer, 4);
		pointer += 4;
		int j;
		for (j = 0; j < size_args; j++) {
			t_posicion * pos_n = malloc(sizeof(t_posicion));
			memcpy(&(pos_n->pagina), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(pos_n->offset), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(pos_n->size), buffer + pointer, 4);
			pointer += 4;
			list_add(stack->args, pos_n);
		}
		//---VARS
		int size_vars = 0;
		memcpy(&(size_vars), buffer + pointer, 4);
		pointer += 4;
		for (j = 0; j < size_vars; j++) {
			t_variable_stack * var_n = malloc(sizeof(t_variable_stack));
			var_n->posicion = malloc(sizeof(t_posicion));
			memcpy(&(var_n->posicion->pagina), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(var_n->posicion->offset), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(var_n->posicion->size), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(var_n->id), buffer + pointer, sizeof(char));
			pointer += sizeof(char);
			list_add(stack->vars, var_n);
		}
		//retPos
		memcpy(&(stack->retPos), buffer + pointer, 4);
		pointer += 4;
		//retVar
		bool hay_retVar = true;
		memcpy(&(hay_retVar), buffer + pointer, sizeof(bool));
		pointer += sizeof(bool);
		if (hay_retVar) {
			stack->retVar = malloc(sizeof(t_posicion));
			memcpy(&(stack->retVar->pagina), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(stack->retVar->offset), buffer + pointer, 4);
			pointer += 4;
			memcpy(&(stack->retVar->size), buffer + pointer, 4);
			pointer += 4;
		}
		//stkact
		memcpy(&(stack->stackActivo), buffer + pointer, sizeof(bool));
		pointer += sizeof(bool);

		//agrego el registro stack al indice de stack
		list_add(pcb->indice_stack, stack);
	}

	memcpy(&(pcb->consola), buffer + pointer, 4);
	pointer += 4;
	memcpy(&(pcb->quantum_actual), buffer + pointer, 4);
	pointer += 4;

	return pcb;
}

void enviar_pcb(t_pcb * pcb, int socket) {
	int bytes_to_send = calcular_size_pcb(pcb);

	void * buffer_pcb = serializar_pcb(pcb);

	send(socket, buffer_pcb, bytes_to_send, 0);
}

t_pcb * recibir_pcb(int socket, int bytes_a_recibir) {
	void * buffer_pcb = malloc(bytes_a_recibir);

	recv(socket, buffer_pcb, bytes_a_recibir, 0);

	t_pcb * pcb = deserializar_pcb(buffer_pcb);

	return pcb;
}
