/*
 * ProcesarConsola.c
 *
 *  Created on: 27 de abr. de 2016
 *      Author: gabriel
 */

#include "Consola.h"
#include <elestac_sockets.h>

int main(int argc, char * argv[]) {

	signal(SIGINT, rutina); //aborta el programa

	printf("Proceso Consola..\n");
	printf("\n");

	FILE * fp_in;
	long fp_size;
	char * prog_buffer = NULL;

	socketConsola = 0;		//Descriptor de consola

	//Cargar configuracion
	if (chequear_argumentos(argc, 3) == -1)
		return EXIT_FAILURE;

	cargar_archivo_config(argv, (void *) cargar_config);

	fp_in = fopen(argv[2], "rb");

	if (fp_in == NULL) {
		MostrarMensajeDeError(NoSePudoAbrirIn);
		return -3;
	}

	fseek(fp_in, 0, SEEK_END);
	fp_size = ftell(fp_in);
	rewind(fp_in);

	prog_buffer = malloc(fp_size);
	memset(prog_buffer, '\0', fp_size);
	if ((fread(prog_buffer, 1, fp_size, fp_in)) < 0) {
		fclose(fp_in);
		free(prog_buffer);
		puts("Error en el copy");
	}

	if ((socketConsola = clienteDelServidor(consola->ip_nucleo, consola->puerto_nucleo)) == -1) {
		MostrarMensajeDeError(NoSePudoCrearSocket);
		return EXIT_FAILURE;
	}

	//--------handshake
	int result = enviar_handshake(socketConsola, 1);
	if (result == -1) {
		return result;
	}

	//-----------INICIO ENVIO PROGRAMA
	//---------primero envio el header
	int tamanio_paquete = 4 + fp_size;

	result = enviar_header(Iniciar_ansisop, tamanio_paquete, socketConsola);
	if (result == -1) {
		return -1;
	}

	//-----------luego envio el codigo del programa
	void * buffer_pack = serializar_ansisop(prog_buffer, fp_size);
	result = send(socketConsola, buffer_pack, tamanio_paquete, 0);
	free(buffer_pack);
	if (result == -1) {
		printf("Error en el send del programa\n");
		return EXIT_FAILURE;
	}
	puts("Se ha enviado el codigo del programa AnSISOP");
	//-------------FIN ENVIO PROGRAMA

	//-----------Quedo en espera para recibir cosas
	int recibido = 1;

	while (recibido >= 0) {

		pthread_mutex_lock(&mutex_nucleo);

		t_header * cabecera = recibir_header(socketConsola);

		if (cabecera == NULL) {
			pthread_mutex_unlock(&mutex_nucleo);
			continue;
		}

		switch (cabecera->identificador) {
		case Imprimir_valor:
			printf("\n");
			printf("VALOR: %d\n", cabecera->tamanio);
			break;
		case Imprimir_texto:
			printf("\n");
			char* texto = string_new();
			texto = recibir_texto(cabecera->tamanio, socketConsola);
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
