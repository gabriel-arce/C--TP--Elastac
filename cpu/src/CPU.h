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

//#define CONFIG_PATH "../cpu/src/CPU.conf"
#define CONFIG_PATH "CPU.conf"

//------------------Estructuras

typedef struct {
	int puerto_nucleo;
	char * ip_nucleo;
	int puerto_UMC;
	char * ip_UMC;
	int quantum;
} t_CPU_config;

typedef struct{
	int pagina;
	int offset;
	int size;
} t_posicion;

//------------------Funciones

t_CPU_config * cargar_config();


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
