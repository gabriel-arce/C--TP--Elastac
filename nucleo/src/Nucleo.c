/*
 ============================================================================
 Name        : Nucleo.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Nucleo.h"

void cargar_conf(){
	t_config *config = config_create(CONFIG);
	t_nucleo *nucleo = malloc(sizeof(t_nucleo));

//	nucleo->puerto_cpu = config_get_int_value(config, "PUERTO_CPU");

	if(config_has_property(config, "PUERTO_PROG"))
		nucleo->puerto_programas = config_get_int_value(config,"PUERTO_PROG");

	if(config_has_property(config, "PUERTO_CPU"))
		nucleo->puerto_cpu = config_get_int_value(config,"PUERTO_CPU");

	if(config_has_property(config, "QUANTUM"))
		nucleo->quantum = config_get_int_value(config, "QUANTUM");

	if(config_has_property(config, "QUANTUM_SLEEP"))
		nucleo->quantum_sleep = config_get_int_value(config, "QUANTUM_SLEEP");

	nucleo->sem_ids  = list_create();
	nucleo->sem_init = list_create();
	nucleo->io_ids	 = list_create();
	nucleo->io_sleep = list_create();
	nucleo->shared_vars = list_create();

	if(config_has_property(config, "SEM_IDS"))
		nucleo->sem_ids = config_get_array_value(config, "SEM_IDS");

	config_destroy(config);
}

int get_quantum(t_nucleo *nucleo){
	return nucleo->quantum;
}

int get_quantum_sleep(t_nucleo *nucleo){
	return nucleo->quantum_sleep;
}

