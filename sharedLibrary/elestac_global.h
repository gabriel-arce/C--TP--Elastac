/*
 * elestac_global.h
 *
 *  Created on: 10 de may. de 2016
 *      Author: gabriel
 */

#ifndef ELESTAC_GLOBAL_H_
#define ELESTAC_GLOBAL_H_

#include <string.h>

typedef enum{
	CantidadArgumentosIncorrecta,
	NoSePudoAbrirIn,
	NoSePudoCrearSocket,
	NoSePudoEnviarSocket,
	OtroError,
} Error;

void MostrarAyuda();
void MostrarError( Error );
char *concat(char *, char *);

#endif /* ELESTAC_GLOBAL_H_ */
