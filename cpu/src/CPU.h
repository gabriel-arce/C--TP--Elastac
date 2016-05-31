/*
 * CPU.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <elestac_config.h>
#include <elestac_sockets.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <elestac_paquetes.h>

#define CONFIG_PATH "../cpu/src/CPU.conf"  			//Eclipse
//#define CONFIG_PATH "CPU.conf"					//Terminal

//------------------Estructuras

typedef struct {
	int puerto_nucleo;
	char * ip_nucleo;
	int puerto_UMC;
	char * ip_UMC;
} t_CPU_config;

typedef enum {
	Listo,
	Corriendo,
	Terminado,
	Bloqueado,
	Nuevo,
	FinQuantum,
} t_estado;

typedef struct{
	int pagina;
	int offset;
	int size;
} t_posicion;

typedef struct {
	t_list * args;   				  // (t_posicion)
	t_list * vars;    			 // (t_variable_stack)
	uint32_t retPos;
	t_posicion * retVar;
	bool stackActivo;
} t_stack;

typedef struct {
	t_nombre_variable  id;
	t_posicion * posicion;
}t_variable_stack;

typedef struct {
	uint32_t posicion;			//Posicion de comienzo
	uint32_t tamanio;			//Tamanio de instruccion
} t_indice_de_codigo;

typedef struct {
	char * nombre_funcion;
	uint32_t posicionPrimeraInstruccion;
}t_indice_de_etiquetas;

typedef struct{
	uint32_t pagina;
	uint32_t offset;
}t_sp;

typedef struct {
	uint32_t pcb_pid;									//Identificador unico
	uint32_t pcb_pc;									//Program counter
	t_sp * pcb_sp;										//Stack pointer
	uint32_t paginas_codigo;							//Paginas del codigo
	t_list * indice_codigo;								//Indice del codigo  (t_indice_de_codigo)                   //hay que ver como lo devuelve el metadata
	t_indice_de_etiquetas * indice_etiquetas;				//Indice de etiquetas				  						//idem
	t_list * indice_stack;								//Indice del Stack (t_stack)
	t_estado * estado;									//Codigo interno para ver los estados del pcb
	int consola;										//Consola
} t_pcb;




//-------------------Variables

t_pcb * pcbActual;					//PCB del programa ejecutando
int quantum;
int tamanio_paginas;
t_CPU_config *cpu;


//------------------Funciones

t_CPU_config * cargar_config();
void cargarConfiguracion();


//------------------Primitivas

t_posicion definirVariable(t_nombre_variable identificador_variable);
t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_posicion direccion_variable);
void asignar(t_posicion direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta);
t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar,t_puntero_instruccion linea_en_ejecuccion);
t_puntero_instruccion retornar(t_valor_variable retorno);
int imprimir(t_valor_variable valor_mostrar);
int imprimirTexto(char* texto);
int entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
int wait(t_nombre_semaforo identificador_semaforo);
int signal(t_nombre_semaforo identificador_semaforo);





#endif /* CPU_H_ */
