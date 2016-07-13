/*
 * elestac_pcb.h
 *
 *  Created on: 31 de may. de 2016
 *      Author: gabriel
 */

#ifndef ELESTAC_PCB_H_
#define ELESTAC_PCB_H_

#include <parser/metadata_program.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERIALIZADOR				"#"

typedef struct {
	uint32_t posicion;			//Posicion de comienzo
	uint32_t tamanio;			//Tamanio de instruccion
} t_indice_de_codigo;

typedef struct {
	int pagina;
	int offset;
	int size;
} t_posicion;

typedef struct {
	t_posicion * posicion;
	t_nombre_variable id;
} t_variable_stack;

typedef struct {
	t_list * args;   				  // (t_posicion) -> tipo del dato que contiene la lista
	t_list * vars;					  // (t_variable_stack)
	uint32_t retPos;
	t_posicion * retVar;
	bool stackActivo;
} t_stack;

typedef struct {
	uint32_t pagina;
	uint32_t offset;
} t_sp;

typedef struct {
	uint32_t pcb_pid;												//Identificador unico
	uint32_t pcb_pc;													//Program counter
	t_sp * pcb_sp;														//Stack pointer
	uint32_t paginas_codigo;								//Paginas del codigo
	t_list * indice_codigo;										//Indice del codigo   (t_indice_de_codigo)
	char* indice_etiquetas;									//Indice de etiquetas (se busca con metadata_buscar_etiqueta)
	uint32_t cantidad_de_etiquetas;
	t_list * indice_stack;											//Indice del Stack	  (t_stack)
	uint32_t consola;												//Consola
	uint32_t quantum_actual;								//Numero de instrucciones ejecutadas en la rafaga actual
} t_pcb;

t_pcb *crearPCB(char *programa, int fd,  uint8_t stack_size, t_queue *cola_pcb);
void destruirPCB(t_pcb *pcb);
char* serializarPCB(t_pcb* pcb);  //discontinuo
t_pcb *convertirPCB(char *mensaje);   //discontinuo
int crearPCBID(t_queue *cola_pcb);
void salirPor(const char *msg);
t_indice_de_codigo *crearIndiceCodigo(int start, int offset);

//serializacion del PCB
void imprimir_pcb(t_pcb * pcb);
int calcular_size_pcb(t_pcb * pcb);
void * serializar_pcb(t_pcb * pcb);
t_pcb * deserializar_pcb(void * buffer);
void enviar_pcb(t_pcb * pcb, int socket);
t_pcb * recibir_pcb(int socket, int bytes_a_recibir);


#endif /* ELESTAC_PCB_H_ */
