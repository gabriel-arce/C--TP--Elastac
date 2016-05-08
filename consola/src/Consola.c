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

void cargar_config() {
	t_config *config	= config_create(CONFIG_PATH);
	consola					= malloc(sizeof(t_consola_config));

	consola->ip_nucleo						= malloc(sizeof(char) * 15);
	consola->programa_ansisop		= malloc(sizeof(char) * 15);

	consola->ip_nucleo 					= string_new();
	consola->programa_ansisop	= string_new();

	consola->puerto_nucleo = getIntProperty(config, "PUERTO_NUCLEO");

	string_append(&consola->ip_nucleo, getStringProperty(config, "IP_NUCLEO"));
	string_append(&consola->programa_ansisop, getStringProperty(config, "PROGRAMA_ANSISOP"));

	config_destroy(config);

}

void MostrarAyuda(){
	puts("Ayuda");
	}

void MostrarMensajeDeError(Error e){
	switch(e){
		case CantidadArgumentosIncorrecta: {
			puts("[CONSOLA] Cantidad de Argumentos Incorrecta.\n");
			break;
		}

	  case NoSePudoAbrirIn: {
		  puts("[CONSOLA] No Se Pudo Abrir Archivo Script.\n");
		  break;
	   }

	  case NoSePudoCrearSocket: {
		  puts("[CONSOLA] No Se Pudo Crear Socket.\n");
		  break;
	  }

	  case NoSePudoEnviarSocket: {
		  puts("[CONSOLA] No Se Pudo Enviar Socket Al Nucleo.\n");
		  break;
	  }

	  case OtroError: {
		  puts("[CONSOLA] Error Desconocido.\n");
		  break;
	  }
	}
}

