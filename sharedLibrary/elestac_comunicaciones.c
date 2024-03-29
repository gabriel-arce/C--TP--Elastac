/*
 * elestac_comunicaciones.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "elestac_comunicaciones.h"

void * serializar_header(uint8_t id, uint32_t size) {
	t_header * header = malloc(sizeof(t_header));
	header->identificador = id;
	header->tamanio = size;

	void * buffer = malloc(5);
	memcpy(buffer, &(header->identificador), 1);
	memcpy(buffer + 1, &(header->tamanio), 4);

	free(header);

	return buffer;
}

t_header * deserializar_header(void * buffer) {
	t_header * header = malloc(sizeof(t_header));

	memcpy(&(header->identificador), buffer, 1);
	memcpy(&(header->tamanio), buffer + 1, 4);

	free(buffer);

	return header;
}
//----------------------------------------------------->
void * serializar_iniciar_prog(int pid, int paginas, char * codigo) {

	int n = 0;
	n = 12 + strlen(codigo);

	void * buffer = malloc(n);

	t_paquete_inicializar_programa * paquete = malloc(
			sizeof(t_paquete_inicializar_programa));
	paquete->pid = pid;
	paquete->paginas_requeridas = paginas;
	paquete->programa_length = strlen(codigo);

	memcpy(buffer, &(paquete->pid), 4);
	memcpy(buffer + 4, &(paquete->paginas_requeridas), 4);
	memcpy(buffer + 8, &(paquete->programa_length), 4);
	memcpy(buffer + 12, codigo, strlen(codigo));

	free(paquete);

	return buffer;
}

t_paquete_inicializar_programa * deserializar_iniciar_prog(void * buffer) {
	t_paquete_inicializar_programa * paquete = malloc(
			sizeof(t_paquete_inicializar_programa));

	memcpy(&(paquete->pid), buffer, 4);
	memcpy(&(paquete->paginas_requeridas), buffer + 4, 4);
	memcpy(&(paquete->programa_length), buffer + 8, 4);
	paquete->codigo_programa = malloc(paquete->programa_length);
	memcpy(paquete->codigo_programa, buffer + 12, paquete->programa_length);

	free(buffer);

	return paquete;
}
//----------------------------------------------------->
void * serializar_leer_pagina(int pagina, int offset, int bytes) {
	t_paquete_solicitar_pagina * paquete = malloc(
			sizeof(t_paquete_solicitar_pagina));
	void * buffer = malloc(12);

	paquete->nro_pagina = pagina;
	paquete->offset = offset;
	paquete->bytes = bytes;

	memcpy(buffer, &(paquete->nro_pagina), 4);
	memcpy(buffer + 4, &(paquete->offset), 4);
	memcpy(buffer + 8, &(paquete->bytes), 4);

	free(paquete);
	return buffer;
}

t_paquete_solicitar_pagina * deserializar_leer_pagina(void * buffer) {
	t_paquete_solicitar_pagina * paquete = malloc(
			sizeof(t_paquete_solicitar_pagina));

	memcpy(&(paquete->nro_pagina), buffer, 4);
	memcpy(&(paquete->offset), buffer + 4, 4);
	memcpy(&(paquete->bytes), buffer + 8, 4);

	free(buffer);
	return paquete;
}
//----------------------------------------------------->
void * serializar_almacenar_pagina(int pagina, int offset, int bytes,
		void * buffer) {
	t_paquete_almacenar_pagina * paquete = malloc(
			sizeof(t_paquete_almacenar_pagina));
	void * ret_buffer = malloc(12 + bytes);

	paquete->nro_pagina = pagina;
	paquete->offset = offset;
	paquete->bytes = bytes;

	memcpy(ret_buffer, &(paquete->nro_pagina), 4);
	memcpy(ret_buffer + 4, &(paquete->offset), 4);
	memcpy(ret_buffer + 8, &(paquete->bytes), 4);
	memcpy(ret_buffer + 12, buffer, bytes);

	free(paquete);
	return ret_buffer;
}

t_paquete_almacenar_pagina * deserializar_almacenar_pagina(void * buffer) {
	t_paquete_almacenar_pagina * paquete = malloc(
			sizeof(t_paquete_almacenar_pagina));

	memcpy(&(paquete->nro_pagina), buffer, 4);
	memcpy(&(paquete->offset), buffer + 4, 4);
	memcpy(&(paquete->bytes), buffer + 8, 4);
	paquete->buffer = malloc(paquete->bytes);
	memcpy(paquete->buffer, buffer + 12, paquete->bytes);

	free(buffer);
	return paquete;
}
//----------------------------------------------------->
void * serializar_fin_prog(int pid) {
	return serializar_header((uint8_t) 13, (uint32_t) pid);
}

int deserializar_fin_prog(void * buffer) {
	t_header * head = deserializar_header(buffer);

	if (head->identificador != (uint8_t) 13) {
		puts("Error en el header **fin_programa**");
		return -1;
	}
	int response = head->tamanio;

	free(head);
	return response;
}
//----------------------------------------------------->
void * serializar_cambio_proceso(int pid) {
	return serializar_header((uint8_t) 18, (uint32_t) pid);
}

int deserializar_cambio_proceso(void * buffer) {
	t_header * head = deserializar_header(buffer);

	if (head->identificador != (uint8_t) 18) {
		puts("Error en el header **cambio_proceso_activo**");
		return -1;
	}
	int response = head->tamanio;

	free(head);
	return response;
}
//----------------------------------------------------->
void * serializar_tamanio_pagina(int page_size) {
	return serializar_header((uint8_t) 19, (uint32_t) page_size);
}

int deserializar_tamanio_pagina(void * buffer) {
	t_header * head = deserializar_header(buffer);

	if (head->identificador != (uint8_t) 19) {
		puts("Error en el header **tamaño_pagina**");
		return -1;
	}

	int response = head->tamanio;

	free(head);
	return response;
}
//----------------------------------------------------->
void * serializar_respuesta_inicio(int response) {
	return serializar_header((uint8_t) 17, (uint32_t) response);
}

int deserializar_respuesta_inicio(void * buffer) {
	t_header * head = deserializar_header(buffer);

	if (head->identificador != (uint8_t) 17) {
		puts("Error en el header **respuesta_inicio_programa**");
		return -1;
	}

	int response = head->tamanio;

	free(head);
	return response;
}
//----------------------------------------------------->
void * serializar_ansisop(void * programa, int codigo_length) {

	void * buffer_pack = malloc(4 + codigo_length);
	memset(buffer_pack, '\0', 4 + codigo_length);

	memcpy(buffer_pack, &(codigo_length), 4);
	memcpy(buffer_pack + 4, programa, codigo_length);

	return buffer_pack;
}

t_paquete_programa * deserializar_ansisop(void * buffer) {

	t_paquete_programa * paquete = malloc(sizeof(t_paquete_programa));

	memcpy(&(paquete->programa_length), buffer, 4);
	paquete->codigo_programa = malloc(paquete->programa_length);
	memcpy(paquete->codigo_programa, buffer + 4, paquete->programa_length);

	free(buffer);
	return paquete;
}
//----------------------------------------------------->
void * serializar_imprimir_texto(char * texto) {
	return serializar_ansisop((void *)texto, strlen(texto));
}

char * deserializar_imprimir_texto(void * buffer) {

	t_paquete_programa * paquete = deserializar_ansisop(buffer);

	char * texto = string_new();
	string_append(&texto, paquete->codigo_programa);
	free(paquete->codigo_programa);
	free(paquete);

	return texto;
}
//----------------------------------------------------->
void * serializar_imprimir_valor(int valor) {
	return serializar_header((uint8_t) 11, (uint32_t) valor);
}

int deserializar_imprimir_valor(void * buffer) {
	int valor = 0;

	t_header * head = deserializar_header(buffer);
	valor = head->tamanio;
	free(head);

	return valor;
}

//----------------------------------------------------->
void * serializar_identificador_semaforo(char* id) {
	return serializar_ansisop((void *)id, strlen(id));
}

char * deserializar_identificador_semaforo(void * buffer) {
	t_paquete_programa * paquete = deserializar_ansisop(buffer);

	char * texto = string_new();
	string_append(&texto, paquete->codigo_programa);
	free(paquete->codigo_programa);
	free(paquete);

	return texto;
}
//----------------------------------------------------->
void * serializar_nombre_compartida(t_nombre_compartida variable){

	return serializar_ansisop(variable, strlen(variable));
}

char * deserializar_nombre_compartida(void * buffer) {
	t_paquete_programa * paquete = deserializar_ansisop(buffer);

	char * texto = string_new();
	string_append(&texto, paquete->codigo_programa);
	free(paquete->codigo_programa);
	free(paquete);

	return texto;
}
//----------------------------------------------------->

void * serializar_asignar_valor_compartido(t_nombre_compartida nombre_variable, t_valor_variable valor_variable){

	int nombre_variable_lentgh = string_length(nombre_variable);

	t_paquete_asignar_valor_compartido * paquete = malloc(sizeof(t_paquete_solicitar_pagina));
	void * buffer = malloc(8 + nombre_variable_lentgh);

	paquete->valor = valor_variable;
	paquete->nombre_length = nombre_variable_lentgh;

	memcpy(buffer, &(paquete->valor), 4);
	memcpy(buffer + 4, &(paquete->nombre_length), 4);
	memcpy(buffer + 8, nombre_variable, nombre_variable_lentgh);

	free(paquete);
	return buffer;

}


t_paquete_asignar_valor_compartido * deserializar_asignar_valor_compartido(void * buffer) {

	t_paquete_asignar_valor_compartido * paquete = malloc(sizeof(t_paquete_asignar_valor_compartido));


	memcpy(&(paquete->valor), buffer, 4);
	memcpy(&(paquete->nombre_length), buffer + 4, 4);
	paquete->nombre = string_duplicate((char *)(buffer + 8));

	free(buffer);
	return paquete;
}

//----------------------------------------------------->

void * serializar_entrada_salida(t_nombre_dispositivo nombre_dispositivo, int tiempo){

	int nombre_dispositivo_length = string_length(nombre_dispositivo);
	void * buffer = malloc(8 + nombre_dispositivo_length);
	memcpy(buffer, &(tiempo), 4);
	memcpy(buffer + 4, &(nombre_dispositivo_length), 4);
	memcpy(buffer + 8, nombre_dispositivo, nombre_dispositivo_length);

	return buffer;
}


t_paquete_entrada_salida * deserializar_entrada_salida(void * buffer){

	t_paquete_entrada_salida* paquete = malloc(sizeof(t_paquete_entrada_salida));

		memcpy(&(paquete->tiempo), buffer, 4);
		memcpy(&(paquete->nombre_length), buffer + 4, 4);
		paquete->nombre = malloc(paquete->nombre_length);
		memcpy(paquete->nombre, buffer + 8, paquete->nombre_length);
//		paquete->nombre = string_duplicate((char *)(buffer + 8));

		free(buffer);
		return paquete;
}
//----------------------------------------------------->
int enviar_handshake(int socket, int id) {
	void * buffer = serializar_header((uint8_t) id, (uint32_t) 0);

	int result = send(socket, buffer, 5, 0);

	if (result == -1) {
		puts("Error en el envio del handshake");
	} else {
		puts("Handshake enviado");
	}

	free(buffer);
	return result;
}

int recibir_handshake(int socket) {
	void * buffer = malloc(5);
	int handshake_id = 0;

	int result = recv(socket, buffer, 5, 0);

	if (result == -1) {
		puts("Error en el recv del handshake");
		return result;
	}
	if (result <= 0)
		return -1;

	t_header * header = deserializar_header(buffer);

	handshake_id = (int) header->identificador;
	free(header);

	return handshake_id;
}
//----------------------------------------------------->
int enviar_header(int id, int tamanio, int socket) {
	void * buffer = serializar_header((uint8_t) id, (uint32_t) tamanio);

	int result = send(socket, buffer, 5, 0);

	if (result <= 0) {
		//printf("Error en el envio del header %d\n", id);
	}

	free(buffer);
	return result;
}

t_header * recibir_header(int socket) {
	void * buffer = malloc(5);

	int result = recv(socket, buffer, 5, 0);

	if (result <= 0) {
		//puts("Error en el recv del header");
		return NULL;
	}

	t_header * header = deserializar_header(buffer);

	return header;
}
//----------------------------------------------------->
int enviar_solicitud_lectura(int pagina, int offset, int bytes, int socket) {
	int result = enviar_header(15, 12, socket);

	if (result == -1)
		return result;

	void * solicitud = serializar_leer_pagina(pagina, offset, bytes);
	result = send(socket, solicitud, 12, 0);

	if (result == -1)
		return result;

	free(solicitud);
	return EXIT_SUCCESS;
}

t_paquete_solicitar_pagina * recibir_solicitud_lectura(int bytes_to_recv,
		int socket) {
	void * buffer = malloc(bytes_to_recv);
	int result = recv(socket, buffer, bytes_to_recv, 0);

	if (result <= 0) {
		puts("Error en el recv de ++Solicitud_lectura+++");
		return NULL;
	}

	t_paquete_solicitar_pagina * solicitud = deserializar_leer_pagina(buffer);

	return solicitud;
}
//----------------------------------------------------->
int enviar_solicitud_escritura(int pagina, int offset, int bytes, void * buffer,
		int socket) {

	int bytes_to_send = 12 + bytes;
	int result = enviar_header(16, bytes_to_send, socket);

	if (result <= 0)
		return result;

	void * solicitud = serializar_almacenar_pagina(pagina, offset, bytes,
			buffer);
	result = send(socket, solicitud, bytes_to_send, 0);

	if (result <= 0)
		return result;

	free(solicitud);
	return EXIT_SUCCESS;
}

t_paquete_almacenar_pagina * recibir_solicitud_escritura(int bytes_to_recv,
		int socket) {
	void * buffer = malloc(bytes_to_recv);
	int result = recv(socket, buffer, bytes_to_recv, 0);

	if (result <= 0) {
		puts("Error en el recv de +++Solicitud_lectura+++");
		return NULL;
	}

	t_paquete_almacenar_pagina * solicitud = deserializar_almacenar_pagina(
			buffer);

	return solicitud;
}
//----------------------------------------------------->
int enviar_respuesta_inicio(int socket, int respuesta) {
	void * buffer = serializar_respuesta_inicio(respuesta);
	int r = send(socket, buffer, 5, 0);

	if (r == -1) {
		puts("Error en el envio de +++Respuesta_inicio_programa+++");
		return r;
	}

	free(buffer);
	return r;
}

int recibir_respuesta_inicio(int socket) {
	void * buffer = malloc(5);
	int r = recv(socket, buffer, 5, 0);

	if (r <= 0)
		return 0;

	int respuesta = deserializar_respuesta_inicio(buffer);

	return respuesta;
}
//----------------------------------------------------->
int enviar_inicializar_programa(int pid, int paginas, char * codigo, int socket) {
	void * buffer = serializar_iniciar_prog(pid, paginas, codigo);
	int tamanio_buffer = 12 + strlen(codigo);

	int r = send(socket, buffer, tamanio_buffer, 0);

	if (r <= 0)
		puts("Error en el envio de ++inicializar_programa++");

	free(buffer);
	return r;
}

t_paquete_inicializar_programa * recibir_inicializar_programa(
		int bytes_a_recibir, int socket) {
	int recibido = -1;
	void * buffer = malloc(bytes_a_recibir);

	recibido = recv(socket, buffer, bytes_a_recibir, 0);

	if (recibido <= 0) {
		puts("Error en el recv de ++inicializar_programa++");
		return NULL;
	}

	t_paquete_inicializar_programa * paquete = deserializar_iniciar_prog(
			buffer);

	return paquete;
}
//----------------------------------------------------->
int enviar_texto(char * texto, int socket) {
	int result = enviar_header(12, string_length(texto) + 4, socket);

	if (result == -1)
		return result;

	void * buffer_out = serializar_imprimir_texto(texto);
	result = send(socket, buffer_out, string_length(texto) + 4, 0);

	if (result == -1)
		return result;

	free(buffer_out);
	return EXIT_SUCCESS;
}

char* recibir_texto(int buffer_size, int socket) {

	void * buffer = malloc(buffer_size);
	int r = recv(socket, buffer, buffer_size, 0);

	if (r <= 0)
		return NULL;

	char * respuesta = deserializar_imprimir_texto(buffer);

	return respuesta;
}

//------------------------------------------------------>
int enviar_valor_de_variable(t_valor_variable valor, int socket) {
	int result = enviar_header(11, valor, socket);

	if (result == -1)
		return result;
	return EXIT_SUCCESS;
}

t_valor_variable recibir_valor_de_variable(int socket) {

	void * buffer = malloc(5);
	int r = recv(socket, buffer, 5, 0);

	if (r <= 0)
		return 0;

	t_valor_variable respuesta = deserializar_imprimir_valor(buffer);

	return respuesta;
}

//---------------------------------------------------->
int enviar_wait_identificador_semaforo(char* identificador_semaforo, int socket) {

	int result = enviar_header(22, string_length(identificador_semaforo) + 4, socket);

	if (result == -1)
		return result;

	void * buffer_out = serializar_identificador_semaforo(identificador_semaforo);

	result = send(socket, buffer_out, string_length(identificador_semaforo) + 4, 0);
	if (result == -1)
		return result;

	free(buffer_out);
	return EXIT_SUCCESS;
}

char* recibir_wait_identificador_semaforo(int buffer_size, int socket) {

	void * buffer = malloc(buffer_size);
	int r = recv(socket, buffer, buffer_size, 0);

	if (r <= 0)
		return NULL;

	char * respuesta = deserializar_imprimir_texto(buffer);

	return respuesta;
}

//---------------------------------------------------->
int enviar_signal_identificador_semaforo(char* identificador_semaforo, int socket) {

	int result = enviar_header(23, string_length(identificador_semaforo) + 4, socket);

	if (result == -1)
		return result;

	void * buffer_out = serializar_identificador_semaforo(identificador_semaforo);

	result = send(socket, buffer_out,string_length(identificador_semaforo) + 4, 0);

	if (result == -1)
		return result;

	free(buffer_out);
	return EXIT_SUCCESS;
}

char* recibir_signal_identificador_semaforo(int buffer_size, int socket) {

	void * buffer = malloc(buffer_size);
	int r = recv(socket, buffer, buffer_size, 0);

	if (r <= 0)
		return 0;

	char * respuesta = deserializar_imprimir_texto(buffer);

	return respuesta;
}


//---------------------------------------------------->
int enviar_obtener_valor_compartido(t_nombre_compartida variable, int socket){

	int result = enviar_header(25, string_length(variable) + 4, socket);

	if (result == -1)
			return result;

	void * buffer_out = serializar_nombre_compartida(variable);

	result = send(socket, buffer_out, string_length(variable) + 4, 0);

	if (result == -1)
			return result;

		free(buffer_out);
		return EXIT_SUCCESS;

}

char* recibir_obtener_valor_compartido(int buffer_size, int socket) {

	void * buffer = malloc(buffer_size);
	int r = recv(socket, buffer, buffer_size, 0);

	if (r <= 0)
		return 0;

	char * respuesta = deserializar_imprimir_texto(buffer);

	return respuesta;
}


//---------------------------------------------------->

int enviar_asignar_valor_compartido(t_nombre_compartida nombre_variable, t_valor_variable valor, int socket){

	int nombre_variable_length = string_length(nombre_variable);

	int result = enviar_header(26, nombre_variable_length + 8, socket);

	if (result == -1)
			return result;

	void * buffer_out = serializar_asignar_valor_compartido(nombre_variable,valor);

	result = send(socket, buffer_out, nombre_variable_length + 8, 0);

	if (result == -1)
			return result;

		free(buffer_out);
		return EXIT_SUCCESS;

}

t_paquete_asignar_valor_compartido * recibir_asignar_valor_compartido(int buffer_size, int socket) {

	void * buffer = malloc(buffer_size);

	int r = recv(socket, buffer, buffer_size, 0);

	if (r <= 0)
		return NULL;

	t_paquete_asignar_valor_compartido * respuesta = deserializar_asignar_valor_compartido(buffer);

	return respuesta;
}

//---------------------------------------------------->

int enviar_entrada_salida(t_nombre_dispositivo nombre_dispositivo, int tiempo, int socket){

	int nombre_dispositivo_length = string_length(nombre_dispositivo);

	int result = enviar_header(24, nombre_dispositivo_length + 8, socket);

	if (result == -1)
			return result;

	void * buffer_out = serializar_entrada_salida(nombre_dispositivo, tiempo);

	result = send(socket, buffer_out, nombre_dispositivo_length + 8, 0);

	if (result == -1)
			return result;

		free(buffer_out);
		return EXIT_SUCCESS;

}

t_paquete_entrada_salida * recibir_entrada_salida(int buffer_size, int socket) {

	void * buffer = malloc(buffer_size);

	int r = recv(socket, buffer, buffer_size, 0);

	if (r <= 0)
		return NULL;

	t_paquete_entrada_salida * respuesta = deserializar_entrada_salida(buffer);

	return respuesta;
}
