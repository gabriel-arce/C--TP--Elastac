/*
 * elestac_global.c
 *
 *  Created on: 10 de may. de 2016
 *      Author: gabriel
 */

#include "elestac_global.h"

void MostrarAyuda(){
	puts("Ayuda");
	}

void MostrarMensajeDeError(char *proceso, Error e){

	switch(e){
		case CantidadArgumentosIncorrecta: {
			puts(concat(proceso, "Cantidad de Argumentos Incorrecta.\n"));
			break;
		}

	  case NoSePudoAbrirIn: {
		  puts(concat(proceso, "No Se Pudo Abrir Archivo Script.\n"));
		  break;
	   }

	  case NoSePudoCrearSocket: {
		  puts(concat(proceso, "No Se Pudo Crear Socket.\n"));
		  break;
	  }

	  case NoSePudoEnviarSocket: {
		  puts(concat(proceso, "No Se Pudo Enviar Socket Al Nucleo.\n"));
		  break;
	  }

	  case OtroError: {
		  puts(concat(proceso,	"Error Desconocido.\n"));
		  break;
	  }
	}

}

char *concat(char *s1, char *s2){
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char *result = malloc(len1 + len2 + 1);

	memcpy(result, s1, len1);
	memcpy(result, s2, len2);

	return result;
}
