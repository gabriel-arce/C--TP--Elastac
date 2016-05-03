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




	/*BEGIN CONECTION*/
	//int socket_CPU_nucleo = crearSocket();
	//int socket_CPU_UMC    = crearSocket();

	//conectarA(socket_CPU_nucleo, config->ip_nucleo, config->puerto_nucleo);
	//close(socket_CPU);

	//conectarA(socket_CPU_UMC, config->ip_UMC, config->puerto_UMC);
	//close(socket_CPU);

	/*END CONECTION*/





t_CPU_config *cargar_config() {

	printf(" Cargando configuracion.. \n");                                 /* Nunca sale el printf, como que se corta antes el programa*/
	t_config *config = config_create(CONFIG_PATH);
	t_CPU_config *cpu_config	= malloc(sizeof(t_CPU_config));

	cpu_config->ip_nucleo 					= string_new();
	cpu_config->ip_UMC						= string_new();

	cpu_config->puerto_nucleo = getIntProperty(config, "PUERTO_NUCLEO");
	cpu_config->puerto_nucleo = config_get_int_value(config, "PUERTO_NUCLEO");
	//cpu_config->puerto_UMC    = getIntProperty(config, "PUERTO_UMC");

	string_append(&cpu_config->ip_nucleo, getStringProperty(config, "IP_NUCLEO"));
	string_append(&cpu_config->ip_UMC, getStringProperty(config, "IP_UMC"));

	config_destroy(config);

	return cpu_config;
}
