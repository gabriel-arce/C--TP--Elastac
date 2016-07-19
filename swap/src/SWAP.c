/*
 ============================================================================
 Name        : SWAP.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <swap.h>

void cargar_config(char ** config_path) {

	t_config * config_file = config_create(config_path[1]);

	swap_config = malloc(sizeof(t_swap_config));
	memset(swap_config, 0, sizeof(t_swap_config));

	swap_config->espacio_swap = malloc(sizeof(char) * 10);
	swap_config->espacio_swap = string_new();
	string_append(&swap_config->espacio_swap,
			getStringProperty(config_file, "NOMBRE_SWAP"));

	swap_config->puerto_escucha = getIntProperty(config_file, "PUERTO_ESCUCHA");
	swap_config->cant_paginas = getIntProperty(config_file, "CANTIDAD_PAGINAS");
	swap_config->pagina_size = getIntProperty(config_file, "TAMANIO_PAGINA");
	swap_config->retardo_compactacion = getIntProperty(config_file,
			"RETARDO_COMPACTACION");

	config_destroy(config_file);
}

void imprimir_config() {
	printf("PUERTO_ESCUCHA: %d\n", swap_config->puerto_escucha);
	printf("NOMBRE_SWAP: %s\n", swap_config->espacio_swap);
	printf("CANTIDAD_PAGINAS: %d\n", swap_config->cant_paginas);
	printf("TAMANIO_PAGINA: %d\n", swap_config->pagina_size);
	printf("RETARDO_COMPACTACION: %d\n", swap_config->retardo_compactacion);
}

void inicializar_semaforos() {
	pthread_mutex_init(&mutex_umc_recv, 0);
}

void iniciar_servidor_UMC() {
	printf("\n");

	int optval = 1;

	socket_SWAP = crearSocket();

	setsockopt(socket_SWAP, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	bindearSocket(socket_SWAP, swap_config->puerto_escucha);

	escucharEn(socket_SWAP);

	socket_UMC = aceptarEntrantes(socket_SWAP);

	int id = recibir_handshake(socket_UMC);

	if (id == -1)
		puts("Se conecto alguien desconocido");

	if (id == ID_UMC)
		printf("Se conecto UMC con el socket: %d\n", socket_UMC);

	printf("\n");
}

void escuchar_solicitudes_UMC() {
	printf("\n");

	bool keep_iterating = true;
	t_header * header = NULL;

	while (keep_iterating) {
		pthread_mutex_lock(&mutex_umc_recv);

		header = recibir_header(socket_UMC);

		if (!header) {
			keep_iterating = false;
			pthread_mutex_unlock(&mutex_umc_recv);
			continue;
		}

		switch (header->identificador) {
			case Inicializar_programa:
				inicializar_programa(header->tamanio);
			break;
			case Finalizar_programa:
				finalizar_programa(header->tamanio);
			break;
			case Solicitar_pagina:
				leer_pagina(header->tamanio);
			break;
			case Almacenar_pagina:
				almacenar_pagina(header->tamanio);
			break;
		default:
			break;
		}

		free(header);

		pthread_mutex_unlock(&mutex_umc_recv);
	}

	printf("\n");
}

/*
 * En las proximas funciones ante cualquier error debe retornar "EXIT_ERROR"
 * ya que "mas arriba" analiza el resultado de cada operacion
 * */

int inicializar_programa(int buffer_init_size) {

	puts("INICIALIZAR_PROGRAMA");

	t_paquete_inicializar_programa * paquete_inicio_prog = recibir_inicializar_programa(buffer_init_size, socket_UMC);
	//LAS POSIBLES RESPUESTAS SON:	 0 -> NO | 1 -> SI
	int respuesta_inicio = 0;

	if (paquete_inicio_prog == NULL) {
		enviar_respuesta_inicio(socket_UMC, Respuesta_inicio_NO);
		return EXIT_ERROR;
	}

//	printf("pid: %d\n", paquete_inicio_prog->pid);
//	printf("paginas: %d\n", paquete_inicio_prog->paginas_requeridas);
//	printf("length: %d\n", paquete_inicio_prog->programa_length);
//	printf("codigo: \n%s\n", paquete_inicio_prog->codigo_programa);

	//calculo si paquete_inicio_prog->paginas_requeridas <= paginas_libres_del_espacio_swap

	//TRUE -> se reservan las paginas requeridas y se almacena paquete_inicio_prog->codigo_programa **
	//** siempre seran en las primeras paginas reservadas, y luego en la siguiente reservada el stack (eso se almacena despues)

	//FALSE -> se rechaza el programa

	//ENTONCES EN LO POSIBLE QUE SEA ASI: respuesta_inicio = *nombre_de_la_funcion_que_hace_la_magia*
	//o sea esa funcion tienen que devolver 0 o 1

	enviar_a_umc_respuesta_inicio(paquete_inicio_prog, respuesta_inicio);

	return EXIT_SUCCESS;
}

void enviar_a_umc_respuesta_inicio(t_paquete_inicializar_programa * paquete_inicio_prog, int respuesta_inicio) {
	int r = enviar_respuesta_inicio(socket_UMC, respuesta_inicio);

	if (r)
		puts("Respuesta enviada");

	enviar_header(Inicializar_programa, 12 + paquete_inicio_prog->programa_length, socket_UMC);

	enviar_inicializar_programa(paquete_inicio_prog->pid, paquete_inicio_prog->paginas_requeridas, paquete_inicio_prog->codigo_programa, socket_UMC);

	free(paquete_inicio_prog->codigo_programa);
	free(paquete_inicio_prog);
}

int finalizar_programa(int pid) {

	puts("FINALIZAR_PROGRAMA");

	printf("pid: %d\n", pid);

	//libero memoria y estructuras en funcion de "pid"
	//muy probable que luego de esto se deba compactar
	return EXIT_SUCCESS;
}

int leer_pagina(int buffer_read_size) {
	t_paquete_solicitar_pagina * paquete_lect_pag = NULL;
	void * buffer = malloc(buffer_read_size);
	int tamanio_paquete = buffer_read_size - 4;
	void * buffer_paquete = malloc(tamanio_paquete);
	int pid = -1;

	if (recv(socket_UMC, buffer, buffer_read_size, 0) <= 0) {
		printf("\nError en el recv para lectura de datos\n");
		send(socket_UMC, (void *) "", 0, 0);
		return EXIT_ERROR;
	}

	memcpy(&(pid), buffer, 4);
	memcpy(buffer_paquete, buffer + 4, tamanio_paquete);

	paquete_lect_pag = deserializar_leer_pagina(buffer_paquete);

	if (paquete_lect_pag == NULL) {
		send(socket_UMC, (void *) "", 0, 0);
		return EXIT_ERROR;
	}

	printf("pid: %d\n", pid);
	printf("pagina: %d\n", paquete_lect_pag->nro_pagina);
	printf("offset: %d\n", paquete_lect_pag->offset);
	printf("bytes: %d\n", paquete_lect_pag->bytes);

	//char * datos = string_duplicate("soy los datos que se pidieron");

	//busco los datos y los copio en "void * datos_leidos"
	//luego le envio a umc "datos_leidos" (sin header ni nada)

	free(buffer);
	free(paquete_lect_pag);

	return EXIT_SUCCESS;
}

int almacenar_pagina(int buffer_write_size) {

	puts("ALMACENAR_PAGINA");

	t_paquete_almacenar_pagina * paquete_escrt_pag = recibir_solicitud_escritura(buffer_write_size, socket_UMC);

	if (paquete_escrt_pag == NULL) {
		send(socket_UMC, (void *) "-1", 2, 0);
		return EXIT_ERROR;
	}

	printf("pagina: %d\n", paquete_escrt_pag->nro_pagina);
	printf("offset: %d\n", paquete_escrt_pag->offset);
	printf("bytes: %d\n", paquete_escrt_pag->bytes);
	printf("buffer: %s\n", (char *) paquete_escrt_pag->buffer);

	//busco la pagina, almaceno el contenido de "paquete_escrt_pag->buffer" en el bloque correspondiente,
	//y si es necesario actualizo estructuras
	//luego le envias a umc el "ok", o sea algo asi: send(socket_UMC, (void *) "+1", 2, 0);

	return EXIT_SUCCESS;
}
