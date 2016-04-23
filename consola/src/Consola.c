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

int main(void) {

	t_consola_config *config = cargar_config();

	/*BEGIN CONECTION*/
	struct sockaddr_in direccion_nucleo;
	direccion_nucleo.sin_family = AF_INET;
	direccion_nucleo.sin_addr.s_addr = inet_addr(config.ip_nucleo);
	direccion_nucleo.sin_port = htons(config.puerto_nucleo);

	int sock_consola = socket(AF_INET, SOCK_STREAM, 0);

	//caso de fallo de conexion
	if (connect(sock_consola, (void*) &direccion_nucleo, sizeof(direccion_nucleo)) != 0) {
		perror("No se pudo conectar con el proceso NUCLEO");
		return 1;
	}
	/*END CONECTION*/


	return EXIT_SUCCESS;
}

t_consola_config *cargar_config() {
	t_config *config = config_create(CONFIG_PATH);
	t_consola_config *consola_config = malloc(sizeof(t_consola_config));

	if (config_has_property(config, "PUERTO_NUCLEO"))
		consola_config->puerto_nucleo = config_get_int_value(config, "PUERTO_NUCLEO");

	if (config_has_property(config, "IP_NUCLEO"))
		consola_config->ip_nucleo = config_get_string_value(config, "IP_NUCLEO");

	if (config_has_property(config, "PROGRAMA_ANSISOP"))
		consola_config->programa_ansisop = config_get_string_value(config, "PROGRAMA_ANSISOP");

	config_destroy(config);

	return consola_config;
}
