/*
 * UMC.h
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#ifndef UMC_H_
#define UMC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/log.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <elestac_config.h>
#include <elestac_sockets.h>
#include <elestac_semaforos.h>
#include <elestac_comunicaciones.h>

#define CONFIG_PATH "umc.conf" //para runear en terminal
//#define CONFIG_PATH "../umc/src/umc.conf"  //para runear en eclipse
//#define REPORT_PATH "reporte.txt"
#define REPORT_PATH "../umc/src/reporte.txt"
#define LOG_FILE "../umc/src/umc.log"
#define MSJ_ERROR1 "Error en sintaxis del comando\n"
#define MSJ_ERROR2 "Error en # de variable"
#define PROMPT "#>"
#define LENGTH_MAX_COMANDO 7
#define CLOCK 99 //el condigo ascii de 'c' es 99
#define CLOCK_MODIFICADO 100 //aca le "sumo" 1 a c

//Cabeceras
#define NUCLEO 2
#define UMC 3
#define SWAP 4
#define CPU 5

#define Tamanio_pagina 19
#define Inicializar_programa 14
#define Solicitar_pagina 15
#define Almacenar_pagina 16
#define Finalizar_programa 13
#define Cambio_proceso_activo 18
#define Respuesta_inicio_programa 17

typedef struct {
	int puerto_escucha;
	int puerto_swap;
	char * ip_swap;
	int cant_frames;
	int frames_size;
	int frame_x_prog;
	int entradas_tlb;
	int retardo;
	int algoritmo;
} t_umc_config;

typedef struct {
	int socket_cpu;
	int id_cpu;
	int proceso_activo;
} t_sesion_cpu;

typedef struct {
	int nro_frame;
	int pagina;
	int pid;
	int libre;
} t_mem_frame;

typedef struct {
	int pagina;
	int frame;
	int pid;
	int dirtybit;
} t_pagina;

typedef struct {
	int pagina;
	int frame;
	int pid;
} t_tlb;

t_umc_config * umc_config;
int nucleos_conectados;
int socket_nucleo;
int socket_cliente, socket_servidor;
pthread_t hiloConsola, hilo_server, hilo_cliente;
int id_cpu;
t_list * cpu_conectadas;
pthread_mutex_t mutex_servidor;
pthread_mutex_t mutex_hilos, mutex_lista_cpu;
pthread_mutex_t mutex_nucleo;
pthread_mutex_t mutex_memoria;
int memoria_size;
void * memoria_principal;
t_log * logger;
FILE * archivo_reporte;
t_list * tlb;
t_list * marcos_memoria;
t_list * tabla_de_paginas;

void new_line();
void cargar_config();
void imprimir_config();
int tlb_habilitada();
void inicializar_memoria();
void crear_archivo_reporte();
void crear_archivo_log();
void * lanzar_consola();
void * escucha_conexiones();
void * conecta_swap();
int no_es_comando();
void modificar_retardo(int ret);
void reporte_estructuras();
void reporte_contenido();
void limpiar_tlb();
void marcar_paginas();
void enviar_pagina_size(int sock_fd);
// begin OPERACIONES PRINCIPALES
void * inicializar_programa(int id_programa, int paginas_requeridas, char * codigo);
void * solicitar_bytes(int nro_pagina, int offset, int tamanio); //cuidado que devuelve algo!!
void * almacenar_bytes(int nro_pagina, int offset, int tamanio, char * buffer);
void * finalizar_programa(int id_programa);
// end OPERACIONES PRINCIPALES
void * atiende_nucleo();
void * atende_cpu();
int pedir_espacio_swap(int pid, int paginas_necesarias);

#endif /* UMC_H_ */
