/*
 * Nucleo.h
 *
 *  Created on: 20 de abr. de 2016
 *      Author: gabriel
 */

#ifndef NUCLEO_H_
#define NUCLEO_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

#define CONFIG "../nucleo.conf"

typedef struct {
	int puerto_programas;
	int puerto_cpu;
	int quantum;
	int quantum_sleep;
	char **sem_ids;
	char **sem_init;
	char **io_ids;
	char **io_sleep;
	char **shared_vars;
} t_nucleo;

void cargar_conf(t_nucleo *nucleo);
int get_quantum(t_nucleo *nucleo);
int get_quantum_sleep(t_nucleo *nucleo);


#endif /* NUCLEO_H_ */
