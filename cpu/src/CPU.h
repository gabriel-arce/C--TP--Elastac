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
#include <elestac_pcb.h>
#include <signal.h>

#define CONFIG_PATH "../cpu/src/CPU.conf"  			//Eclipse
//#define CONFIG_PATH "CPU.conf"					//Terminal
#define MAXIMO_BUFFER 2000
#define SERIALIZADOR				"##"

//------------------Estructuras

typedef struct {
	int puerto_nucleo;
	char * ip_nucleo;
	int puerto_UMC;
	char * ip_UMC;
} t_CPU_config;

typedef struct {
	uint8_t identificador;
	uint32_t tamanio;
}__attribute__((packed)) t_header;

//-------------------Variables

t_pcb * pcbActual;					//PCB del programa ejecutando
int quantum;
int tamanio_paginas;
t_CPU_config * cpu;
int socketNucleo, socketUMC;
fd_set master;				// conjunto maestro de descriptores de fichero
fd_set read_fds;			// conjunto temporal de descriptores de fichero para select()
bool hotPlugActivado;

//------------------Funciones

t_CPU_config * cargar_config();
void cargarConfiguracion();
void conectarConNucleo();
void conectarConUMC();
void escucharAlNucleo();
void recibirPCB(char *buffer);
t_stack * buscarStackActivo();
void crearStackInicial();
t_variable_stack * buscarVariableEnStack(t_nombre_variable  id);
int stackActivo(t_stack * stack);
int getQuantumPcb();
int getQuantum();
void ejecutarProximaInstruccion();
t_indice_de_codigo * buscarProximaInstruccion();
bool pcbCorriendo();
void restaurarQuantum();
void actualizarQuantum();
void enviarPCB();
void cambiarEstadoACorriendo();
void cambiarEstadoAFinQuantum();
void cambiarEstadoATerminado();
void actualizarPC();
void borrarPCBActual();
void escribirBytes(uint32_t pagina, uint32_t offset, uint32_t size, t_valor_variable valorVariable);
t_valor_variable leerBytes(uint32_t pagina, uint32_t offset, uint32_t size);
char* obtenerInstruccion(t_indice_de_codigo * instruccionACorrer);
void mandarTextoANucleo(char* texto);
void rutina(int n);
void desconectarCPU();


//------------------Primitivas

t_posicion definirVariable(t_nombre_variable identificador_variable);
t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_posicion direccion_variable);
void asignar(t_posicion direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor);
t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta);
t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta,t_posicion donde_retornar, t_puntero_instruccion linea_en_ejecuccion);
t_puntero_instruccion retornar(t_valor_variable retorno);
int imprimir(t_valor_variable valor_mostrar);
int imprimirTexto(char* texto);
int entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
int wait(t_nombre_semaforo identificador_semaforo);
int signals(t_nombre_semaforo identificador_semaforo);

#endif /* CPU_H_ */
