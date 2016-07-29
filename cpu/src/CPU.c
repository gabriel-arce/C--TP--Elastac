/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU.h"

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		 = definirVariable,
		.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
		.AnSISOP_dereferenciar			 = dereferenciar,
		.AnSISOP_asignar				 = asignar,
		.AnSISOP_obtenerValorCompartida  = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida  = asignarValorCompartida,
		.AnSISOP_irAlLabel 				 = irAlLabel,
		.AnSISOP_llamarConRetorno        = llamarConRetorno,
		.AnSISOP_retornar				 = retornar,
		.AnSISOP_imprimir				 = imprimir,
		.AnSISOP_imprimirTexto			 = imprimirTexto,
		.AnSISOP_entradaSalida           = entradaSalida,
		.AnSISOP_finalizar				 = finalizar,
};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait					 = wait,
		.AnSISOP_signal					 = signals,

};



void cargarConfiguracion(char ** config_path){

	printf("PROCESO CPU \n");
	printf(" Cargando configuracion.. \n");
	t_config *config = config_create(config_path[1]);

	cpu = malloc(sizeof(t_CPU_config));

	cpu->ip_nucleo 					= string_new();
	cpu->ip_UMC						= string_new();

	cpu->puerto_nucleo = getIntProperty(config, "PUERTO_NUCLEO");
	cpu->puerto_UMC    = getIntProperty(config, "PUERTO_UMC");

	string_append(&cpu->ip_nucleo, getStringProperty(config, "IP_NUCLEO"));
	string_append(&cpu->ip_UMC, getStringProperty(config, "IP_UMC"));

	config_destroy(config);

	printf("IP Nucleo: %s\n", cpu->ip_nucleo);
	printf("Puerto Nucleo: %d\n", cpu->puerto_nucleo);
	printf("IP UMC: %s\n", cpu->ip_UMC);
	printf("Puerto UMC: %d\n", cpu->puerto_UMC);

	hotPlugActivado = false;



}

void conectarConNucleo(){
	if((socketNucleo = clienteDelServidor(cpu->ip_nucleo, cpu->puerto_nucleo)) == -1)
		salirPor("[CPU} No se pudo conectar al Nucleo");

//envio de handshake
	int result = -1;
	result = enviar_handshake(socketNucleo, 5);
	if (result == -1) {
		exit(EXIT_FAILURE);
	}

		//escucho a la espera del quantum y el quantum_sleep
	 	escucharPorSocket(socketNucleo);
	 	escucharPorSocket(socketNucleo);

}

void escucharPorSocket(int socket){			//unifico todos los recibos de headers
 	int recibido = -1;
 	t_header * header;

 	while(recibido < 0){
 		header = recibir_header(socket);
 		if(header != NULL){
 			recibido = 1;
 		}
 	}
 	switch(header->identificador){
 	case QUANTUM:
 	 		quantum = header->tamanio;
 	 		printf("quantum: %d \n",header->tamanio);
 	 		break;

 	case QUANTUM_SLEEP:
 	 		quantum_sleep = header->tamanio;
 	 		printf("quantum sleep: %d \n",header->tamanio);
 	 		break;

 	case TAMANIO_PAGINA:
 	 		tamanio_paginas = header->tamanio;
 	 		printf("tamanio de paginas: %d \n",header->tamanio);
 	 		break;

 	case EJECUTAR_PCB:
 	 		almacenarPCB(header->tamanio);
 	 		puts("PCB recibido");
 	 		break;
 	 	}
 	free(header);
 	}

void almacenarPCB(uint32_t tamanioBuffer){
 	pcbActual = recibir_pcb(socketNucleo, tamanioBuffer);
	pcbCorriendo = true;
  }


void conectarConUMC(){
	if((socketUMC = clienteDelServidor(cpu->ip_UMC, cpu->puerto_UMC)) == -1)
		salirPor("[CPU} No se pudo conectar a UMC");

//	---------envio el handshake a umc
	int result = -1;
	result = enviar_handshake(socketUMC, 5);
	if (result == -1) {
		exit(EXIT_FAILURE);
	}
//	---------recibo el tamaño de pagina
	escucharPorSocket(socketUMC);
}

void cambiar_proceso_activo(int pid) {
	int result = enviar_header(CAMBIO_DE_PROCESO_ACTIVO, pid, socketUMC);

	if (result == -1){
		salirPor("No se pudo realizar el cambio de proceso activo");
	}
	else{
		printf("cambio de proceso activo, PID: %d \n", pcbActual->pcb_pid);
	}

}


// escucho a la espera de algun PCB
void escucharAlNucleo(){

	escucharPorSocket(socketNucleo);
	}


void enviarPCB(){

	puts("enviando PCB a nucleo");

	enviar_header(20,calcular_size_pcb(pcbActual),socketNucleo);
	enviar_pcb(pcbActual, socketNucleo);

}

void escribirBytes(uint32_t pagina, uint32_t offset, uint32_t size, t_valor_variable valorVariable){

	t_header * header;

	if(enviar_solicitud_escritura(pagina, offset, size, &valorVariable, socketUMC) == -1){
		salirPor("no se pudo concretar la solicitud de escritura");
	}

	header = recibir_header(socketUMC);

	if(header->tamanio == 0){

		imprimirTexto("Segmentation fault, abortando el programa");
		finalizacionPrograma();
	}
}

t_valor_variable leerBytesDeVariable(uint32_t pagina, uint32_t offset, uint32_t size){

	t_valor_variable retorno;
	void* valor = malloc(size);

	if(enviar_solicitud_lectura(pagina, offset, size, socketUMC) == -1){
		salirPor("No se concreto la solicitud de lectura");
	}

	if(recv(socketUMC, valor, size, 0) != size){
		 		salirPor("no se pudo recibir la variable");
		 	}

	memcpy(&retorno,valor, size);
	return  retorno;
}

char * leerBytesDeInstruccion(uint32_t pagina, uint32_t offset, uint32_t size){

	void * instruccion = malloc(size);

	if(enviar_solicitud_lectura(pagina, offset, size, socketUMC) == -1){
			salirPor("No se concreto la solicitud de lectura");
		}

	if(recv(socketUMC, instruccion, size, 0) != size){
	 		salirPor("no se pudo recibir la instruccion");
	 	}

	return (char*)instruccion;
}

void mandarTextoANucleo(char* texto){

	if(enviar_texto(texto, socketNucleo) == -1){
	 		salirPor("no se pudo enviar texto a nucleo");
	 	}
}

void desconectarCPU(){

	puts("Desconectando CPU");

	enviar_header(FINALIZACION_DE_CPU,0,socketNucleo);

	free(cpu->ip_UMC);
	free(cpu->ip_nucleo);
	free(cpu);
}



//=================================================================================================================================================================
//----------------------------------------------------------------Primitivas

t_puntero definirVariable(t_nombre_variable identificador_variable) {


	t_variable_stack * variableStack = malloc(sizeof(t_variable_stack));
	t_posicion * posicion = malloc(sizeof(t_posicion));
	t_stack * stackActivo;

	variableStack->posicion = posicion;

	variableStack->id = identificador_variable;

	if(list_size(pcbActual->indice_stack) == 0){				//me fijo si ya hay algun stack creado
		crearStack();

	}

	if(((pcbActual->pcb_sp->offset) + 4) <= tamanio_paginas){     //me fijo si la variable entra (totalmente) en la pagina  (va a haber fragmentacion interna)

		variableStack->posicion->pagina = (pcbActual->pcb_sp->pagina);
		variableStack->posicion->offset = (pcbActual->pcb_sp->offset);
		variableStack->posicion->size = 4;

		pcbActual->pcb_sp->offset += 4;

	}
	else {

		variableStack->posicion->pagina = ((pcbActual->pcb_sp->pagina) + 1);    //pongo la variable en la pagina siguiente
		variableStack->posicion->offset = 0;
		variableStack->posicion->size = 4;

		pcbActual->pcb_sp->pagina += 1;									//pagina siguiente en el stack pointer
		pcbActual->pcb_sp->offset = 4;
	}


	stackActivo = buscarStackActivo();
	list_add(stackActivo->vars, variableStack);						//Agrego variableStack a stackActivo

	printf("Escribir variable en UMC:  %c \n", identificador_variable);

	escribirBytes(variableStack->posicion->pagina, variableStack->posicion->offset, variableStack->posicion->size, 0);		//TODO En realidad no se tendrian que inicializar las variables

	return convertirPosicionAPuntero(variableStack->posicion);
	}


t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable) {


	t_variable_stack * variableStack;

	variableStack =  buscarVariableEnStack(identificador_variable);  //busco la variable por el identificador
	return convertirPosicionAPuntero(variableStack->posicion);
}

t_valor_variable dereferenciar(t_puntero direccion_variable) {

	t_valor_variable  valorVariable;
	t_posicion  posicion;

	posicion = convertirPunteroAPosicion(direccion_variable);

	puts("Leer variable en UMC");

	valorVariable = leerBytesDeVariable(posicion.pagina, posicion.offset, posicion.size);
	return  valorVariable;

}

void asignar(t_puntero direccion_variable, t_valor_variable valor) {
	t_posicion posicion = convertirPunteroAPosicion(direccion_variable);

	escribirBytes(posicion.pagina, posicion.offset, posicion.size, valor);

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

	t_header * header_in;

	enviar_obtener_valor_compartido( variable, socketNucleo);

	header_in = recibir_header(socketNucleo);

	if(header_in->identificador == 1){ salirPor("no existe la variable compartida");}

	return (header_in->tamanio);

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

	enviar_asignar_valor_compartido(variable, valor, socketNucleo);

	t_header * header = recibir_header(socketNucleo);

	if(header->tamanio == 1){ salirPor("no existe la variable compartida");}
	return valor;
}

void irAlLabel(t_nombre_etiqueta etiqueta){

	printf("Ir a etiqueta: %s", etiqueta);

	pcbActual->pcb_pc = metadata_buscar_etiqueta(etiqueta, pcbActual->indice_etiquetas, pcbActual->cantidad_de_etiquetas);

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){							//TODO faltan los argumentos!! (creo que no hay que hacer nada)

	printf("Ejecutando funcion: %s", etiqueta);

	uint32_t direccionDeRetorno; 			//numero que debera tomar el PC al finalizar la funcion
	t_posicion posicionDeRetorno;

	posicionDeRetorno = convertirPunteroAPosicion(donde_retornar);
	direccionDeRetorno = pcbActual->pcb_pc;     //Creo que el PC ya esta actualizado (confirmarlo)

	desactivarStackActivo();
	crearStack();
	asignarPosicionYDireccionDeRetorno(posicionDeRetorno, direccionDeRetorno);
	irAlLabel(etiqueta);


}

void retornar(t_valor_variable retorno){

	printf("Retornando valor: %d \n", retorno);

	retornarValorAVariable(retorno);
	modificarElPC();
	eliminarStackActivo();
	activarUltimoStack();
}

void imprimir(t_valor_variable valor_mostrar){

	printf("Imprimiendo valor: %d \n", valor_mostrar);

	if(enviar_valor_de_variable(valor_mostrar,socketNucleo) == -1){
	 			salirPor("No se concreto la impresion por pantalla");
	 		}
}

void imprimirTexto(char* texto){

	printf("Imprimiendo texto: %s", texto);

	mandarTextoANucleo(texto);

}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	printf ("Entrada y salida en: %s , por: %d tiempo \n", dispositivo, tiempo);

	enviar_entrada_salida(dispositivo, tiempo, socketNucleo);

	pcbCorriendo = false;

}

void wait(t_nombre_semaforo identificador_semaforo){

	printf("Wait semaforo:  %s \n", identificador_semaforo);

	t_header * header_in;

	enviar_wait_identificador_semaforo(identificador_semaforo, socketNucleo);

	//espera respuesta (si fue a bloqueado retorno pcb)
	header_in = recibir_header(socketNucleo);

	if(header_in == NULL){
		salirPor("no se recibio respuesta del estado del semaforo");
	}

	else{

	switch(header_in->identificador){

	case SEMAFORO_NO_BLOQUEADO:
		break;

	case SEMAFORO_BLOQUEADO:

		puts("Semaforo bloqueado, PCB detenido");

		pcbCorriendo = false;
		break;
		}
	}

}

void signals(t_nombre_semaforo identificador_semaforo){

	printf ("Signal semaforo:  %s \n", identificador_semaforo);

	enviar_signal_identificador_semaforo(identificador_semaforo, socketNucleo);
}

void finalizar(){

	printf ("Finalizando programa:  %d \n", pcbActual->pcb_pid);

	//mandarTextoANucleo("end");
	finalizacionPrograma();
	}



//=================================================================================================================================================================
//----------------------------------------------------------------------Hot plug (signal)


void rutina (int n) {
	switch (n) {
		case SIGUSR1:

			printf("Hot plug activado \n");
			printf("Se desconectará el CPU cuando termine la ejecucion del programa actual\n");

			hotPlugActivado = true;
			break;

		case SIGINT:	//TODO ver como abortar
			enviar_texto("Abortando el programa", socketNucleo);
			enviar_header(ABORTAR_PROGRAMA, pcbActual->pcb_pid, socketNucleo);
			enviarPCB();
			exit(EXIT_FAILURE);
			break;
	}
}



//=================================================================================================================================================================
//----------------------------------------------------------------------Otras

t_stack * buscarStackActivo(){

	return list_find(pcbActual->indice_stack, (void*) stackActivo);
}

bool stackActivo(t_stack * stack){

return (stack->stackActivo == true);

}


void crearStack(){

	t_stack * stackNuevo = malloc(sizeof(t_stack));
	stackNuevo->stackActivo = true;
	stackNuevo->args = list_create();
	stackNuevo->vars = list_create();
	stackNuevo->retVar = NULL;
	stackNuevo->retPos = 0;

	list_add(pcbActual->indice_stack, stackNuevo);
}

t_variable_stack * buscarVariableEnStack(t_nombre_variable  id){

	t_stack * stackActivo = buscarStackActivo();

	bool _es_la_que_busco(t_variable_stack *variable){
		//return string_equals_ignore_case(&variable->id, &id);
		return (id == (variable->id));
	}

	return list_find(stackActivo->vars, (void*) _es_la_que_busco);
}

int getQuantum(){
	return quantum;
}

int getQuantumPcb(){
	return pcbActual->quantum_actual;
}

void ejecutarProximaInstruccion(){

	printf("Quantum actual: %d \n", pcbActual->quantum_actual);

	char* instruccionEnString;

	t_indice_de_codigo * instruccionACorrer = malloc(sizeof(t_indice_de_codigo));

	instruccionACorrer = buscarProximaInstruccion();    // busco la instruccion en el indice de codigo
	actualizarPC();

	instruccionEnString = obtenerInstruccion(instruccionACorrer);   //pido la instruccion al umc

	printf("Ejecutando la instruccion: %s \n", instruccionEnString);

	analizadorLinea(instruccionEnString, &functions, &kernel_functions);

	free(instruccionACorrer);

}

t_indice_de_codigo * buscarProximaInstruccion(){
	int pc;

	pc = pcbActual->pcb_pc;
	return list_get(pcbActual->indice_codigo, pc);
}


void restaurarQuantum(){

	finDeQuantum();

	puts("Restaurando Quantum");

	pcbActual->quantum_actual = 1;
}

void actualizarQuantum(){

	pcbActual->quantum_actual ++;
}

void actualizarPC(){

	pcbActual->pcb_pc ++;
}

char* obtenerInstruccion(t_indice_de_codigo * instruccionACorrer){

	char* instruccion = string_new();
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
	uint32_t aux;
	uint32_t loQueGuardo;

	pagina = instruccionACorrer->posicion / tamanio_paginas;					//dividir devuelve el numero entero (redondeado para abajo)
	offset = (instruccionACorrer->posicion - (tamanio_paginas * pagina));
	aux = offset + instruccionACorrer->tamanio;

	if(aux <= tamanio_paginas){
		size= instruccionACorrer->tamanio;
		string_append(&instruccion, leerBytesDeInstruccion(pagina, offset, size));
	}
	else{

		while(aux > tamanio_paginas){

			loQueGuardo = (tamanio_paginas - offset);

			string_append(&instruccion, leerBytesDeInstruccion(pagina,offset, loQueGuardo));

			pagina++;
			offset = 0;
			aux -= loQueGuardo;

		}

		string_append(&instruccion, leerBytesDeInstruccion(pagina, offset, aux));
	}

	return instruccion;
}

void desactivarStackActivo(){
	t_stack * stackActivo;

	stackActivo = buscarStackActivo();
	stackActivo->stackActivo = false;

}

void asignarPosicionYDireccionDeRetorno(t_posicion donde_retornar, uint32_t direccionDeRetorno){
	t_stack * stackActivo;


	stackActivo = buscarStackActivo();
	stackActivo->retVar = malloc(sizeof(t_posicion));
	stackActivo->retVar = &donde_retornar;
	stackActivo->retPos = direccionDeRetorno;

}


void retornarValorAVariable(t_valor_variable retorno){
	t_stack * stackActivo;

	stackActivo = buscarStackActivo();
	escribirBytes(stackActivo->retVar->pagina,stackActivo->retVar->offset,stackActivo->retVar->size, retorno);   //escribo el retorno de la funcion de la variable de retorno

}


void modificarElPC(){
	t_stack * stackActivo;

	stackActivo = buscarStackActivo();
	pcbActual->pcb_pc = stackActivo->retPos;
}


void eliminarStackActivo(){
	int posicionUltimoStack;
	t_stack * ultimoStack;

	posicionUltimoStack = list_size(pcbActual->indice_stack);
	ultimoStack = list_get(pcbActual->indice_stack, posicionUltimoStack);

	actualizarStackPointer(ultimoStack);

	stack_destroy(ultimoStack);
	list_remove(pcbActual->indice_stack, posicionUltimoStack);
	free(ultimoStack);
}

void activarUltimoStack(){

	int ultimoStack;
	t_stack * stackAACtivar;

	ultimoStack = list_size(pcbActual->indice_stack);

	stackAACtivar =  list_get(pcbActual->indice_stack, ultimoStack);

	stackAACtivar->stackActivo = true;
}


t_posicion  convertirPunteroAPosicion(t_puntero puntero){				//para usar si explota con t_posicion

	t_posicion  posicion;
	div_t output;

	output = div(puntero, tamanio_paginas);

	posicion.pagina = output.quot;
	posicion.offset = output.rem;
	posicion.size = 4;

	return posicion;
}

t_puntero  convertirPosicionAPuntero(t_posicion * posicion){

	uint32_t  puntero;

	puntero = ((posicion->pagina * tamanio_paginas) + posicion->offset);

	return puntero;
}

void borrarPCBActual(){

	puts("Borrando PCB actual");
/*
	list_destroy_and_destroy_elements(pcbActual->indice_stack, (void*) stack_destroy);
	list_destroy(pcbActual->indice_codigo);
	free(pcbActual->indice_etiquetas);
	free(pcbActual->pcb_sp);
	free(pcbActual);
*/

	destruirPCB(pcbActual);

}

void quantumSleep(){

	div_t output;

	output = div(quantum_sleep,1000);

	   struct timespec time;
	   time.tv_sec = output.quot;
	   time.tv_nsec = (output.rem * 1000000);  //convierte milisegundos a nanosegundos

	if (nanosleep(&time, NULL) < 0 ){
		printf("Nano sleep system call failed \n");
	}
}

void stack_destroy(t_stack * stack){

	list_destroy(stack->args);
	list_destroy(stack->vars);
	free(stack->retVar);

}

void finalizacionPrograma(){
	pcbCorriendo = false;
	enviar_header(FINALIZACION_PROGRAMA,0,socketNucleo);
}

void finDeQuantum(){
	enviar_header(FINALIZACION_QUANTUM,0,socketNucleo);
}

void actualizarStackPointer(t_stack * stackAEliminar){

	t_posicion * posicionDeUltimaVariable;

	posicionDeUltimaVariable = list_get(stackAEliminar->vars, 0);

	pcbActual->pcb_sp->pagina = posicionDeUltimaVariable->pagina;
	pcbActual->pcb_sp->offset = posicionDeUltimaVariable->offset;
}

