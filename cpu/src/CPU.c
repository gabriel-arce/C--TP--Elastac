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


int main(void) {

	t_CPU_config * config = cargar_config();

	//puts("PROCESO CPU/n");

	/*BEGIN CONECTION*/
	//int socket_CPU_nucleo = crearSocket();
	//int socket_CPU_UMC    = crearSocket();

	//conectarA(socket_CPU_nucleo, config->ip_nucleo, config->puerto_nucleo);
	//close(socket_CPU);

	//conectarA(socket_CPU_UMC, config->ip_UMC, config->puerto_UMC);
	//close(socket_CPU);

	/*END CONECTION*/

	return EXIT_SUCCESS;
}




t_CPU_config *cargar_config() {
	t_config *config = config_create(CONFIG_PATH);
	t_CPU_config *CPU_config = malloc(sizeof(t_CPU_config));

	if (chequearProperty(config, "PUERTO_NUCLEO"))
		CPU_config->puerto_nucleo = config_get_int_value(config, "PUERTO_NUCLEO");

	if (chequearProperty(config, "IP_NUCLEO"))
		CPU_config->ip_nucleo = config_get_string_value(config, "IP_NUCLEO");

	if (chequearProperty(config, "PROGRAMA_ANSISOP"))
		CPU_config->puerto_UMC = config_get_string_value(config, "PUERTO_UMC");

	if (chequearProperty(config, "IP_NUCLEO"))
		CPU_config->ip_UMC = config_get_string_value(config, "IP_UMC");

	config_destroy(config);

	return CPU_config;
}
