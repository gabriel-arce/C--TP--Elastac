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
#include "ADM.h"

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
#define Respuesta__SI 1
#define Respuesta__NO 0

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

t_umc_config * umc_config;
int nucleos_conectados;
int socket_nucleo;
int socket_cliente, socket_servidor;
pthread_t hiloConsola, hilo_server, hilo_cliente;
int id_cpu;
t_list * cpu_conectadas;
pthread_mutex_t mutex_servidor;
pthread_mutex_t mutex_lista_cpu;
pthread_mutex_t mutex_nucleo;
pthread_mutex_t mutex_memoria;
int memoria_size;
void * memoria_principal;
t_log * logger;
bool tlb_on;

void new_line();
bool tlb_habilitada();
void enviar_pagina_size(int sock_fd);
int cambio_proceso_activo(int pid, int cpu);
int inicializar_en_swap(t_paquete_inicializar_programa * paquete);
void finalizar_en_swap(int pid);
void flush_tlb_by_certain_pid(int pid);

//INTERFAZ DE UMC
void inicializar_programa(t_paquete_inicializar_programa * paquete);
int leer_bytes(int socket_cpu, int bytes);
int almacenar_bytes(int socket_cpu, int bytes);
void finalizar_programa(int id_programa);

//MANEJO DE ARCHIVOS Y LOGS
void cargar_config();
void imprimir_config();
void crear_archivo_reporte();
void crear_archivo_log();
void * atiende_nucleo();
void * atiende_cpu();

//FUNCIONES DE LA CONSOLA
int no_es_comando();
void modificar_retardo(int ret);
void reporte_estructuras();
void reporte_contenido();
void flush_tlb();
void flush_memory(int pid);

//FUNCIONES CON HILOS
void * lanzar_consola();
void * escucha_conexiones();
void * conecta_swap();
void inicializar_semaforos();

#endif /* UMC_H_ */
