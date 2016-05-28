/*
 * ProcesarConsola.c
 *
 *  Created on: 27 de abr. de 2016
 *      Author: gabriel
 */

#include "Consola.h"
#include <elestac_sockets.h>

int main(int argc, char * argv[]) {

	FILE * fp_in;
	long fp_size;
	char * prog_buffer = NULL;

	int socketConsola = 0;		//Descriptor de consola
	int socketEscucha = 0;		//Descriptor de escucha

	if (argc == 1) {
		printf("Debe ingresar el nombre del programa AnSISOP\n");
		return -1;
	}

	if (argc != 2) {
		MostrarMensajeDeError(CantidadArgumentosIncorrecta);
		return -2;
	}

	fp_in = fopen(argv[1], "r");

	if (fp_in == NULL) {
		MostrarMensajeDeError(NoSePudoAbrirIn);
		return -3;
	}

	fseek(fp_in, 0L, SEEK_END);
	fp_size = ftell(fp_in);
	rewind(fp_in);

	prog_buffer = calloc(1, fp_size + 1);
	if ((fread(prog_buffer, fp_size, 1, fp_in)) != 1) {
		fclose(fp_in);
		free(prog_buffer);
		puts("Error en el copy");
	}

	//Cargar configuracion
	printf("Proceso Consola..\n");
	printf("\n");

	cargar_config();

	//Crear socket al nucleo
	if ((socketConsola = clienteDelServidor(consola->ip_nucleo,
			consola->puerto_nucleo)) == -1) {
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}
	//--------handshake
	void * buffer_out = malloc(5);
	t_header * handshake_out = malloc(sizeof(t_header));

	handshake_out->identificador = 1;
	handshake_out->tamanio = 0;

	memcpy(buffer_out, &handshake_out->identificador, sizeof(uint8_t));
	memcpy(buffer_out + 1, &handshake_out->tamanio, sizeof(uint32_t));

	if (send(socketConsola, buffer_out, 5, 0) == -1) {
		puts("Error en el send del handshake");
		return -1;
	}
	free(buffer_out);
	free(handshake_out);

	//---------envio el programa
	t_header * programa_header = malloc(sizeof(t_header));
	programa_header->identificador = Iniciar_ansisop;
	programa_header->tamanio = fp_size+1;
	void * buffer_header = malloc(5);

	if (send(socketConsola, buffer_header, 5, 0) == -1) {
		puts("Error en el send de la cabecera de programa");
		return -1;
	}

	if (send(socketConsola, prog_buffer, fp_size, 0) == -1) {
		puts("Error en el send del programa");
		return -1;
	}

	free(programa_header);
	free(buffer_header);

	//-----------Quedo en espera para recibir cosas
	int recibido = 1;
	void * buffer_in = malloc(5);
	t_header * cabecera = malloc(sizeof(t_header));

	while(recibido) {
		recibido = recv(socketConsola, buffer_in, 5, 0);

		if (recibido == -1) {
			printf("Error en el recv");
			continue;
		}

		memcpy(&cabecera->identificador, buffer_in, 1);
		memcpy(&cabecera->tamanio, buffer_in + 1, 4);

		switch(cabecera->identificador) {
		case Imprimir : puts("Imprimo algo\n"); break;
		case Imprimir_texto : puts("Imprimo texto\n"); break;
		case Fin_ansisop : puts("Finalizo programa\n"); break;
		default : puts("Cabecera no reconocida"); break;
		}

		cabecera->identificador = 0;
		cabecera->tamanio = 0;
	}

	free(buffer_in);
	fclose(fp_in);
	free(prog_buffer);
	return EXIT_SUCCESS;
}
