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
#include "elestac_config.h"

t_nucleo *cargar_conf(){
	t_config *config = config_create(CONFIG);
	t_nucleo *nucleo = malloc(sizeof(t_nucleo));

	nucleo->sem_ids  	= list_create();
	nucleo->sem_init 	= list_create();
	nucleo->io_ids	 	= list_create();
	nucleo->io_sleep 	= list_create();
	nucleo->shared_vars = list_create();

	if(chequearProperty(config, "PUERTO_PROG"))
		nucleo->puerto_programas = getIntProperty(config, "PUERTO_PROG");

	if(chequearProperty(config, "PUERTO_CPU"))
		nucleo->puerto_cpu = getIntProperty(config,"PUERTO_CPU");

	if(chequearProperty(config, "QUANTUM"))
		nucleo->quantum = getIntProperty(config, "QUANTUM");

	if(chequearProperty(config, "QUANTUM_SLEEP"))
		nucleo->quantum_sleep = getIntProperty(config, "QUANTUM_SLEEP");

	if(chequearProperty(config, "SEM_IDS"))
		nucleo->sem_ids = getListProperty(config, "SEM_IDS");

	if(chequearProperty(config, "SEM_INIT"))
		nucleo->sem_init = getListProperty(config, "SEM_INIT");

	if(chequearProperty(config, "IO_IDS"))
		nucleo->io_ids = getListProperty(config, "IO_IDS");

	if(chequearProperty(config, "IO_SLEEP"))
		nucleo->io_sleep = getListProperty(config, "IO_SLEEP");

	if(chequearProperty(config, "SHARED_VARS"))
		nucleo->shared_vars = getListProperty(config, "SHARED_VARS");

	config_destroy(config);

	return nucleo;
}

int get_quantum(t_nucleo *nucleo){
	return nucleo->quantum;
}

int get_quantum_sleep(t_nucleo *nucleo){
	return nucleo->quantum_sleep;
}
