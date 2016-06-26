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
#include <elestac_comunicaciones.h>
#include <time.h>

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

//-------------------Variables

t_pcb * pcbActual;					//PCB del programa ejecutando
int quantum;
int quantum_sleep;
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
void cambiar_proceso_activo(int pid);
void escucharAlNucleo();
void recibirPCB(char *buffer);
t_stack * buscarStackActivo();
void crearStack();
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
void desactivarStackActivo();
void asignarPosicionYDireccionDeRetorno(t_posicion donde_retornar, uint32_t posicionDeRetorno);
void retornarValorAVariable(t_valor_variable retorno);
void modificarElPC();
void eliminarStackActivo();
void activarUltimoStack();
void cambiarEstadoABloqueado();
t_posicion  convertirPunteroAPosicion(t_puntero puntero);
t_puntero  convertirPosicionAPuntero(t_posicion * posicion);

//------------------Primitivas

t_posicion definirVariable(t_nombre_variable identificador_variable);
t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_posicion direccion_variable);
void asignar(t_posicion direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_posicion donde_retornar);
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signals(t_nombre_semaforo identificador_semaforo);

#endif /* CPU_H_ */
