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
	string_append(&swap_config->espacio_swap, getStringProperty(swap_config, "NOMBRE_SWAP"));

	swap_config->puerto_escucha = getIntProperty(swap_config, "PUERTO_ESCUCHA");
	swap_config->cant_paginas = getIntProperty(swap_config, "CANTIDAD_PAGINAS");
	swap_config->pagina_size = getIntProperty(swap_config, "TAMANIO_PAGINA");
	swap_config->retardo_compactacion = getIntProperty(swap_config, "RETARDO_COMPACTACION");

	config_destroy(config_file);
}
