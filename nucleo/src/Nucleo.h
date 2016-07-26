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
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <elestac_config.h>
#include <signal.h>
#include <elestac_sockets.h>
#include <elestac_semaforos.h>
#include <pthread.h>
#include <elestac_pcb.h>
#include <elestac_comunicaciones.h>
#include <parser/metadata_program.h>

/****** Constantes ******/

#define CONFIG_NUCLEO	"nucleo.conf"
//#define CONFIG_NUCLEO	"../nucleo/src/nucleo.conf"
#define MAXIMO_BUFFER	2000
#define PUERTO_NUCLEO	7200
#define MAX_CLIENTES 10
#define ETIQUETA_NUCLEO	"[NUCLEO]"
#define HANDSHAKE					 "Hola! Soy nucleo!.."
#define SERIALIZADOR				"##"

//#define CONFIG_NUCLEO	"nucleo.conf"
#define CONFIG_NUCLEO	"../nucleo/src/nucleo.conf"
#define Tamanio_pagina				19
#define NOMBRE_SEMAFORO	20
#define NOMBRE_IO						20

//Interfaces
#define CONSOLA	1
#define NUCLEO 2
#define CPU	5
#define Tamanio_pagina 19
#define Inicializar_programa 14
#define Respuesta_inicio 17
#define Fin_programa 13


/****** Estructuras ******/
typedef enum {
	imprimir_variable = 11,
	imprimir_texto,
	FinalizacionPrograma = 21,
	Wait,
	Signal,
	EntradaSalida,
	ObtenerValorCompartido,
	AsignarValorCompartido,
	MuereCPU,
	MandarQuantum,
	MandarQuantumSleep,
	FinalizacionQuantum,
	SemaforoBloqueado,
	SemaforoNoBloqueado,
	abortarPrograma,
} t_accionesPCB;

typedef enum {
	No,
	Si,
} t_respuesta;

typedef struct {
	char *id;
	t_valor_variable valor;
} t_variableCompartida;

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
	char *id;
	sem_t *valor;
	int io_sleep;
	t_list *bloqueados;
} t_semNucleo;

typedef struct {
	char *id;
	uint32_t valorSleep;
} t_ioNucleo;

typedef struct {
	uint8_t fd;
	uint8_t cpuID;
	uint8_t disponible;
} t_clienteCPU;

typedef struct {
	t_pcb * pcb;
	t_semNucleo * semaforo;
	int tiempo;
}t_parametrosHiloBloqueados;

/****** Variables Globales ******/
t_nucleo *nucleo;
t_config  *config;
int socketUMC;
int tamanio_pagina;
int pid_global;

t_queue *cola_listos;
t_list	*lista_bloqueados;
//, *cola_ejecutando;

t_list *lista_ejecutando;
t_list *lista_cpu;
t_list *lista_finalizados;
t_list *lista_semaforos;
t_list *lista_io;
t_list *lista_sharedValues ;

sem_t *mutexListos;
sem_t *mutexCPU;
sem_t *mutexEjecutando;
sem_t *mutexFinalizados;
sem_t *semCpuDisponible;
sem_t *semListos;
sem_t *semBloqueados;
sem_t *semCPU;
sem_t *semFinalizados;
sem_t *mutexConsolas;
sem_t *mutex_pid;


pthread_t pIDServerNucleo;
pthread_t pIDServerConsola;
pthread_t pIDServerCPU;
pthread_t pIDProcesarMensaje;
pthread_t pIDPlanificador;
pthread_t hiloEjecucion;
pthread_t hiloBloqueado;
pthread_t pIDCpu;
pthread_t PiDBloqueado;

fd_set master;				// conjunto maestro de descriptores de fichero
fd_set read_fds;				// conjunto temporal de descriptores de fichero para select()

/****** Funciones ******/
void cargarConfiguracion(char ** config_path);
void crearListasYColas();
void crearSemaforos();
void crearServerConsola();
void crearServerCPU();
void crearClienteUMC();
void planificar_consolas();
void mainEjecucion();
void mainBloqueado();
void pasarAEjecutar();
void enviarAEjecutar(t_pcb *pcb, t_clienteCPU *cpu);
void entradaSalida();
void pasarAListos(t_pcb *pcb);
void destruirSemaforos();
void sacarDeEjecutar(t_pcb *pcb);
int obtenerCPUID();
t_clienteCPU *obtenerCPUDisponible();
int CPUestaDisponible(t_clienteCPU *cpu);
t_header * deserializar_header(void * buffer);
void finalizar();
t_paquete_programa *obtener_programa(t_header *header, int fd);
void accionesDeCPU(t_clienteCPU *cpu);
void agregarPCBaBloqueados(t_queue *cola, t_pcb *pcb, t_clienteCPU *cpu);
void agregarPCBaFinalizados(t_list *lista, t_pcb *pcb, t_clienteCPU *cpu);
char *getSemaforo(char *valor);
int getSemValue(char *valor);
t_semNucleo *crearSemaforoGlobal(char *semaforo, int valor, int io_sleep);
char *getIOId(char *valor);
int getIOSleep(char *valor);
t_ioNucleo *crearIOGlobal(nombre, valor);
t_semNucleo *obtenerSemaforoPorID(char *nombreSemaforo);
void ejecutarSignal(int tamanio_buffer, t_clienteCPU * cpu);
void ejecutarWait(int tamanio_buffer, t_clienteCPU *cpu);
//t_pcb *recibir_pcb(t_clienteCPU *cpu, uint32_t tamanio);
void ejecutarObtenerValorCompartido(int fd, int tamanio_buffer);
void ejecutarAsignarValorCompartido(int fd, int tamanio_buffer);
void ejecutarFinalizacionPrograma(t_clienteCPU *cpu, t_header *header);
void ejecutarEntradaSalida(t_clienteCPU *cpu,t_header * header);
void inicializar_programa(void * fd);
int calcular_cantidad_paginas(int codigo_length);
int generar_pid();
void ejecutarMuerteCPU(t_clienteCPU *cpu);
void  interrupcionConsola(int interrupcion);
void destruirCPU(t_clienteCPU *cpu);
void crearHiloBloqueados(t_pcb *pcb, t_semNucleo *semaforo);
void ejecutarImprimirTexto(int socket, int tamanio_buffer);
void ejecutarImprimirVariable(int socket, int tamanio_buffer);
void ejecutaFinalizacionDeQuantum(t_clienteCPU * cpu);
t_variableCompartida *crearSharedGlobal(char *sharedNombre);
char *getSharedValue(char *valor);

#endif /* NUCLEO_H_ */
