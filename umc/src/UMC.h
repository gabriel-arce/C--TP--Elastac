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
#include <elestac_paquetes.h>

#define CONFIG_PATH "umc.conf" //para runear en terminal
//#define CONFIG_PATH "../umc/src/umc.conf"  //para runear en eclipse
//#define REPORT_PATH "reporte.txt"
#define REPORT_PATH "../umc/src/reporte.txt"
#define LOG_FILE "../umc/src/umc.log"
#define MSJ_ERROR1 "Error en sintaxis del comando\n"
#define MSJ_ERROR2 "Error en # de variable"
#define PROMPT "#>"
#define LENGTH_MAX_COMANDO 7
#define MENSAJE_HANDSHAKE "Hola soy umc \n"

typedef struct {
	int puerto_escucha;
	int puerto_swap;
	char * ip_swap;
	int cant_frames;
	int frames_size;
	int frame_x_prog;
	int entradas_tlb;
	int retardo;
} t_umc_config;

typedef struct {
	int socket_nucleo;
} t_sesion_nucleo;

typedef struct {
	int socket_cpu;
	int id_cpu;
} t_sesion_cpu;

typedef struct {
	int nro_frame;
	int pagina;
	int pid;
	int libre;
} t_mem_frame;

t_umc_config * umc_config;
int contador_hilos;
int socket_cliente, socket_servidor;
pthread_t hiloConsola, hilo_server, hilo_cliente;
int id_cpu;
t_list * cpu_conectadas;
pthread_mutex_t mutex_hilos, mutex_lista_cpu;
int memoria_size;
char * memoria_principal;
t_log * logger;
FILE * archivo_reporte;
t_list * tlb;
t_list * lista_frames;
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
void reporte_estructuras(char * arg);
void reporte_contenido(char * arg);
void limpiar_tlb();
void marcar_paginas();
// begin OPERACIONES PRINCIPALES
void * inicializar_programa(int id_programa, int paginas_requeridas);
void * solicitar_bytes(int nro_pagina, int offset, int tamanio); //cuidado que devuelve algo!!
void * almacenar_bytes(int nro_pagina, int offset, int tamanio, char * buffer);
void * finalizar_programa(int id_programa);
// end OPERACIONES PRINCIPALES

#endif /* UMC_H_ */
