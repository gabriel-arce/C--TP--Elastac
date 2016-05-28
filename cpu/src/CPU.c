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
	//cpu_config->puerto_nucleo = config_get_int_value(config, "PUERTO_NUCLEO");       // ¿a alguien le sirve esto? yo no lo puse
	cpu_config->puerto_UMC    = getIntProperty(config, "PUERTO_UMC");

	string_append(&cpu_config->ip_nucleo, getStringProperty(config, "IP_NUCLEO"));
	string_append(&cpu_config->ip_UMC, getStringProperty(config, "IP_UMC"));

	config_destroy(config);

	return cpu_config;
}

t_posicion definirVariable(t_nombre_variable identificador_variable) {

}

t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable) {

}

t_valor_variable dereferenciar(t_posicion direccion_variable) {

}

void asignar(t_posicion direccion_variable, t_valor_variable valor) {

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

}

t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta){

}
t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar,t_puntero_instruccion linea_en_ejecuccion){

}
t_puntero_instruccion retornar(t_valor_variable retorno){

}

int imprimir(t_valor_variable valor_mostrar){

}

int imprimirTexto(char* texto){

}

int entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

}

int wait(t_nombre_semaforo identificador_semaforo){

}

int signal(t_nombre_semaforo identificador_semaforo){

}
