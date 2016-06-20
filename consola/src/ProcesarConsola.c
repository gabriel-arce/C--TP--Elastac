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

	pthread_mutex_init(&mutex_nucleo, 0);

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

//	printf("%s\n", prog_buffer);
//	printf("Length del programa: %d\n", string_length(prog_buffer));

	//Cargar configuracion
	cargar_config();

	//Crear socket al nucleo
	if ((socketConsola = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1) {
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	//--------handshake
	void * buffer_hs = serializar_header((uint8_t) 1, (uint32_t) 0);
	int r = send(socketConsola, buffer_hs, 5, 0);

	if (r == -1) {
		printf("Error en el envio del handshake\n");
		return EXIT_FAILURE;
	}
	printf("Se ha enviado el handshake a NUCLEO\n");

	free(buffer_hs);

	//-----------INICIO ENVIO PROGRAMA
	//---------primero envio el header
	int codigo_length = string_length(prog_buffer);
	int tamanio_paquete = 4 + codigo_length;

	void * buffer_head = serializar_header((uint8_t) Iniciar_ansisop, (uint32_t) tamanio_paquete);
	r = send(socketConsola, buffer_head, 5, 0);
	free(buffer_head);
	if (r == -1) {
		printf("Error en el send de la cabecera\n");
		return EXIT_FAILURE;
	}

	//-----------luego envio el codigo del programa
	void * buffer_pack = serializar_ansisop(prog_buffer);
	r = send(socketConsola, buffer_pack, tamanio_paquete, 0);
	free(buffer_pack);
	if (r == -1) {
		printf("Error en el send del programa\n");
		return EXIT_FAILURE;
	}
	puts("Se ha enviado el codigo del programa AnSISOP");
//	//-------------FIN ENVIO PROGRAMA

	//-----------Quedo en espera para recibir cosas
	int recibido = 1;

	while (recibido >= 0) {

		pthread_mutex_lock(&mutex_nucleo);

		void * buffer_in = malloc(5);

		recibido = recv(socketConsola, buffer_in, 5, MSG_WAITALL);

		if (recibido == -1) {
			free(buffer_in);
			printf("Error en el recv");
			break;
		}

		if (recibido == 0) {
			free(buffer_in);
			pthread_mutex_unlock(&mutex_nucleo);
			continue;
		}

		t_header * cabecera = deserializar_header(buffer_in);

		switch (cabecera->identificador) {
		case Imprimir_valor:
			printf("\n");
			printf("VALOR: %d\n", cabecera->tamanio);
			break;
		case Imprimir_texto:
			printf("\n");
			void * text_buff = malloc(cabecera->tamanio);
			recv(socketConsola, text_buff, cabecera->tamanio, MSG_WAITALL);
			char * texto = deserializar_imprimir_texto(text_buff);
			printf("TEXTO: %s\n", texto);
			free(texto);
			break;
		case Fin_ansisop:
			printf("\n");
			if (cabecera->tamanio == 0) {
				puts("No se ha podido inicializar el programa. Programa rechazado");
			} else {
				printf("Finalizo programa nro.: %d\n", cabecera->tamanio);
			}
			recibido = -1;
			break;
		default:
			puts("Cabecera no reconocida");
			break;
		}

		free(cabecera);
		pthread_mutex_unlock(&mutex_nucleo);
	}

	fclose(fp_in);
	free(prog_buffer);
	return EXIT_SUCCESS;
}
