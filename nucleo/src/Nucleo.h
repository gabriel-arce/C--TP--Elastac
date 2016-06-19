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
#include <pthread.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <elestac_pcb.h>
#include <elestac_comunicaciones.h>

/****** Constantes ******/

//#define CONFIG_NUCLEO	"nucleo.conf"
#define CONFIG_NUCLEO	"../nucleo/src/nucleo.conf"
#define MAXIMO_BUFFER	2000
#define PUERTO_NUCLEO	7200
#define MAX_CLIENTES 10
#define ETIQUETA_NUCLEO	"[NUCLEO]"
#define HANDSHAKE					 "Hola! Soy nucleo!.."
#define SERIALIZADOR				"##"
#define Tamanio_pagina 31

#define CONSOLA	2
#define CPU				5


/****** Estructuras ******/
typedef struct {
	int puerto_programas;
	int puerto_cpu;
	int puerto_umc;
	int quantum;
	int quantum_sleep;
	t_list *sem_ids;
	t_list *sem_init;
	t_list *io_ids;
	t_list *io_sleep;
	t_list *shared_vars;
	char *ip_umc;
	int stack_size;
} t_nucleo;

typedef struct {
	uint8_t fd;
	uint8_t cpuID;
	uint8_t disponible;
} t_clienteCPU;

typedef struct {
	uint32_t programa_length;
	char * codigo_programa;
} t_paquete_programa;

/****** Variables Globales ******/
t_nucleo *nucleo;
t_config  *config;
int socketNucleo;
int tamanio_pagina;

t_queue *cola_listos;
t_queue	*cola_bloqueados;
//, *cola_ejecutando;

t_list *lista_ejecutando;
t_list *lista_cpu;
t_list *lista_finalizados;

sem_t *mutexListos;
sem_t *mutexCPU;
sem_t *mutexEjecutando;
sem_t *mutexFinalizados;
sem_t *semCpuDisponible;
sem_t *semListos;
sem_t *semBloqueados;
sem_t *semCPU;
sem_t *semFinalizados;


pthread_t pIDServerNucleo;
pthread_t pIDServerConsola;
pthread_t pIDServerCPU;
pthread_t pIDProcesarMensaje;
pthread_t pIDPlanificador;
pthread_t hiloEjecucion;
pthread_t hiloBloqueado;

fd_set master;				// conjunto maestro de descriptores de fichero
fd_set read_fds;				// conjunto temporal de descriptores de fichero para select()

/****** Funciones ******/
void cargarConfiguracion();
void crearListasYColas();
void crearSemaforos();
void crearServerNucleo();
void crearServerConsola();
void crearServerCPU();
void crearClienteUMC();
void hiloProcesarMensaje(char *datos);
void procesarMensaje(int fd, char *buffer);
void planificar_consolas();
void mainEjecucion();
void mainBloqueado();
void pasarAEjecutar();
t_pcb *enviarAEjecutar(t_pcb *pcb, int fd);
void entradaSalida();
void pasarAListos(t_pcb *pcb);
void destruirSemaforos();
void sacarDeEjecutar(t_pcb *pcb);
int obtenerCPUID();
t_clienteCPU *obtenerCPUDisponible();
int CPUestaDisponible(t_clienteCPU *cpu);
t_header * deserializar_header(void * buffer);
uint8_t recibirHandshakeConsola(void *buffer);
t_paquete_programa *recibirDatosConsola(int fd);
void finalizar();
t_header *recibirHeaderConsola(buffer);
void enviarHandshakeAUMC();
void recibirHandshakeDeUMC();
//void recibirDatosConsola(buffer);

#endif /* NUCLEO_H_ */
