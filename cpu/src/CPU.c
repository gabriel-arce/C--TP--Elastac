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

	hotPlugActivado = malloc(sizeof(bool));
	hotPlugActivado = false;



}

void conectarConNucleo(){
	if((socketNucleo = clienteDelServidor(cpu->ip_nucleo, cpu->puerto_nucleo)) == -1)
		salirPor("[CPU} No se pudo conectar al Nucleo");
	int result = -1;
	result = enviar_handshake(socketNucleo, 5);
	if (result == -1) {
		exit(EXIT_FAILURE);
	}

//	 int recibido = 1;
//	 while (recibido > 0) {
//		 aca recibe el pcb y ejecuta			<<<<---------EMPEZAR DESDE ACA A PROCESAR EL PCB!!
//	 }
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
	t_header * head = recibir_header(socketUMC);
	tamanio_paginas = head->tamanio;
	free(head);
}

void cambiar_proceso_activo(int pid) {
	int result = enviar_header(18, pid, socketUMC);

	if (result == -1)
		salirPor("No se pudo realizar el cambio de proceso activo");
}

// este servidor esta demas
void escucharAlNucleo(){
	 int listener;														//Descriptor de escucha
	 int i;
	 int maximo;													// Número de descriptor maximo
	 int newfd;
	 int nbytes;
	 int reuse;
	 char buffer[MAXIMO_BUFFER];

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//Crear socket de escucha
	listener = crearSocket();

	//descriptor para enlace
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1)
		salirPor("[CPU] No es posible reusar el socket\n");

	//Enlazar
	bindearSocket(listener, cpu->puerto_nucleo);

	//Escuchar
	escucharEn(listener);

	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	maximo = listener; // por ahora es éste

	while(1){

		read_fds = master; 								// cópialo

		if (select(maximo+1, &read_fds, NULL, NULL, NULL) == -1)
			salirPor("select");

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maximo; i++){

			if (FD_ISSET(i, &read_fds)){
				// ¡¡tenemos datos!!

				if (i == listener){

					if ((newfd = aceptarEntrantes(listener)) == -1)						// gestionar nuevas conexiones
						salirPor("accept");

					else {

						FD_SET(newfd, &master);														// añadir al conjunto maestro
						if (newfd > maximo)
							maximo = newfd;																	// actualizar el máximo
					}

				} else	{

					if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0){			// gestionar datos de un cliente
						// error o conexión cerrada por el cliente
						if (nbytes == 0)																			// conexión cerrada
							printf("[CPU] socket %d desconectado\n", i);
						else
							salirPor("recv");

						close(i); 																						// ¡Hasta luego!
						FD_CLR(i, &master); 																	// eliminar del conjunto maestro

					} else {
					// tenemos datos de algún cliente
						if (FD_ISSET(i, &master))	{
							recibirPCB(buffer);

						}
					}
				}
			}
		}
	}
}


void recibirPCB(char *buffer){

	pcbActual = malloc(sizeof(t_pcb));

	pcbActual = convertirPCB(buffer);					//Deserializar pcb

	puts("PCB recibido");

	cambiarEstadoACorriendo();

}

void enviarPCB(){

	//TODO enviar a nucleo

}

void borrarPCBActual(){

	//TODO free
}

void escribirBytes(uint32_t pagina, uint32_t offset, uint32_t size, t_valor_variable valorVariable){

	// TODO enviar a UMC

	//si hay algun error devuelve -1 y se destruye el programa
}

t_valor_variable leerBytesDeVariable(uint32_t pagina, uint32_t offset, uint32_t size){     //TODO puede retornar valor o un string

	t_valor_variable  valor;



}

char * leerBytesDeInstruccion(uint32_t pagina, uint32_t offset, uint32_t size){

	//TODO enviar a UMC
}

void mandarTextoANucleo(char* texto){

	//TODO enviar a Nucleo
}

void desconectarCPU(){

	//mandar a nucleo que muere este CPU
}



//=================================================================================================================================================================
//----------------------------------------------------------------Primitivas

t_posicion definirVariable(t_nombre_variable identificador_variable) {

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

	return * variableStack->posicion;
	}


t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable) {

	t_variable_stack * variableStack;

	variableStack =  buscarVariableEnStack(identificador_variable);  //busco la variable por el identificador
	return * variableStack->posicion;
}

t_valor_variable dereferenciar(t_posicion direccion_variable) {

	t_valor_variable  valorVariable;

	valorVariable = leerBytesDeVariable(direccion_variable.pagina, direccion_variable.offset, direccion_variable.size);

	return  valorVariable;

}

void asignar(t_posicion direccion_variable, t_valor_variable valor) {

	escribirBytes(direccion_variable.pagina, direccion_variable.offset, direccion_variable.size, valor);

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

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

	desactivarStackActivo();
	crearStack();


}

void retornar(t_valor_variable retorno){

	//cambiar stackActivo
}

int imprimir(t_valor_variable valor_mostrar){

	//mandar a nucleo
}

int imprimirTexto(char* texto){

	if(string_equals_ignore_case(texto, "end")){

		cambiarEstadoATerminado();
	}

	mandarTextoANucleo(texto);

	return string_length(texto);
}

int entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	//cambiar estado a bloqueado
	//mandar a nucleo

}

void wait(t_nombre_semaforo identificador_semaforo){

	// mandar a nucleo
}

void signals(t_nombre_semaforo identificador_semaforo){

	//mandar a nucleo
}


//=================================================================================================================================================================
//----------------------------------------------------------------------Hot plug (signal)


void rutina (int n) {
	switch (n) {
		case SIGUSR1:
			printf("Hot plug activado \n");
			if(pcbCorriendo()){
				printf("Se desconectará el CPU cuando termine la ejecucion del programa actual\n");
			}
			hotPlugActivado = true;
	}
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

	actualizarPC();
	instruccionACorrer = buscarProximaInstruccion();    // busco la instruccion en el indice de codigo


	instruccionEnString = obtenerInstruccion(instruccionACorrer);   //pido la instruccion al umc

	analizadorLinea(instruccionEnString, &functions, &kernel_functions);

	free(instruccionACorrer);


}

t_indice_de_codigo * buscarProximaInstruccion(){
	int pc;

	pc = pcbActual->pcb_pc;
	return list_get(pcbActual->indice_codigo, pc);   //hay que ver si la lista empieza en 0 o en 1
}

bool pcbCorriendo(){

	return (pcbActual->estado == Corriendo);
}

void restaurarQuantum(){

	pcbActual->quantum_actual = 0;
}

void actualizarQuantum(){

	pcbActual->quantum_actual ++;
}

void cambiarEstadoACorriendo(){

	pcbActual->estado = Corriendo;
	puts("PCB corriendo");

}

void cambiarEstadoAFinQuantum(){

	pcbActual->estado = FinQuantum;
	puts("finalizo el Quantum");
}

void cambiarEstadoATerminado(){

	pcbActual->estado = Terminado;
	puts("Se termino el programa actual");
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
	stackActivo->stackDeRetorno = true;

}



