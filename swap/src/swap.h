/*
 * swap.h
 *
 *  Created on: 29/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <elestac_config.h>
#include <elestac_sockets.h>
#include <elestac_comunicaciones.h>

t_list * bitmap;
t_list * list_swap;

typedef struct {
	int pid;
	int pagina;
	int posicion_en_swap;
} t_swap;

void inicializar_espacio_swap();
void inicializar_bitmap();
void * leer_bytes(int pid, t_paquete_solicitar_pagina * paquete);
int escribir_bytes(int pid, t_paquete_almacenar_pagina * paquete);
int calcular_direccion_fisica(int pid, int pagina, int offset);
int hay_espacios_libres_contiguos(int cantidad_necesaria);
bool contiguos_necesarios(int posicion_actual, int cantidad_necesaria);
void marcar_en_bitmap(int start, int end, int bit);
void crear_estructuras_control(int pid, int paginas_totales, int start);
int iniciar_programa(t_paquete_inicializar_programa * paquete_init,
		bool notyetcompacted);
void fin_programa(int pid);
int almacenar_codigo(char * codigo, int posicion_inicial_swap);
int finalizar_en_archivo(int pid);
int compactar();

#define Respuesta_inicio_SI 1
#define Respuesta_inicio_NO 0

static const int EXIT_ERROR = -1;

//~~~Interfaces~~~
#define ID_UMC 3
#define Inicializar_programa 14
#define Respuesta_inicio 17
#define Finalizar_programa 13
#define Solicitar_pagina 15
#define Almacenar_pagina 16
//~~~__________~~~

typedef struct {
	int puerto_escucha;
	char * espacio_swap;
	int cant_paginas;
	int pagina_size;
	int retardo_compactacion;
} t_swap_config;

t_swap_config * swap_config;
int socket_SWAP;
int socket_UMC;
pthread_mutex_t mutex_umc_recv;

void cargar_config(char ** config_path);
void imprimir_config();
void iniciar_servidor_UMC();
void escuchar_solicitudes_UMC();
void inicializar_semaforos();
void enviar_a_umc_respuesta_inicio(
		t_paquete_inicializar_programa * paquete_inicio_prog,
		int respuesta_inicio);
bool pagina_valida(int pid, int pagina, int offset, int bytes);

//*****OPERACIONES CON UMC*****
int inicializar_programa(int buffer_init_size);
int finalizar_programa(int pid);
int leer_pagina(int buffer_read_size);
int almacenar_pagina(int buffer_write_size);
//*****-------------------*****

//-----------------OUT OF BOUNDS---------------------------
void ver_que_onda();
void mostrar_bitmap();
void dump_memory(const void* data, size_t size);
void mostrar_estructuras_swap();
void signal_handler(int n_singal);

#endif /* SWAP_H_ */
