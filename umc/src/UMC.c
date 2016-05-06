/*
 ============================================================================
 Name        : UMC.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <UMC.h>

void new_line() {
	printf("\n");
}

void cargar_config() {

	t_config * config_file = config_create(CONFIG_PATH);
	umc_config = malloc(sizeof(t_umc_config));
	memset(umc_config, 0, sizeof(t_umc_config));

	umc_config->ip_swap = malloc(sizeof(char) * 15);
	umc_config->ip_swap = string_new();
	string_append(&umc_config->ip_swap,
			getStringProperty(config_file, "IP_SWAP"));

	umc_config->puerto_escucha = getIntProperty(config_file, "PUERTO");
	umc_config->puerto_swap = getIntProperty(config_file, "PUERTO_SWAP");
	umc_config->cant_frames = getIntProperty(config_file, "MARCOS");
	umc_config->frames_size = getIntProperty(config_file, "MARCO_SIZE");
	umc_config->frame_x_prog = getIntProperty(config_file, "MARCO_X_PROG");
	umc_config->entradas_tlb = getIntProperty(config_file, "ENTRADAS_TLB");
	umc_config->retardo = getIntProperty(config_file, "RETARDO");

	config_destroy(config_file);
}

void imprimir_config() {
	puts("ARCHIVO DE CONFIGURACION:");
	printf("PUERTO: %d\n",umc_config->puerto_escucha);
	printf("IP_SWAP: %s\n", umc_config->ip_swap);
	printf("PUERTO_SWAP: %d\n", umc_config->puerto_swap);
	printf("FRAMES: %d\n", umc_config->frames_size);
	printf("MARCO_SIZE: %d\n", umc_config->frames_size);
	printf("MARCO_X_PROG: %d\n", umc_config->frame_x_prog);
	printf("ENTRADAS_TLB: %d\n", umc_config->entradas_tlb);
	printf("RETARDO: %d\n", umc_config->retardo);
	new_line();
}

int tlb_habilitada() {
	return (umc_config->entradas_tlb > 0);
}

void * lanzar_consola() {
	printf("***CONSOLA DEL PROCESO UMC***\n");
	new_line();

	char * buffer_in = malloc(100);
	int fin = 0;
	char cadFin[] = "fin";

	while (fin == 0) {
		new_line();
		printf("Comandos disponibles:\n");
		printf("(1): retardo x[ms]\n");
		printf("(2): dump #\n");
		printf("	#1- estructuras de memoria\n");
		printf("	#2- contenido de memoria\n");
		printf("(3): flush #\n");
		printf("	#1- tlb\n");
		printf("	#2- memory\n");
		printf("(4): fin \n");
		new_line();

		printf(PROMPT);
		scanf("%[^\n]%*c", buffer_in);

		char ** substrings = string_split(buffer_in, " ");
		char * cmd_in = substrings[0];
		int opt_var = strtol(substrings[1], NULL, 10);

		if (string_equals_ignore_case(cmd_in, cadFin)) {
			fin = 1;
			free(buffer_in);
			exit(1);
		}

		if (no_es_comando(cmd_in)) {
			puts(MSJ_ERROR1);
			continue;
		}

		if (string_equals_ignore_case(cmd_in, "retardo")) {
			modificar_retardo(opt_var);
			continue;
		}

		if (string_equals_ignore_case(cmd_in, "dump")) {
			switch (opt_var) {
			case 1:
				reporte_estructuras();
				break;
			case 2:
				reporte_contenido();
				break;

			default:
				puts(MSJ_ERROR2);
				continue;
			}
		}

		if (string_equals_ignore_case(cmd_in, "flush")) {
			switch (opt_var) {
			case 1:
				limpiar_tlb();
				break;
			case 2:
				marcar_paginas();
				break;
			default:
				puts(MSJ_ERROR2);
				continue;
			}
		}

	}

	return EXIT_SUCCESS;

}

int no_es_comando(char * com) {

	char * comandos[3] = { "retardo", "dump", "flush" };

	int i;

	for (i = 0; i < 3; i++) {
		if (string_equals_ignore_case(com, (char *) comandos[i])) {
			return false;
		}
	}

	return true;
}

void * conecta_swap() {
	if ((socket_cliente = clienteDelServidor(umc_config->ip_swap,
			umc_config->puerto_swap)) == -1)
		exit(EXIT_FAILURE);

	//envia handshake
	t_handshake * handshake = malloc(sizeof(t_handshake));
	handshake->identificador = ID_UMC;

	send(socket_cliente, handshake, sizeof(t_handshake), 0);

	//creo el hilo para atender a swap

	return EXIT_SUCCESS;
}

void * escucha_conexiones() {

	int optval = 1;

	socket_servidor = crearSocket();
	if (socket_servidor == -1)
		exit(EXIT_FAILURE);
	new_line();

	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if (bindearSocket(socket_servidor, umc_config->puerto_escucha) == -1)
		exit(EXIT_FAILURE);
	new_line();

	if (escucharEn(socket_servidor) == -1)
		exit(EXIT_FAILURE);
	new_line();

	struct sockaddr_in direccion_cliente;
	unsigned int addrlen = sizeof(struct sockaddr_in);
	int socket_nuevo = 0;
	int recibidos = 1;

	while(recibidos > 0) {

		socket_nuevo = accept(socket_servidor, (struct sockaddr*) &direccion_cliente, &addrlen);
		setsockopt(socket_nuevo, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		t_handshake * handshake_cliente = malloc(sizeof(t_handshake));

		recv(socket_nuevo, handshake_cliente, sizeof(t_handshake), MSG_WAITALL);

		switch (handshake_cliente->identificador) {
		case ID_NUCLEO:
			new_line();
			printf("Se conecto Nucleo \n");
			t_sesion_nucleo * nucleo = malloc(sizeof(t_sesion_nucleo));
			nucleo->socket_nucleo = socket_nuevo;
			enviarPorSocket(socket_servidor, MENSAJE_HANDSHAKE);
			//creo el hilo para atender el nucleo
			break;
		case ID_CPU:
			new_line();
			printf("Se conecto una CPU \n");
			t_sesion_cpu * cpu = malloc(sizeof(t_sesion_cpu));
			cpu->socket_cpu = socket_nuevo;
			cpu->id_cpu = ++id_cpu;
			pthread_mutex_lock(&mutex_lista_cpu);
			list_add(cpu_conectadas, cpu);
			pthread_mutex_unlock(&mutex_lista_cpu);
			enviarPorSocket(socket_servidor, MENSAJE_HANDSHAKE);
			//creo el hilo para atender los cpu
			break;
		default:
			enviarPorSocket(socket_servidor, "No te conozco");
		}
	}

	return EXIT_SUCCESS;
}

void modificar_retardo(int ret) {
	printf("comando retardo %d\n", ret);
}

void reporte_estructuras() {
	puts("comando dump - estructuras");
}

void reporte_contenido() {
	puts("comando dump - contenido");
}

void limpiar_tlb() {
	puts("comando flush - tlb");
}

void marcar_paginas() {
	puts("comando flush - memory");
}
