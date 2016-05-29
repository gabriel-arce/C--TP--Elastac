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

//--------------------------------------Primitivas

t_posicion definirVariable(t_nombre_variable identificador_variable) {

	t_variable_stack * variableStack = malloc(sizeof(variableStack));
	t_posicion * posicionVariable = malloc(sizeof(posicionVariable));

	variableStack->id = identificador_variable;

	if(pcbActual->indice_stack->elements_count == 0){				//me fijo si ya hay algun stack creado
		t_stack * stackNuevo = malloc(sizeof(stackNuevo));
		stackNuevo->stackActivo = true;
		stackNuevo->args = list_create();

	}

	if(((pcbActual->pcb_sp->offset) + 4) <= tamanio_paginas){     //me fijo si la variable entra (totalmente) en la pagina  (va a haber fragmentacion interna)

		posicionVariable->pagina = (pcbActual->pcb_sp->pagina);
		posicionVariable->offset = (pcbActual->pcb_sp->offset);
		posicionVariable->size = 4;
		variableStack->posicion = posicionVariable;

		pcbActual->pcb_sp->offset += 4;

	}
	else {

		posicionVariable->pagina = ((pcbActual->pcb_sp->pagina) + 1);    //pongo la variable en la pagina siguiente
		posicionVariable->offset = 0;
		posicionVariable->size = 4;
		variableStack->posicion = posicionVariable;
		pcbActual->pcb_sp->pagina += 1;									//pagina siguiente en el stack pointer
		pcbActual->pcb_sp->offset = 4;
	}


	//buscarStackActivo();
	//Agrego variableStack a stackActivo



	//mandar a umc escribirBytes()

	return * posicionVariable;
	}


t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable) {

	t_stack stackActivo;
	//BuscarStackActivo();
	//list_find(stackActivo->vars, identificador_variable);   //busco la variable por el identificador
	//retornar posicion de la variable
}

t_valor_variable dereferenciar(t_posicion direccion_variable) {

	//mandar a umc leerBytes(direccion_variable)

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

t_stack buscarStackActivo(){

	//list_find(pcbActual->indice_stack, stackActivo);
}
