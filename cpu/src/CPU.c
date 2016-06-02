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
		.AnSISOP_llamarConRetorno        = llamarFuncion,
		.AnSISOP_retornar				 = retornar,
		.AnSISOP_imprimir				 = imprimir,
		.AnSISOP_imprimirTexto			 = imprimirTexto,
		.AnSISOP_entradaSalida           = entradaSalida,
		//.AnSISOP_wait					 = wait,
		//.AnSISOP_signal					 = signal,

};

AnSISOP_kernel kernel_functions = { };




t_CPU_config *cargar_config() {

	printf(" Cargando configuracion.. \n");
	t_config *config = config_create(CONFIG_PATH);
	t_CPU_config *cpu_config	= malloc(sizeof(t_CPU_config));

	cpu_config->ip_nucleo 					= string_new();
	cpu_config->ip_UMC						= string_new();

	cpu_config->puerto_nucleo = getIntProperty(config, "PUERTO_NUCLEO");
	cpu_config->puerto_UMC    = getIntProperty(config, "PUERTO_UMC");

	string_append(&cpu_config->ip_nucleo, getStringProperty(config, "IP_NUCLEO"));
	string_append(&cpu_config->ip_UMC, getStringProperty(config, "IP_UMC"));

	config_destroy(config);

	return cpu_config;
}


void cargarConfiguracion(){

	printf("PROCESO CPU \n");
	printf(" Cargando configuracion.. \n");
	t_config *config = config_create(CONFIG_PATH);

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



}


void conectarConNucleo(){
	if((socketNucleo = clienteDelServidor(cpu->ip_nucleo, cpu->puerto_nucleo)) == -1)
		salirPor("[CPU} No se pudo conectar al Nucleo");
}

void conectarConUMC(){
	if((socketUMC = clienteDelServidor(cpu->ip_UMC, cpu->puerto_UMC)) == -1)
		salirPor("[CPU] No se pudo conectar al UMC");
}


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
	t_pcb *pcb;

	//Deserializar pcb
	pcb = convertirPCB(buffer);

	//Aumentar program counter
	pcb->pcb_pc++;

	//solicitar al UMC la proxima sentencia a ejecutar, con el indice de codigo del PCB
	solicitarAlUMCProxSentencia();

	//cuando recibe, parsear

}


void solicitarAlUMCProxSentencia(){

	//Enviar al UMC
};

//--------------------------------------Primitivas

t_posicion definirVariable(t_nombre_variable identificador_variable) {

	t_variable_stack * variableStack = malloc(sizeof(variableStack));
	t_stack * stackActivo;

	variableStack->id = identificador_variable;

	if(list_size(pcbActual->indice_stack) == 0){				//me fijo si ya hay algun stack creado
		crearStackInicial();

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


	//stackActivo = buscarStackActivo();
	list_add(stackActivo->vars, &variableStack);						//Agrego variableStack a stackActivo

	//mandar a umc escribirBytes()

	return * variableStack->posicion;
	}


t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable) {

	t_variable_stack * variableStack;

	//variableStack =  buscarVariableEnStack(identificador_variable);  //busco la variable por el identificador
	return * variableStack->posicion;
}

t_valor_variable dereferenciar(t_posicion direccion_variable) {

	t_valor_variable * valorVariable;

	//mandar a umc leerBytes(direccion_variable)
	return * valorVariable;

}

void asignar(t_posicion direccion_variable, t_valor_variable valor) {

	//mandar a umc esribirBytes()
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

	//mandar a nucleo
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

	//mandar a nucleo
}

t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta){

}
t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar,t_puntero_instruccion linea_en_ejecuccion){

	//cambiar stackActivo
}
t_puntero_instruccion retornar(t_valor_variable retorno){

	//cambiar stackActivo
}

int imprimir(t_valor_variable valor_mostrar){

	//mandar a nucleo
}

int imprimirTexto(char* texto){

	//mandar a nucleo
	//hay que ver si es "end" y cambiar estado a FINALIZADO
}

int entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	//mandar a nucleo
}

int wait(t_nombre_semaforo identificador_semaforo){

	// mandar a nucleo
}

int signal(t_nombre_semaforo identificador_semaforo){

	//mandar a nucleo
}

//---------------------------------Otras

t_stack * buscarStackActivo(){

	//list_find(&pcbActual->indice_stack, stackActivo);
}

void crearStackInicial(){

	t_stack * stackNuevo = malloc(sizeof(stackNuevo));
	stackNuevo->stackActivo = true;
	stackNuevo->args = list_create();
	stackNuevo->vars = list_create();

	list_add(pcbActual->indice_stack, &stackNuevo);
}

t_variable_stack * buscarVariableEnStack(t_nombre_variable  id){
	t_stack * stackActivo = buscarStackActivo();

	int _es_la_que_busco(t_variable_stack *variable){
		return string_equals_ignore_case(variable->id, id);
	}

	return list_find(stackActivo->vars, (void*) _es_la_que_busco);
}

