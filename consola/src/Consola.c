/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Consola.h"
#include <elestac_config.h>

/*int main(void) {

	t_consola_config * config = cargar_config();

	puts("PROCESO CONSOLA/n");
	printf("Programa ansisop : %s", config->programa_ansisop);

	BEGIN CONECTION
	//int socket_consola = crearSocket();

	//conectarA(socket_consola, config->ip_nucleo, config->puerto_nucleo);
	//close(socket_consola);
	END CONECTION


	return EXIT_SUCCESS;
}*/

t_consola_config *cargar_config() {
	t_config *config 									= config_create(CONFIG_PATH);
	t_consola_config *consola_config	= malloc(sizeof(t_consola_config));

	consola_config->ip_nucleo 					= string_new();
	consola_config->programa_ansisop	= string_new();

	consola_config->puerto_nucleo = getIntProperty(config, "PUERTO_NUCLEO");

	string_append(&consola_config->ip_nucleo, getStringProperty(config, "IP_NUCLEO"));
	string_append(&consola_config->programa_ansisop, getStringProperty(config, "PROGRAMA_ANSISOP"));

	config_destroy(config);

	return consola_config;
}
