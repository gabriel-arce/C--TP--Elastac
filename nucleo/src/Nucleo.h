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
#include <commons/collections/queue.h>
#include <elestac_config.h>
#include <signal.h>
#include <elestac_sockets.h>
#include <elestac_semaforos.h>

//#define CONFIG_NUCLEO	"nucleo.conf"
#define CONFIG_NUCLEO	"../nucleo/src/nucleo.conf"
#define MAXIMO_BUFFER	2000
#define PUERTO_NUCLEO	7200
#define MAX_CLIENTES 10
#define PROCESO_CONSOLA		0
#define PROCESO_CPU					1

typedef enum {
	Listo,
	Corriendo,
	Ejecutando,
	Terminado,
} t_estado;

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

typedef struct {
	int pcb_pid;									//Identificador unico
	int pcb_pc;									//Program counter
	int pcb_sp;									//Stack pointer
	int paginas_codigo;					//Paginas del codigo
	t_indice indice_codigo;			//Indice del codigo
	int indice_etiquetas;					//Indice de etiquetas
} t_pcb;

t_nucleo *nucleo;
t_config  *config;

t_queue *cola_pcb, *cola_listos, *cola_bloqueados, *cola_ejecutando;
sem_t *mutex;

void *cargar_conf();
int get_quantum(t_nucleo *nucleo);
int get_quantum_sleep(t_nucleo *nucleo);
void escuchar_procesos();
void planificar_procesos();

t_pcb *crear_lista_pcb();
void destruir_pcb(t_pcb *pcb);

#endif /* NUCLEO_H_ */
