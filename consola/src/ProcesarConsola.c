/*
 * ProcesarConsola.c
 *
 *  Created on: 27 de abr. de 2016
 *      Author: gabriel
 */

#include "Consola.h"
#include <elestac_sockets.h>

int main(int argc, char * argv[]) {

	printf("Proceso Consola..\n");
	printf("\n");

	FILE * fp_in;
	long fp_size;
	char * prog_buffer = NULL;

	int socketConsola = 0;		//Descriptor de consola

	if (argc == 1) {
		printf("Debe ingresar el nombre del programa AnSISOP\n");
		return -1;
	}

	if (argc != 2) {
		MostrarMensajeDeError(CantidadArgumentosIncorrecta);
		return -2;
	}

	fp_in = fopen(argv[1], "rb");

	if (fp_in == NULL) {
		MostrarMensajeDeError(NoSePudoAbrirIn);
		return -3;
	}

	fseek(fp_in, 0, SEEK_END);
	fp_size = ftell(fp_in);
	rewind(fp_in);

	prog_buffer = malloc((fp_size + 1) * sizeof(char));
	if ((fread(prog_buffer, 1, fp_size, fp_in)) < 0) {
		fclose(fp_in);
		free(prog_buffer);
		puts("Error en el copy");
	}
	prog_buffer[fp_size + 1] = '\0';

	printf("%s\n", prog_buffer);
	printf("Length del programa: %d\n", string_length(prog_buffer));

	//Cargar configuracion
	cargar_config();

	//Crear socket al nucleo
	if ((socketConsola = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1) {
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	//--------handshake
	t_header * handshake = malloc(sizeof(t_header));
	handshake->identificador = (uint8_t) 2;
	handshake->tamanio = 0;
	void * buffer_hs = malloc(5);
	memcpy(buffer_hs, &(handshake->identificador), 1);
	memcpy(buffer_hs + 1, &(handshake->tamanio), 4);
	int r = send(socketConsola, buffer_hs, 5, 0);

	if (r == -1) {
		printf("Error en el envio del handshake\n");
		return EXIT_FAILURE;
	}
	printf("se envio el handshake\n");

	free(handshake);
	free(buffer_hs);

	//-----------INICIO ENVIO PROGRAMA
	//---------primero envio el header
	int codigo_length = string_length(prog_buffer);
	int tamanio_paquete = 4 + codigo_length;

	t_header * head = malloc(sizeof(t_header));
	head->identificador = (uint8_t) Iniciar_ansisop;
	head->tamanio = tamanio_paquete;
	void * buffer_head = malloc(5);
	memcpy(buffer_head, &(head->identificador), 1);
	memcpy(buffer_head + 1, &(head->tamanio), 4);
	r = send(socketConsola, buffer_head, 5, 0);
	free(head);
	free(buffer_head);
	if (r == -1) {
		printf("Error en el send de la cabecera\n");
		return EXIT_FAILURE;
	}

	//-----------luego envio el codigo del programa
	t_paquete_programa * prog_package = malloc(sizeof(t_paquete_programa));
	prog_package->programa_length = codigo_length;
	prog_package->codigo_programa = malloc(codigo_length);
	string_append(&prog_buffer, prog_package->codigo_programa);
	void * buffer_pack = malloc(tamanio_paquete);
	memcpy(buffer_pack, &(prog_package->programa_length), 4);
	memcpy(buffer_pack + 4, prog_package->codigo_programa, codigo_length);
	r = send(socketConsola, buffer_pack, tamanio_paquete, 0);
	free(prog_package->codigo_programa);
	free(prog_package);
	free(buffer_pack);
	if (r == -1) {
		printf("Error en el send del programa\n");
		return EXIT_FAILURE;
	}
	//-------------FIN ENVIO PROGRAMA

	//-----------Quedo en espera para recibir cosas
	int recibido = 1;
	t_header * cabecera = malloc(sizeof(t_header));

	while (recibido) {

		void * buffer_in = malloc(5);

		recibido = recv(socketConsola, buffer_in, 5, 0);

		if (recibido == -1) {
			printf("Error en el recv");
			continue;
		}

		memcpy(&cabecera->identificador, buffer_in, 1);
		memcpy(&cabecera->tamanio, buffer_in + 1, 4);

		switch (cabecera->identificador) {
		case Imprimir:
			puts("Imprimo algo\n");
			break;
		case Imprimir_texto:
			puts("Imprimo texto\n");
			break;
		case Fin_ansisop:
			puts("Finalizo programa\n");
			break;
		default:
			puts("Cabecera no reconocida");
			break;
		}

		cabecera->identificador = 0;
		cabecera->tamanio = 0;
		free(buffer_in);
	}

	fclose(fp_in);
	free(prog_buffer);
	return EXIT_SUCCESS;
}
