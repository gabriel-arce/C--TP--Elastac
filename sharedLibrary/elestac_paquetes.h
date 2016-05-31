/*
 * elestac_paquetes.h
 *
 *  Created on: 10/5/2016
 *      Author: utnso
 */

#ifndef ELESTAC_PAQUETES_H_
#define ELESTAC_PAQUETES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <parser/parser.h>

typedef enum {
	Listo,
	Corriendo,
	Terminado,
	Bloqueado,
	Nuevo,
	FinQuantum,
} t_estado;

typedef struct {
	uint8_t identificador;
	uint32_t tamanio;
}__attribute__((packed)) t_header;

typedef struct {
	uint8_t posicion;			//Posicion de comienzo
	uint8_t tamanio;			//Tamanio de instruccion
} t_indice;

typedef struct {
	char **args;
	t_list vars;
	uint8_t retPos;
	t_size retVar;
} t_stack;

typedef struct {
	uint8_t pcb_pid;									//Identificador unico
	uint8_t pcb_pc;									//Program counter
	uint8_t pcb_sp;									//Stack pointer
	uint8_t paginas_codigo;					//Paginas del codigo
	t_indice indice_codigo;					//Indice del codigo
	char * indice_etiquetas;					//Indice de etiquetas
	t_stack indice_stack;							//Indice del Stack
	int quantum;										//quantum - privado
	t_estado estado;								//Codigo interno para ver los estados del pcb
	int consola;											//Consola
} t_pcb;

t_header * recibir_handshake(int sock_fd, t_header * handshake_in);
int enviar_handshake(int sock_fd, uint8_t id_proc, uint32_t size);
void * serializar_header(t_header * header);
t_header * deserializar_header(void * buffer);
int get_pcb_size(t_pcb * pcb);
void * serializar_pcb(t_pcb * pcb);
t_pcb * deserializar_pcb(void * buffer);

#endif /* ELESTAC_PAQUETES_H_ */
