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

};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait					 = wait,
		.AnSISOP_signal					 = signals,

};



void cargarConfiguracion(){

	printf("PROCESO CPU \n");
	printf(" Cargando configuracion.. \n");
	t_config *config = config_create(CONFIG_PATH);

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
 	 		break;

 	case QUANTUM_SLEEP:
 	 		quantum_sleep = header->tamanio;
 	 		break;

 	case TAMANIO_PAGINA:
 	 		tamanio_paginas = header->tamanio;
 	 		break;

 	case EJECUTAR_PCB:
 	 		almacenarPCB(header->tamanio);
 	 		break;
 	 	}
 	free(header);
 	}

void almacenarPCB(uint32_t tamanioBuffer){
 	void * buffer = malloc(tamanioBuffer);
 	recv(socketNucleo,buffer, tamanioBuffer,0);
 	recibirPCB(buffer);
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

	if (result == -1)
		salirPor("No se pudo realizar el cambio de proceso activo");
}


// escucho a la espera de algun PCB
void escucharAlNucleo(){
	escucharPorSocket(socketNucleo);
	}


void recibirPCB(void *buffer){

	pcbActual = malloc(sizeof(t_pcb));

	pcbActual = convertirPCB(buffer);

	puts("PCB recibido");

	pcbCorriendo = true;

}

void enviarPCB(){

	enviar_pcb(pcbActual, socketNucleo);

}

void escribirBytes(uint32_t pagina, uint32_t offset, uint32_t size, t_valor_variable valorVariable){

	t_header * header;
	if(enviar_solicitud_escritura(pagina, offset, size, &valorVariable, socketUMC) == -1){
		salirPor("no se pudo concretar la solicitud de escritura");
	}
	header = recibir_header(socketUMC);

	if(header->tamanio == 0){

		imprimirTexto("Stack Overflow, abortando el programa");
		finalizacionPrograma();
	}
}

t_valor_variable leerBytesDeVariable(uint32_t pagina, uint32_t offset, uint32_t size){

	t_valor_variable valor;
	t_header * header;

	if(enviar_solicitud_lectura(pagina, offset, size, socketUMC) == -1){
		salirPor("No se concreto la solicitud de lectura");
	}

	header = recibir_header(socketUMC);
	if(header->tamanio == 0){salirPor("no se pudo leer la variable");}

	valor = recibir_valor_de_variable(socketUMC);

	return valor;
}

char * leerBytesDeInstruccion(uint32_t pagina, uint32_t offset, uint32_t size){

	char * instruccion = string_new();

	if(enviar_solicitud_lectura(pagina, offset, size, socketUMC) == -1){
			salirPor("No se concreto la solicitud de lectura");
		}

	if(recv(socketUMC, instruccion, size, 0) <= 0){
	 		salirPor("no se pudo recibir la instruccion");
	 	}

	return instruccion;
}

void mandarTextoANucleo(char* texto){

	if(enviar_texto(texto, socketNucleo) == -1){
	 		salirPor("no se pudo enviar texto a nucleo");
	 	}
}

void desconectarCPU(){

	enviar_header(FINALIZACION_DE_CPU,0,socketNucleo);

	free(cpu->ip_UMC);
	free(cpu->ip_nucleo);
	free(cpu);
}



//=================================================================================================================================================================
//----------------------------------------------------------------Primitivas

t_puntero definirVariable(t_nombre_variable identificador_variable) {

	t_variable_stack * variableStack = malloc(sizeof(variableStack));
	t_stack * stackActivo;

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
	list_add(stackActivo->vars, &variableStack);						//Agrego variableStack a stackActivo

	escribirBytes(variableStack->posicion->pagina, variableStack->posicion->offset, variableStack->posicion->size, 0);		//En realidad no se tendrian que inicializar las variables

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

	valorVariable = leerBytesDeVariable(posicion.pagina, posicion.offset, posicion.size);
	return  valorVariable;

}

void asignar(t_puntero direccion_variable, t_valor_variable valor) {
	t_posicion posicion = convertirPunteroAPosicion(direccion_variable);

	escribirBytes(posicion.pagina, posicion.offset, posicion.size, valor);

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){				//TODO enviar a nucleo

	//mandar a nucleo

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){			//TODO enviar a nucleo

	//mandar a nucleo
	return valor;
}

void irAlLabel(t_nombre_etiqueta etiqueta){


	pcbActual->pcb_pc = metadata_buscar_etiqueta(etiqueta, pcbActual->indice_etiquetas, pcbActual->cantidad_de_etiquetas);

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){							//TODO faltan los argumentos!!
	uint32_t direccionDeRetorno; 			//numero que debera tomar el PC al finalizar la funcion
	t_posicion posicionDeRetorno;

	posicionDeRetorno = convertirPunteroAPosicion(donde_retornar);
	direccionDeRetorno = pcbActual->pcb_pc + 1;

	desactivarStackActivo();
	crearStack();
	asignarPosicionYDireccionDeRetorno(posicionDeRetorno, direccionDeRetorno);
	irAlLabel(etiqueta);		//TODO hay que ver si el parser lo hace solo


}

void retornar(t_valor_variable retorno){

	retornarValorAVariable(retorno);
	modificarElPC();
	eliminarStackActivo();
	activarUltimoStack();
}

void imprimir(t_valor_variable valor_mostrar){

	if(enviar_valor_de_variable(valor_mostrar,socketNucleo) == -1){
	 			salirPor("No se concreto la impresion por pantalla");
	 		}
}

void imprimirTexto(char* texto){

	if(string_equals_ignore_case(texto, "end")){

		if(list_size(pcbActual->indice_stack) == 1){

		finalizacionPrograma();
	}
		else{
			//TODO tendria que retornar a la funcion anterior pero tengo que hacer una prueba para ver como funciona el parser
			//(Creo que no hay que hacer nada en este caso)
		}
	}

	mandarTextoANucleo(texto);

}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	//TODO serializar y mandar
	pcbCorriendo = false;

}

void wait(t_nombre_semaforo identificador_semaforo){

	enviar_wait_identificador_semaforo(identificador_semaforo, socketUMC);
	//espera respuesta (si fue a bloqueado retorno pcb)
	t_header * header_in;

	enviar_wait_identificador_semaforo(identificador_semaforo, socketNucleo);

	header_in = recibir_header(socketNucleo);

	if(header_in == NULL){
		salirPor("no se recibio respuesta del estado del semaforo");
	}

	else{

	switch(header_in->identificador){

	case SEMAFORO_NO_BLOQUEADO:
		break;

	case SEMAFORO_BLOQUEADO:
		pcbCorriendo = false;
		break;
		}
	}

}

void signals(t_nombre_semaforo identificador_semaforo){

	enviar_signal_identificador_semaforo(identificador_semaforo, socketNucleo);
}


//=================================================================================================================================================================
//----------------------------------------------------------------------Hot plug (signal)


void rutina (int n) {
	switch (n) {
		case SIGUSR1:
			printf("Hot plug activado \n");
				printf("Se desconectará el CPU cuando termine la ejecucion del programa actual\n");
			}
			hotPlugActivado = true;
	}



//=================================================================================================================================================================
//----------------------------------------------------------------------Otras

t_stack * buscarStackActivo(){

	return list_find(pcbActual->indice_stack, (void*) stackActivo);
}

int stackActivo(t_stack * stack){

return (stack->stackActivo);

}


void crearStack(){

	t_stack * stackNuevo = malloc(sizeof(stackNuevo));
	stackNuevo->stackActivo = true;
	stackNuevo->args = list_create();
	stackNuevo->vars = list_create();

	list_add(pcbActual->indice_stack, &stackNuevo);
}

t_variable_stack * buscarVariableEnStack(t_nombre_variable  id){

	t_stack * stackActivo = buscarStackActivo();

	int _es_la_que_busco(t_variable_stack *variable){
		return string_equals_ignore_case(&variable->id, &id);
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
	t_indice_de_codigo * instruccionACorrer = malloc(sizeof(t_indice_de_codigo));
	char* instruccionEnString;

	instruccionACorrer = buscarProximaInstruccion();    // busco la instruccion en el indice de codigo
	actualizarPC();

	instruccionEnString = obtenerInstruccion(instruccionACorrer);   //pido la instruccion al umc

	analizadorLinea(instruccionEnString, &functions, &kernel_functions);

	free(instruccionACorrer);


}

t_indice_de_codigo * buscarProximaInstruccion(){
	int pc;

	pc = pcbActual->pcb_pc;
	return list_get(pcbActual->indice_codigo, pc);
}


void restaurarQuantum(){

	pcbActual->quantum_actual = 0;
}

void actualizarQuantum(){

	pcbActual->quantum_actual ++;
}

void actualizarPC(){

	pcbActual->pcb_pc ++;
}

char* obtenerInstruccion(t_indice_de_codigo * instruccionACorrer){				//TODO testear algoritmo

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

	list_destroy_and_destroy_elements(pcbActual->indice_stack, (void*) stack_destroy);
	list_destroy(pcbActual->indice_codigo);
	free(pcbActual->indice_etiquetas);
	free(pcbActual->pcb_sp);
	free(pcbActual);

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
	pcbCorriendo = false;

	enviar_header(FINALIZACION_QUANTUM,0,socketNucleo);
}


