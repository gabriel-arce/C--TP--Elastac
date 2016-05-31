/*
 * elestac_paquetes.c
 *
 *  Created on: 10/5/2016
 *      Author: utnso
 */

#include "elestac_paquetes.h"

int enviar_handshake(int sock_fd, uint8_t id_proc, uint32_t size) {

	t_header * handshake = malloc(sizeof(t_header));
	handshake->identificador = id_proc;
	handshake->tamanio = size;
	void * buffer_out = malloc(5);

	memcpy(buffer_out, &handshake->identificador, 1);
	memcpy(buffer_out + 1, &handshake->tamanio, 4);

	send(sock_fd, buffer_out, 5, 0);

	int resultado_send = send(sock_fd, buffer_out, 5, 0);

	free(handshake);
	free(buffer_out);

	return resultado_send;
}

t_header * recibir_handshake(int sock_fd, t_header * handshake_in) {

	void * buffer_in = malloc(5);

	int recibido = recv(sock_fd, buffer_in, 5, MSG_WAITALL);

	if (recibido == -1) {
		printf("Error en el recv.\n");
		return NULL;
	}

	memcpy(&handshake_in->identificador, buffer_in, 1);
	memcpy(&handshake_in->tamanio, buffer_in + 1, 4);

	free(buffer_in);

	return handshake_in;

}

void * serializar_header(t_header * header) {
	void * buffer = malloc(5);

	memcpy(buffer, &header->identificador, 1);
	memcpy(buffer + 1, &header->tamanio, 4);

	return buffer;
}

t_header * deserializar_header(void * buffer) {
	t_header * header = malloc(sizeof(t_header));

	memcpy(&header->identificador, buffer, 1);
	memcpy(&header->tamanio, buffer + 1, 4);

	return header;
}

void * serializar_pcb(t_pcb * pcb) {
	int offset = 0;
	int pcb_size = get_pcb_size(pcb);

	void * buffer = malloc(pcb_size);

	memcpy(buffer + offset, &pcb->pcb_pid, sizeof(pcb->pcb_pid));
	offset += sizeof(pcb->pcb_pid);
	memcpy(buffer + offset, &pcb->pcb_pc, sizeof(pcb->pcb_pc));
	offset += sizeof(&pcb->pcb_pc);
	memcpy(buffer + offset, &pcb->pcb_sp, sizeof(pcb->pcb_sp));
	offset += sizeof(pcb->pcb_sp);
	memcpy(buffer + offset, &pcb->paginas_codigo, sizeof(pcb->paginas_codigo));
	offset += sizeof(pcb->paginas_codigo);
	//aca ya empiezan las estructuras

	return buffer;
}

t_pcb * deserializar_pcb(void * buffer) {
	int offset = 0;
	t_pcb * pcb = malloc(t_pcb);

	memcpy(&pcb->pcb_pid, buffer + offset, sizeof(pcb->pcb_pid));
	offset += sizeof(pcb->pcb_pid);
	memcpy(&pcb->pcb_pc, buffer + offset, sizeof(pcb->pcb_pc));
	offset += sizeof(&pcb->pcb_pc);
	memcpy(&pcb->pcb_sp, buffer + offset, sizeof(pcb->pcb_sp));
	offset += sizeof(pcb->pcb_sp);
	memcpy(&pcb->paginas_codigo, buffer + offset, sizeof(pcb->paginas_codigo));
	offset += sizeof(pcb->paginas_codigo);
	//problema: si se va a hacer por listas las estructuras necesito los sizes

	return pcb;
}
