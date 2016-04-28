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
	t_config *config = config_create(CONFIG_PATH);
	t_consola_config *consola_config = malloc(sizeof(t_consola_config));

	if (chequearProperty(config, "PUERTO_NUCLEO"))
		consola_config->puerto_nucleo = config_get_int_value(config, "PUERTO_NUCLEO");

	if (chequearProperty(config, "IP_NUCLEO"))
		consola_config->ip_nucleo = config_get_string_value(config, "IP_NUCLEO");

	if (chequearProperty(config, "PROGRAMA_ANSISOP"))
		consola_config->programa_ansisop = config_get_string_value(config, "PROGRAMA_ANSISOP");

	config_destroy(config);

	return consola_config;
}
