/*
 ============================================================================
 Name        : SWAP.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <swap.h>

void cargar_config() {

	t_config * config_file = config_create(CONFIG_PATH);

	swap_config = malloc(sizeof(t_swap_config));
	memset(swap_config, 0 , sizeof(t_swap_config));

	swap_config->espacio_swap = malloc(sizeof(char) * 10);
	swap_config->espacio_swap = string_new();
	string_append(&swap_config->espacio_swap, getStringProperty(config_file, "NOMBRE_SWAP"));

	swap_config->puerto_escucha = getIntProperty(config_file, "PUERTO_ESCUCHA");
	swap_config->cant_paginas = getIntProperty(config_file, "CANTIDAD_PAGINAS");
	swap_config->pagina_size = getIntProperty(config_file, "TAMANIO_PAGINA");
	swap_config->retardo_compactacion = getIntProperty(config_file, "RETARDO_COMPACTACION");

	config_destroy(config_file);
}

void imprimir_config() {
	printf("PUERTO_ESCUCHA: %d\n", swap_config->puerto_escucha);
	printf("NOMBRE_SWAP: %s\n", swap_config->espacio_swap);
	printf("CANTIDAD_PAGINAS: %d\n", swap_config->cant_paginas);
	printf("TAMANIO_PAGINA: %d\n", swap_config->pagina_size);
	printf("RETARDO_COMPACTACION: %d\n", swap_config->retardo_compactacion);
}
