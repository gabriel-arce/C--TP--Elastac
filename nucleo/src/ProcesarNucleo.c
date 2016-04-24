/*
 * ProcesarNucleo.c
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#include <sharedLibrary/elestac_sockets.h>

#include "Nucleo.h"

int main(void) {
	t_nucleo *nucleo = cargar_conf();
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
