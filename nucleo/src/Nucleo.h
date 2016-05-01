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
#include <stdint.h>
#include <commons/collections/list.h>
#include <elestac_config.h>
#include <signal.h>

#define CONFIG_NUCLEO	"../nucleo/src/nucleo.conf"
#define PUERTO_NUCLEO	7200

typedef struct {
	int puerto_programas;
	int puerto_cpu;
	int quantum;
	int quantum_sleep;
	t_list *sem_ids;
	t_list *sem_init;
	t_list *io_ids;
	t_list *io_sleep;
	t_list *shared_vars;
} t_nucleo;

typedef struct {
	unsigned int posicion;			//Posicion de comienzo
	unsigned int tamanio;			//Tamanio de instruccion
} t_indice;

typedef struct t_list_pcb{
	int pcb_pid;									//Identificador unico
	int pcb_pc;									//Program counter
	int pcb_sp;									//Stack pointer
	int paginas_codigo;					//Paginas del codigo
	t_indice indice_codigo;			//Indice del codigo
	int indice_etiquetas;					//Indice de etiquetas
	struct t_list_pcb *proximo;
} t_pcb;


t_nucleo *cargar_conf();
int get_quantum(t_nucleo *nucleo);
int get_quantum_sleep(t_nucleo *nucleo);
//t_list *obtener_lista(t_config *config, char *property);
//int string_count(char *text, char *pattern);

#endif /* NUCLEO_H_ */
