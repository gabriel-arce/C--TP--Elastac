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


t_nucleo *cargar_conf(){
	t_config *config = config_create(CONFIG);
	t_nucleo *nucleo = malloc(sizeof(t_nucleo));

	nucleo->sem_ids  	= list_create();
	nucleo->sem_init 	= list_create();
	nucleo->io_ids	 	= list_create();
	nucleo->io_sleep 	= list_create();
	nucleo->shared_vars = list_create();

	nucleo->puerto_programas = config_get_int_value(config, "PUERTO_PROG");
	nucleo->puerto_programas = config_get_int_value(config, "PUERTO_CPU");
	nucleo->puerto_programas = config_get_int_value(config, "QUANTUM");
	nucleo->puerto_programas = config_get_int_value(config, "QUANTUM_SLEEP");
	nucleo->puerto_programas = obtener_lista(config, "SEM_IDS");
	nucleo->puerto_programas = obtener_lista(config, "SEM_INIT");
	nucleo->io_ids = obtener_lista(config, "IO_IDS");
	nucleo->io_sleep = obtener_lista(config, "IO_SLEEP");
	nucleo->shared_vars = obtener_lista(config, "SHARED_VARS");

	config_destroy(config);

	return nucleo;
}

t_list *obtener_lista(t_config *config, char *property){
	char **items = config_get_array_value(config, property);
	t_list *ret  = list_create();
	int cant	 = string_count(config_get_string_value(config, property), ",") +1;

	int i = 0;

	for(i; i < cant; i++)
		list_add(ret, (void *)string_duplicate(items[i]));

	return ret;
}

int string_count(char *text, char *pattern){
	char **chunks = string_split(text, pattern);
	int ret;
	for(ret = 0; chunks[ret]; ret++);
	return ret - 1;
}

int get_quantum(t_nucleo *nucleo){
	return nucleo->quantum;
}

int get_quantum_sleep(t_nucleo *nucleo){
	return nucleo->quantum_sleep;
}
